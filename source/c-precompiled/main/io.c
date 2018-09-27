status_t sp_file_close(sp_port_t* port) {
  status_declare;
  status.id = sf_close(((SNDFILE*)(port->data)));
  if (status_is_success) {
    status.group = sp_status_group_sndfile;
  } else {
    port->flags = (sp_port_bit_closed | port->flags);
  };
  return (status);
};
status_t sp_file_open(uint8_t* path, sp_channel_count_t channel_count, sp_sample_rate_t sample_rate, sp_port_t* result_port) {
  status_declare;
  uint8_t bit_position;
  SF_INFO info;
  SNDFILE* file;
  int mode;
  info.format = sp_file_format;
  info.channels = channel_count;
  info.samplerate = sample_rate;
  mode = SFM_RDWR;
  file = sf_open(path, mode, (&info));
  if (!file) {
    status_set_both_goto(sp_status_group_sndfile, (sf_error(file)));
  };
  bit_position = (info.seekable ? sp_port_bit_position : 0);
  result_port->channel_count = channel_count;
  result_port->sample_rate = sample_rate;
  result_port->type = sp_port_type_file;
  result_port->flags = (sp_port_bit_input | sp_port_bit_output | bit_position);
  result_port->data = file;
exit:
  return (status);
};
status_t sp_file_write(sp_port_t* port, size_t sample_count, sp_sample_t** channel_data) {
  status_declare;
  sp_channel_count_t channel_count;
  SNDFILE* file;
  sp_sample_t* interleaved;
  size_t interleaved_size;
  sf_count_t write_count;
  memreg_init(1);
  channel_count = port->channel_count;
  file = port->data;
  interleaved_size = (channel_count * sample_count * sizeof(sp_sample_t*));
  status_require((sph_helper_malloc(interleaved_size, (&interleaved))));
  memreg_add(interleaved);
  sp_interleave(channel_data, interleaved, sample_count, channel_count);
  write_count = sf_writef_double(file, interleaved, sample_count);
  if (!(sample_count == write_count)) {
    status_set_both_goto(sp_status_group_sp, sp_status_id_file_incomplete);
  };
exit:
  memreg_free;
  return (status);
};
status_t sp_file_read(sp_port_t* port, size_t sample_count, sp_sample_t** result_channel_data) {
  status_declare;
  sp_channel_count_t channel_count;
  size_t interleaved_size;
  sp_sample_t* interleaved;
  sf_count_t read_count;
  memreg_init(1);
  channel_count = port->channel_count;
  interleaved_size = (channel_count * sample_count * sizeof(sp_sample_t));
  status_require((sph_helper_malloc(interleaved_size, (&interleaved))));
  memreg_add(interleaved);
  read_count = sf_readf_double(((SNDFILE*)(port->data)), interleaved, sample_count);
  if (!(interleaved_size == read_count)) {
    status_set_both(sp_status_group_sp, sp_status_id_eof);
  };
  sp_deinterleave(result_channel_data, interleaved, read_count, channel_count);
exit:
  memreg_free;
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
status_t sp_file_position(sp_port_t* port, size_t* result_position) {
  status_declare;
  SNDFILE* file;
  sf_count_t count;
  file = port->data;
  count = sf_seek(file, 0, SEEK_CUR);
  if (count == -1) {
    status_set_both_goto(sp_status_group_sndfile, (sf_error(file)));
  };
  *result_position = count;
exit:
  return (status);
};
/** open alsa sound output for capture or playback */
status_t sp_alsa_open(uint8_t* device_name, boolean is_input, sp_channel_count_t channel_count, sp_sample_rate_t sample_rate, int32_t latency, sp_port_t* result_port) {
  status_declare;
  snd_pcm_t* alsa_port;
  if (!device_name) {
    device_name = "default";
  };
  optional_set_number(latency, sp_default_alsa_latency);
  alsa_port = 0;
  sp_alsa_status_require((snd_pcm_open((&alsa_port), device_name, (is_input ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK), 0)));
  sp_alsa_status_require((snd_pcm_set_params(alsa_port, sp_alsa_snd_pcm_format, SND_PCM_ACCESS_RW_NONINTERLEAVED, channel_count, sample_rate, sp_default_alsa_enable_soft_resample, latency)));
  result_port->type = sp_port_type_alsa;
  result_port->flags = (is_input ? sp_port_bit_input : sp_port_bit_output);
  result_port->data = ((void*)(alsa_port));
exit:
  if (status_is_failure && alsa_port) {
    snd_pcm_close(alsa_port);
  };
  return (status);
};
status_t sp_alsa_write(sp_port_t* port, size_t sample_count, sp_sample_t** channel_data) {
  status_declare;
  snd_pcm_t* alsa_port;
  snd_pcm_sframes_t frames_count;
  alsa_port = port->data;
  frames_count = snd_pcm_writen(alsa_port, ((void**)(channel_data)), ((snd_pcm_uframes_t)(sample_count)));
  if ((frames_count < 0) && (snd_pcm_recover(alsa_port, frames_count, 0) < 0)) {
    status_set_both(sp_status_group_alsa, frames_count);
  };
  return (status);
};
status_t sp_alsa_read(sp_port_t* port, sp_sample_count_t sample_count, sp_sample_t** result_channel_data) {
  status_declare;
  snd_pcm_t* alsa_port;
  snd_pcm_sframes_t frames_count;
  alsa_port = port->data;
  frames_count = snd_pcm_readn(alsa_port, ((void**)(result_channel_data)), sample_count);
  if ((frames_count < 0) && (snd_pcm_recover(alsa_port, frames_count, 0) < 0)) {
    status_set_both(sp_status_group_alsa, frames_count);
  };
  return (status);
};
boolean sp_port_position_p(sp_port_t* a) { return ((sp_port_bit_position & a->flags)); };
boolean sp_port_input_p(sp_port_t* a) { return ((sp_port_bit_input & a->flags)); };
boolean sp_port_output_p(sp_port_t* a) { return ((sp_port_bit_output & a->flags)); };
status_t sp_port_read(sp_port_t* port, sp_sample_count_t sample_count, sp_sample_t** result_channel_data) {
  if (sp_port_type_file == port->type) {
    return ((sp_file_read(port, sample_count, result_channel_data)));
  } else if (sp_port_type_alsa == port->type) {
    return ((sp_alsa_read(port, sample_count, result_channel_data)));
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
status_t sp_port_position(sp_port_t* port, size_t* result_position) {
  status_declare;
  if (sp_port_type_file == port->type) {
    return ((sp_file_position(port, result_position)));
  } else if (sp_port_type_alsa == port->type) {
    status_set_both(sp_status_group_sp, sp_status_id_not_implemented);
    return (status);
  };
};
status_t sp_port_set_position(sp_port_t* port, size_t sample_index) {
  status_declare;
  if (sp_port_type_file == port->type) {
    return ((sp_file_set_position(port, sample_index)));
  } else if (sp_port_type_alsa == port->type) {
    status_set_both(sp_status_group_sp, sp_status_id_not_implemented);
    return (status);
  };
};