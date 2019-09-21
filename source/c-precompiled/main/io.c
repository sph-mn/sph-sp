status_t sp_file_close(sp_file_t* file) {
  status_declare;
  status.id = sf_close(((SNDFILE*)(file->data)));
  if (status_is_success) {
    status.group = sp_s_group_sndfile;
  } else {
    file->flags = (sp_file_bit_closed | file->flags);
  };
  status_return;
}
status_t sp_file_open(uint8_t* path, int mode, sp_channels_t channel_count, sp_sample_rate_t sample_rate, sp_file_t* result_file) {
  status_declare;
  SF_INFO info;
  SNDFILE* file;
  int sf_mode;
  uint8_t flags;
  if (sp_file_mode_write == mode) {
    sf_mode = SFM_WRITE;
    flags = sp_file_bit_output;
    info.format = sp_file_format;
    info.channels = channel_count;
    info.samplerate = sample_rate;
  } else if (sp_file_mode_read == mode) {
    sf_mode = SFM_READ;
    flags = sp_file_bit_input;
    info.format = 0;
    info.channels = 0;
    info.samplerate = 0;
  } else if (sp_file_mode_read_write == mode) {
    sf_mode = SFM_RDWR;
    flags = (sp_file_bit_input | sp_file_bit_output);
    info.format = sp_file_format;
    info.channels = channel_count;
    info.samplerate = sample_rate;
  } else {
    status_set_goto(sp_s_group_sp, sp_s_id_file_type);
  };
  file = sf_open(path, sf_mode, (&info));
  if (!file) {
    status_set_goto(sp_s_group_sndfile, (sf_error(file)));
  };
  result_file->flags = (info.seekable ? (sp_file_bit_position | flags) : flags);
  result_file->channel_count = info.channels;
  result_file->sample_rate = info.samplerate;
  result_file->data = file;
exit:
  status_return;
}
/** failure status if not all samples could be written (sp-s-id-file-incomplete) */
status_t sp_file_write(sp_file_t* file, sp_sample_t** channel_data, sp_time_t sample_count, sp_time_t* result_sample_count) {
  status_declare;
  sp_channels_t channel_count;
  SNDFILE* snd_file;
  sp_sample_t* interleaved;
  sp_time_t interleaved_size;
  sf_count_t frames_count;
  memreg_init(1);
  channel_count = file->channel_count;
  snd_file = file->data;
  interleaved_size = (channel_count * sample_count * sizeof(sp_sample_t));
  status_require((sph_helper_malloc(interleaved_size, (&interleaved))));
  memreg_add(interleaved);
  sp_interleave(channel_data, interleaved, sample_count, channel_count);
  frames_count = sp_sf_write(snd_file, interleaved, sample_count);
  if (!(sample_count == frames_count)) {
    status_set(sp_s_group_sp, sp_s_id_file_incomplete);
  };
  *result_sample_count = frames_count;
exit:
  memreg_free;
  status_return;
}
/** failure status only if no results read (sp-s-id-eof) */
status_t sp_file_read(sp_file_t* file, sp_time_t sample_count, sp_sample_t** result_channel_data, sp_time_t* result_sample_count) {
  status_declare;
  sp_channels_t channel_count;
  sp_time_t interleaved_size;
  sp_sample_t* interleaved;
  sf_count_t frames_count;
  memreg_init(1);
  channel_count = file->channel_count;
  interleaved_size = (channel_count * sample_count * sizeof(sp_sample_t));
  status_require((sph_helper_malloc(interleaved_size, (&interleaved))));
  memreg_add(interleaved);
  frames_count = sp_sf_read(((SNDFILE*)(file->data)), interleaved, sample_count);
  if (frames_count) {
    sp_deinterleave(result_channel_data, interleaved, frames_count, channel_count);
  } else {
    status_set(sp_s_group_sp, sp_s_id_eof);
  };
  *result_sample_count = frames_count;
exit:
  memreg_free;
  status_return;
}
/** seeks are defined in number of (multichannel) frames.
  therefore, a seek in a stereo file from the current position forward with an offset of 1 would skip forward by one sample of both channels */
status_t sp_file_position_set(sp_file_t* file, sp_time_t a) {
  status_declare;
  SNDFILE* snd_file;
  sf_count_t count;
  snd_file = file->data;
  count = sf_seek(snd_file, a, SEEK_SET);
  if (count == -1) {
    status_set_goto(sp_s_group_sndfile, (sf_error(snd_file)));
  };
exit:
  status_return;
}
status_t sp_file_position(sp_file_t* file, sp_time_t* result_position) {
  status_declare;
  SNDFILE* snd_file;
  sf_count_t count;
  snd_file = file->data;
  count = sf_seek(snd_file, 0, SEEK_CUR);
  if (count == -1) {
    status_set_goto(sp_s_group_sndfile, (sf_error(snd_file)));
  };
  *result_position = count;
exit:
  status_return;
}
boolean sp_file_input_p(sp_file_t* a) { return ((sp_file_bit_input & a->flags)); }
boolean sp_file_output_p(sp_file_t* a) { return ((sp_file_bit_output & a->flags)); }
boolean sp_file_input_output_p(sp_file_t* a) { return (((sp_file_bit_input & a->flags) && (sp_file_bit_output & a->flags))); }
