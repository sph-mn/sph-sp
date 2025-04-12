
/* 32 bit float wav file writing and reading. http://soundfile.sapp.org/doc/WaveFormat */

#define wav_string_riff htonl(0x52494646)
#define wav_string_fmt htonl(0x666d7420)
#define wav_string_wav htonl(0x57415645)
#define wav_string_data htonl(0x64617461)
status_t sp_file_open_write(uint8_t* path, sp_channel_count_t channel_count, sp_time_t sample_rate, sp_file_t* file) {
  status_declare;
  uint8_t header[44];
  *((uint32_t*)(header)) = wav_string_riff;
  *((uint32_t*)((8 + header))) = wav_string_wav;
  *((uint32_t*)((12 + header))) = wav_string_fmt;
  *((uint32_t*)((16 + header))) = 16;
  *((uint16_t*)((20 + header))) = 3;
  *((uint16_t*)((22 + header))) = channel_count;
  *((uint32_t*)((24 + header))) = sample_rate;
  *((uint32_t*)((28 + header))) = (sample_rate * channel_count * 4);
  *((uint16_t*)((32 + header))) = (channel_count * 4);
  *((uint16_t*)((34 + header))) = 32;
  *((uint32_t*)((36 + header))) = wav_string_data;
  *((uint32_t*)((40 + header))) = 0;
  file->data_size = 0;
  file->channel_count = channel_count;
  file->file = fopen(path, "w");
  if (!file->file) {
    status_set_goto(sp_s_group_libc, errno);
  };
  if (!fwrite(header, 40, 1, (file->file))) {
    fclose((file->file));
    sp_status_set_goto(sp_s_id_file_write);
  };
  fseek((file->file), 4, SEEK_CUR);
exit:
  status_return;
}
void sp_file_close_write(sp_file_t* file) {
  uint32_t chunk_size;
  chunk_size = (36 + file->data_size);
  if (file->data_size & 1) {
    uint8_t pad = 0;
    fwrite((&pad), 1, 1, (file->file));
    file->data_size += 1;
  };
  fseek((file->file), 4, SEEK_SET);
  fwrite((&chunk_size), 4, 1, (file->file));
  fseek((file->file), 40, SEEK_SET);
  fwrite((&(file->data_size)), 4, 1, (file->file));
  fclose((file->file));
}
status_t sp_file_write(sp_file_t* file, sp_sample_t** samples, sp_time_t sample_count) {
  status_declare;
  float* file_data;
  size_t interleaved_size;
  sp_channel_count_t channel_count;
  file_data = 0;
  channel_count = file->channel_count;
  interleaved_size = (channel_count * sample_count * 4);
  srq((sph_helper_malloc(interleaved_size, (&file_data))));
  for (sp_time_t i = 0; (i < sample_count); i += 1) {
    for (sp_channel_count_t j = 0; (j < channel_count); j += 1) {
      file_data[((i * channel_count) + j)] = (samples[j])[i];
    };
  };
  if (!fwrite(file_data, interleaved_size, 1, (file->file))) {
    sp_status_set_goto(sp_s_id_file_write);
  };
  file->data_size += interleaved_size;
exit:
  free(file_data);
  status_return;
}
status_t sp_file_open_read(uint8_t* path, sp_file_t* file) {
  status_declare;
  uint8_t header[44];
  uint32_t subchunk_id;
  uint32_t subchunk_size;
  file->file = fopen(path, "r");
  if (!(fread(header, 44, 1, (file->file)) && (*((uint32_t*)(header)) == wav_string_riff) && (*((uint32_t*)((8 + header))) == wav_string_wav) && (*((uint32_t*)((12 + header))) == wav_string_fmt))) {
    sp_status_set_goto(sp_s_id_file_read);
  };
  if (!((3 == *((uint16_t*)((20 + header)))) && (32 == *((uint16_t*)((34 + header)))))) {
    sp_status_set_goto(sp_s_id_file_not_implemented);
  };
  uint16_t channel_count = *((uint16_t*)((22 + header)));
  if (!((*((uint16_t*)((32 + header))) == (channel_count * 4)) && (*((uint32_t*)((28 + header))) == (*((uint16_t*)((24 + header))) * channel_count * 4)))) {
    sp_status_set_goto(sp_s_id_file_not_implemented);
  };
  file->channel_count = channel_count;
  subchunk_id = *((uint32_t*)((36 + header)));
  subchunk_size = *((uint32_t*)((40 + header)));
  while (!(wav_string_data == subchunk_id)) {
    fseek((file->file), subchunk_size, SEEK_CUR);
    if (!fread((&subchunk_id), 4, 1, (file->file))) {
      sp_status_set_goto(sp_s_id_file_read);
    };
    if (!fread((&subchunk_size), 4, 1, (file->file))) {
      sp_status_set_goto(sp_s_id_file_read);
    };
  };
exit:
  status_return;
}
status_t sp_file_read(sp_file_t file, sp_time_t sample_count, sp_sample_t** samples) {
  status_declare;
  sp_time_t interleaved_count;
  float* file_data;
  sp_time_t read;
  file_data = 0;
  interleaved_count = (file.channel_count * sample_count);
  srq((sph_helper_malloc((4 * interleaved_count), (&file_data))));
  read = fread(file_data, 4, interleaved_count, (file.file));
  if (!(interleaved_count == read)) {
    if (feof((file.file))) {
      if (!read) {
        sp_status_set_goto(sp_s_id_file_eof);
      };
    } else {
      sp_status_set_goto(sp_s_id_file_read);
    };
  };
  for (sp_time_t i = 0; (i < (read / file.channel_count)); i += 1) {
    for (sp_channel_count_t j = 0; (j < file.channel_count); j += 1) {
      (samples[j])[i] = file_data[((i * file.channel_count) + j)];
    };
  };
exit:
  free(file_data);
  status_return;
}
void sp_file_close_read(sp_file_t file) { fclose((file.file)); }
