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
status_t sp_file_open(uint8_t* path, int mode, sp_channel_count_t channel_count, sp_sample_rate_t sample_rate, sp_port_t* result_port) {
  status_declare;
  SF_INFO info;
  SNDFILE* file;
  int sf_mode;
  uint8_t flags;
  if (sp_port_mode_write == mode) {
    sf_mode = SFM_WRITE;
    flags = sp_port_bit_output;
    info.format = sp_file_format;
    info.channels = channel_count;
    info.samplerate = sample_rate;
  } else if (sp_port_mode_read == mode) {
    sf_mode = SFM_READ;
    flags = sp_port_bit_input;
    info.format = 0;
    info.channels = 0;
    info.samplerate = 0;
  } else if (sp_port_mode_read_write == mode) {
    sf_mode = SFM_RDWR;
    flags = (sp_port_bit_input | sp_port_bit_output);
    info.format = sp_file_format;
    info.channels = channel_count;
    info.samplerate = sample_rate;
  } else {
    status_set_both_goto(sp_status_group_sp, sp_status_id_port_type);
  };
  file = sf_open(path, sf_mode, (&info));
  if (!file) {
    status_set_both_goto(sp_status_group_sndfile, (sf_error(file)));
  };
  result_port->flags = (info.seekable ? (sp_port_bit_position | flags) : flags);
  result_port->channel_count = info.channels;
  result_port->sample_rate = info.samplerate;
  result_port->type = sp_port_type_file;
  result_port->data = file;
exit:
  return (status);
};
/** failure status if not all samples could be written (sp-status-id-file-incomplete) */
status_t sp_file_write(sp_port_t* port, sp_sample_t** channel_data, sp_sample_count_t sample_count, sp_sample_count_t* result_sample_count) {
  status_declare;
  sp_channel_count_t channel_count;
  SNDFILE* file;
  sp_sample_t* interleaved;
  sp_sample_count_t interleaved_size;
  sf_count_t frames_count;
  memreg_init(1);
  channel_count = port->channel_count;
  file = port->data;
  interleaved_size = (channel_count * sample_count * sizeof(sp_sample_t));
  status_require((sph_helper_malloc(interleaved_size, (&interleaved))));
  memreg_add(interleaved);
  sp_interleave(channel_data, interleaved, sample_count, channel_count);
  frames_count = sp_sf_write(file, interleaved, sample_count);
  if (!(sample_count == frames_count)) {
    status_set_both(sp_status_group_sp, sp_status_id_file_incomplete);
  };
  *result_sample_count = frames_count;
exit:
  memreg_free;
  return (status);
};
/** failure status only if no results read (sp-status-id-eof) */
status_t sp_file_read(sp_port_t* port, sp_sample_count_t sample_count, sp_sample_t** result_channel_data, sp_sample_count_t* result_sample_count) {
  status_declare;
  sp_channel_count_t channel_count;
  sp_sample_count_t interleaved_size;
  sp_sample_t* interleaved;
  sf_count_t frames_count;
  memreg_init(1);
  channel_count = port->channel_count;
  interleaved_size = (channel_count * sample_count * sizeof(sp_sample_t));
  status_require((sph_helper_malloc(interleaved_size, (&interleaved))));
  memreg_add(interleaved);
  frames_count = sp_sf_read(((SNDFILE*)(port->data)), interleaved, sample_count);
  if (frames_count) {
    sp_deinterleave(result_channel_data, interleaved, frames_count, channel_count);
  } else {
    status_set_both(sp_status_group_sp, sp_status_id_eof);
  };
  *result_sample_count = frames_count;
exit:
  memreg_free;
  return (status);
};
/** seeks are defined in number of (multichannel) frames.
  therefore, a seek in a stereo file from the current position forward with an offset of 1 would skip forward by one sample of both channels */
