/** set default if number is negative */
#define optional_set_number(a, default) \
  if (0 > a) { \
    a = default; \
  }
/** default if number is negative */
#define optional_number(a, default) ((0 > a) ? default : a)
/** define a deinterleave, interleave or similar routine.
    a: deinterleaved
    b: interleaved */
#define define_sp_interleave(name, type, body) \
  void name(type** a, type* b, size_t a_size, uint32_t channel_count) { \
    size_t b_size; \
    uint32_t channel; \
    b_size = (a_size * channel_count); \
    while (a_size) { \
      a_size = (a_size - 1); \
      channel = channel_count; \
      while (channel) { \
        channel = (channel - 1); \
        b_size = (b_size - 1); \
        body; \
      }; \
    }; \
  }
define_sp_interleave(sp_interleave, sp_sample_t, ({ b[b_size] = (a[channel])[a_size]; }));
define_sp_interleave(sp_deinterleave, sp_sample_t, ({ (a[channel])[a_size] = b[b_size]; }));
/* file */
status_t sp_file_close(sp_port_t* port) {
  status_declare;
  status.id = sf_close(((SNDFILE*)(port->data)));
  if (!status.id) {
    port->flags = (sp_port_bit_closed | port->flags);
  };
  return (status);
};
status_t sp_file_open(sp_port_t* result, uint8_t* path, uint32_t channel_count, uint32_t sample_rate) {
  status_declare;
  uint8_t bit_position;
  SF_INFO info;
  SNDFILE* file;
  int mode;
  info.format = sp_file_sf_format;
  info.channels = channel_count;
  info.samplerate = sample_rate;
  mode = SFM_RDWR;
  file = sf_open(path, mode, (&info));
  if (!file) {
    status_set_both_goto(sp_status_group_sndfile, (sf_error(file)));
  };
  bit_position = (info.seekable ? sp_port_bit_position : 0);
  result->channel_count = channel_count;
  result->sample_rate = sample_rate;
  result->type = sp_port_type_file;
  result->flags = (sp_port_bit_input | sp_port_bit_output | bit_position);
  result->data = file;
exit:
  return (status);
};
status_t sp_file_write(sp_port_t* port, size_t sample_count, sp_sample_t** channel_data) {
  status_declare;
  uint32_t channel_count;
  SNDFILE* file;
  size_t interleaved_size;
  sf_count_t result_count;
  local_memory_init(1);
  channel_count = port->channel_count;
  file = port->data;
  interleaved_size = (channel_count * sample_count * sizeof(sp_sample_t*));
  sp_alloc_define(interleaved, sp_sample_t*, interleaved_size);
  local_memory_add(interleaved);
  sp_interleave(channel_data, interleaved, sample_count, channel_count);
  result_count = sf_writef_double(file, interleaved, sample_count);
  if (!(sample_count == result_count)) {
    status_set_both_goto(sp_status_group_sp, sp_status_id_file_incomplete);
  };
exit:
  local_memory_free;
  return (status);
};
status_t sp_file_read(sp_sample_t** result, sp_port_t* port, size_t sample_count) {
  status_declare;
  uint32_t channel_count;
  size_t interleaved_size;
  sf_count_t result_count;
  local_memory_init(1);
  channel_count = port->channel_count;
  interleaved_size = (channel_count * sample_count * sizeof(sp_sample_t));
  sp_alloc_define(interleaved, sp_sample_t*, interleaved_size);
  local_memory_add(interleaved);
  result_count = sf_readf_double(((SNDFILE*)(port->data)), interleaved, sample_count);
  if (!(interleaved_size == result_count)) {
    status_set_both(sp_status_group_sp, sp_status_id_eof);
  };
  sp_deinterleave(result, interleaved, result_count, channel_count);
exit:
  local_memory_free;
  return (status);
};
status_t sp_file_set_position(sp_port_t* port, size_t a) {
  status_declare;
  SNDFILE* file;
  sf_count_t count;
  file = port->data;
  count = sf_seek(file, a, SEEK_SET);
  if (count == -1) {
    status_set_both_goto(sp_status_group_sndfile, (sf_error(file)));
  };
exit:
  return (status);
};
status_t sp_file_position(size_t* result, sp_port_t* port) {
  status_declare;
  SNDFILE* file;
  sf_count_t count;
  file = port->data;
  count = sf_seek(file, 0, SEEK_CUR);
  if (count == -1) {
    status_set_both_goto(sp_status_group_sndfile, (sf_error(file)));
  };
  *result = count;
exit:
  return (status);
};
/* -- alsa */
/** open alsa sound output for capture or playback */
status_t sp_alsa_open(sp_port_t* result, uint8_t* device_name, boolean input_p, uint32_t channel_count, uint32_t sample_rate, int32_t latency) {
  status_declare;
  snd_pcm_t* alsa_port;
  if (!device_name) {
    device_name = "default";
  };
  optional_set_number(latency, sp_default_alsa_latency);
  alsa_port = 0;
  sp_alsa_status_require((snd_pcm_open((&alsa_port), device_name, (input_p ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK), 0)));
  sp_alsa_status_require((snd_pcm_set_params(alsa_port, sp_alsa_snd_pcm_format, SND_PCM_ACCESS_RW_NONINTERLEAVED, channel_count, sample_rate, sp_default_alsa_enable_soft_resample, latency)));
  result->type = sp_port_type_alsa;
  result->flags = (input_p ? sp_port_bit_input : sp_port_bit_output);
  result->data = ((void*)(alsa_port));
exit:
  if (status_is_failure && alsa_port) {
    snd_pcm_close(alsa_port);
  };
  return (status);
};
status_t sp_alsa_write(sp_port_t* port, size_t sample_count, sp_sample_t** channel_data) {
  status_declare;
  snd_pcm_t* alsa_port;
  snd_pcm_sframes_t frames_written;
  alsa_port = port->data;
  frames_written = snd_pcm_writen(alsa_port, ((void**)(channel_data)), ((snd_pcm_uframes_t)(sample_count)));
  if ((frames_written < 0) && (snd_pcm_recover(alsa_port, frames_written, 0) < 0)) {
    status_set_both(sp_status_group_alsa, frames_written);
  };
  return (status);
};
status_t sp_alsa_read(sp_sample_t** result, sp_port_t* port, uint32_t sample_count) {
  status_declare;
  snd_pcm_t* alsa_port;
  snd_pcm_sframes_t frames_read;
  alsa_port = port->data;
  frames_read = snd_pcm_readn(alsa_port, ((void**)(result)), sample_count);
  if ((frames_read < 0) && (snd_pcm_recover(alsa_port, frames_read, 0) < 0)) {
    status_set_both(sp_status_group_alsa, frames_read);
  };
  return (status);
};
/* -- sc-port */
boolean sp_port_position_p(sp_port_t* a) { return ((sp_port_bit_position & a->flags)); };
boolean sp_port_input_p(sp_port_t* a) { return ((sp_port_bit_input & a->flags)); };
boolean sp_port_output_p(sp_port_t* a) { return ((sp_port_bit_output & a->flags)); };
status_t sp_port_read(sp_sample_t** result, sp_port_t* port, uint32_t sample_count) {
  if (sp_port_type_file == port->type) {
    return ((sp_file_read(result, port, sample_count)));
  } else if (sp_port_type_alsa == port->type) {
    return ((sp_alsa_read(result, port, sample_count)));
  };
};
status_t sp_port_write(sp_port_t* port, size_t sample_count, sp_sample_t** channel_data) {
  if (sp_port_type_file == port->type) {
    return ((sp_file_write(port, sample_count, channel_data)));
  } else if (sp_port_type_alsa == port->type) {
    return ((sp_alsa_write(port, sample_count, channel_data)));
  };
};
status_t sp_port_close(sp_port_t* a) {
  status_declare;
  if (sp_port_type_alsa == a->type) {
    sp_alsa_status_require((snd_pcm_close(((snd_pcm_t*)(a->data)))));
  } else if (sp_port_type_file == a->type) {
    status = sp_file_close(a);
  };
  if (status_is_success) {
    a->flags = (sp_port_bit_closed | a->flags);
  };
exit:
  return (status);
};
#define sp_port_not_implemented \
  status_declare; \
  status_set_both(sp_status_group_sp, sp_status_id_not_implemented); \
  return (status);
status_t sp_port_position(size_t* result, sp_port_t* port) {
  if (sp_port_type_file == port->type) {
    return ((sp_file_position(result, port)));
  } else if (sp_port_type_alsa == port->type) {
    sp_port_not_implemented;
  };
};
status_t sp_port_set_position(sp_port_t* port, size_t sample_index) {
  if (sp_port_type_file == port->type) {
    return ((sp_file_set_position(port, sample_index)));
  } else if (sp_port_type_alsa == port->type) {
    sp_port_not_implemented;
  };
};