status_t sp_file_position_set(sp_port_t* port, int64_t a) {
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
status_t sp_file_position(sp_port_t* port, sp_sample_count_t* result_position) {
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
status_t sp_alsa_open(uint8_t* device_name, int mode, sp_channel_count_t channel_count, sp_sample_rate_t sample_rate, int32_t latency, sp_port_t* result_port) {
  status_declare;
  snd_pcm_t* alsa_port;
  int alsa_mode;
  alsa_port = 0;
  if (!device_name) {
    device_name = "default";
  };
  if (0 > latency) {
    latency = sp_default_alsa_latency;
  };
  if (sp_port_mode_write == mode) {
    alsa_mode = SND_PCM_STREAM_CAPTURE;
  } else if (sp_port_mode_read == mode) {
    alsa_mode = SND_PCM_STREAM_PLAYBACK;
  } else {
    status_set_both_goto(sp_status_group_sp, sp_status_id_port_type);
  };
  sp_alsa_status_require((snd_pcm_open((&alsa_port), device_name, alsa_mode, 0)));
  sp_alsa_status_require((snd_pcm_set_params(alsa_port, sp_alsa_snd_pcm_format, SND_PCM_ACCESS_RW_NONINTERLEAVED, channel_count, sample_rate, sp_default_alsa_enable_soft_resample, latency)));
  result_port->type = sp_port_type_alsa;
  result_port->flags = ((mode == sp_port_mode_write) ? sp_port_bit_output : sp_port_bit_input);
  result_port->data = ((void*)(alsa_port));
exit:
  if (status_is_failure && alsa_port) {
    snd_pcm_close(alsa_port);
  };
  return (status);
};
status_t sp_alsa_write(sp_port_t* port, sp_sample_t** channel_data, sp_sample_count_t sample_count, sp_sample_count_t* result_sample_count) {
  status_declare;
  snd_pcm_t* alsa_port;
  snd_pcm_sframes_t frames_count;
  alsa_port = port->data;
  frames_count = snd_pcm_writen(alsa_port, ((void**)(channel_data)), ((snd_pcm_uframes_t)(sample_count)));
  if (frames_count < 0) {
    if (snd_pcm_recover(alsa_port, frames_count, 0)) {
      status_set_both(sp_status_group_alsa, frames_count);
    };
    *result_sample_count = 0;
  } else {
    *result_sample_count = frames_count;
  };
  return (status);
};
/** snd_pcm_recover handles some errors to prepare the stream for next io.
  if frames_count is negative there is no way to know how much has been
  read if any and result_sample_count is zero */
status_t sp_alsa_read(sp_port_t* port, sp_sample_count_t sample_count, sp_sample_t** result_channel_data, sp_sample_count_t* result_sample_count) {
  status_declare;
  snd_pcm_t* alsa_port;
  snd_pcm_sframes_t frames_count;
  alsa_port = port->data;
  frames_count = snd_pcm_readn(alsa_port, ((void**)(result_channel_data)), sample_count);
  if (frames_count < 0) {
    if (snd_pcm_recover(alsa_port, frames_count, 0)) {
      status_set_both(sp_status_group_alsa, frames_count);
    };
    *result_sample_count = 0;
  } else {
    *result_sample_count = frames_count;
  };
  return (status);
};
boolean sp_port_position_p(sp_port_t* a) { return ((sp_port_bit_position & a->flags)); };
boolean sp_port_input_p(sp_port_t* a) { return ((sp_port_bit_input & a->flags)); };
boolean sp_port_output_p(sp_port_t* a) { return ((sp_port_bit_output & a->flags)); };
status_t sp_port_read(sp_port_t* port, sp_sample_count_t sample_count, sp_sample_t** result_channel_data, sp_sample_count_t* result_sample_count) {
  if (sp_port_type_file == port->type) {
    return ((sp_file_read(port, sample_count, result_channel_data, result_sample_count)));
  } else if (sp_port_type_alsa == port->type) {
    return ((sp_alsa_read(port, sample_count, result_channel_data, result_sample_count)));
  };
};
status_t sp_port_write(sp_port_t* port, sp_sample_t** channel_data, sp_sample_count_t sample_count, sp_sample_count_t* result_sample_count) {
  if (sp_port_type_file == port->type) {
    return ((sp_file_write(port, channel_data, sample_count, result_sample_count)));
  } else if (sp_port_type_alsa == port->type) {
    return ((sp_alsa_write(port, channel_data, sample_count, result_sample_count)));
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
status_t sp_port_position(sp_port_t* port, sp_sample_count_t* result_position) {
  status_declare;
  if (sp_port_type_file == port->type) {
    return ((sp_file_position(port, result_position)));
  } else if (sp_port_type_alsa == port->type) {
    status_set_both(sp_status_group_sp, sp_status_id_not_implemented);
    return (status);
  };
};
status_t sp_port_position_set(sp_port_t* port, size_t sample_offset) {
  status_declare;
  if (sp_port_type_file == port->type) {
    return ((sp_file_position_set(port, sample_offset)));
  } else if (sp_port_type_alsa == port->type) {
    status_set_both(sp_status_group_sp, sp_status_id_not_implemented);
    return (status);
  } else {
    status_set_both(sp_status_group_sp, sp_status_id_invalid_argument);
    return (status);
  };
};