
/* the sc version of this file defines macros which are only available in sc.
the macros are used as optional helpers to simplify common tasks where c syntax alone offers no good alternative */
void sp_event_list_display(sp_event_list_t* a) {
  while (a) {
    printf(("(%lu %lu %lu) "), (a->event.start), (a->event.end), (&(a->event)));
    a = a->next;
  };
  printf("\n");
}
void sp_event_list_reverse(sp_event_list_t** a) {
  sp_event_list_t* current;
  sp_event_list_t* next;
  current = *a;
  if (!current) {
    return;
  };
  next = current->next;
  current->next = current->previous;
  current->previous = next;
  while (next) {
    current = next;
    next = current->next;
    current->next = current->previous;
    current->previous = next;
  };
  *a = current;
}

/** removes the list element and frees the event, without having to search in the list.
   updates the head of the list if element is the first element */
void sp_event_list_remove(sp_event_list_t** a, sp_event_list_t* element) {
  if (element->previous) {
    element->previous->next = element->next;
  } else {
    *a = element->next;
  };
  if (element->next) {
    element->next->previous = element->previous;
  };
  if (element->event.free) {
    (element->event.free)((&(element->event)));
  };
  free(element);
}

/** insert sorted by start time descending. event-list might get updated with a new head element */
status_t sp_event_list_add(sp_event_list_t** a, sp_event_t event) {
  status_declare;
  sp_event_list_t* current;
  sp_event_list_t* new;
  status_require((sp_malloc_type(1, sp_event_list_t, (&new))));
  new->event = event;
  current = *a;
  if (!current) {
    /* empty */
    new->next = 0;
    new->previous = 0;
    *a = new;
    goto exit;
  };
  if (current->event.start <= event.start) {
    /* first */
    new->previous = 0;
    new->next = current;
    current->previous = new;
    *a = new;
  } else {
    while (current->next) {
      current = current->next;
      if (current->event.start <= event.start) {
        /* -- middle */
        new->next = current;
        new->previous = current->previous;
        current->previous = new;
        goto exit;
      };
    };
    /* -- last */
    new->next = 0;
    new->previous = current;
    current->next = new;
  };
exit:
  status_return;
}

/** free all list elements and the associated events. only needed if sp_seq will not further process and free all past events */
void sp_event_list_free(sp_event_list_t* events) {
  sp_event_list_t* temp;
  while (events) {
    if (events->event.free) {
      (events->event.free)((&(events->event)));
    };
    temp = events;
    events = events->next;
    free(temp);
  };
}

/** event arrays must have been prepared/sorted once with sp_seq_event_prepare for seq to work correctly.
   like for paths, start is inclusive, end is exclusive, so that 0..100 and 100..200 attach seamless.
   events can have three function pointers: prepare, generate and free.
   generate receives event relative start/end times, and output blocks that start at the current block.
   prepare will only be called once and the function pointer will be set to zero afterwards.
   if prepare fails, sp_seq returns immediately.
   past events including the event list elements are freed when processing the following block */
status_t sp_seq(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_list_t** events) {
  status_declare;
  sp_event_t e;
  sp_event_t* ep;
  sp_event_list_t* current;
  sp_event_list_t* next;
  current = *events;
  while (current) {
    ep = &(current->event);
    e = *ep;
    if (e.end <= start) {
      next = current->next;
      sp_event_list_remove(events, current);
      current = next;
      continue;
    } else if (end <= e.start) {
      break;
    } else if (e.end > start) {
      if (e.prepare) {
        status_require(((e.prepare)(ep)));
        ep->prepare = 0;
        e = *ep;
      };
      status_require(((e.generate)(((start > e.start) ? (start - e.start) : 0), (sp_min(end, (e.end)) - e.start), ((e.start > start) ? sp_block_with_offset(out, (e.start - start)) : out), ep)));
    };
    current = current->next;
  };
exit:
  status_return;
}
typedef struct {
  sp_time_t start;
  sp_time_t end;
  sp_time_t out_start;
  sp_block_t out;
  sp_event_t* event;
  status_t status;
  future_t future;
} sp_seq_future_t;
void* sp_seq_parallel_future_f(void* data) {
  sp_seq_future_t* a = data;
  if (a->event->prepare) {
    a->status = (a->event->prepare)((a->event));
    if (status_id_success == a->status.id) {
      a->event->prepare = 0;
    } else {
      return (0);
    };
  };
  a->status = (a->event->generate)((a->start), (a->end), (a->out), (a->event));
  return (0);
}

/** like sp_seq but evaluates events with multiple threads in parallel.
   there is some overhead, as each event gets its own output block */
status_t sp_seq_parallel(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_list_t** events) {
  status_declare;
  sp_event_list_t* current;
  sp_time_t active;
  sp_time_t allocated;
  sp_time_t e_end;
  sp_event_t* ep;
  sp_event_t e;
  sp_time_t e_start;
  sp_seq_future_t* sf_array;
  sp_seq_future_t* sf;
  sp_event_list_t* next;
  sf_array = 0;
  active = 0;
  allocated = 0;
  current = *events;
  /* count */
  while (current) {
    if (end <= current->event.start) {
      break;
    } else if (current->event.end > start) {
      active += 1;
    };
    current = current->next;
  };
  status_require((sp_malloc_type(active, sp_seq_future_t, (&sf_array))));
  /* distribute */
  current = *events;
  while (current) {
    ep = &(current->event);
    e = *ep;
    if (e.end <= start) {
      next = current->next;
      sp_event_list_remove(events, current);
      current = next;
      continue;
    } else if (end <= e.start) {
      break;
    } else if (e.end > start) {
      sf = (sf_array + allocated);
      e_start = ((start > e.start) ? (start - e.start) : 0);
      e_end = (sp_min(end, (e.end)) - e.start);
      sf->start = e_start;
      sf->end = e_end;
      sf->out_start = ((e.start > start) ? (e.start - start) : 0);
      sf->event = ep;
      sf->status.id = status_id_success;
      status_require((sp_block_new((out.channels), (e_end - e_start), (&(sf->out)))));
      allocated += 1;
      future_new(sp_seq_parallel_future_f, sf, (&(sf->future)));
    };
    current = current->next;
  };
  /* merge */
  for (sp_time_t sf_i = 0; (sf_i < active); sf_i += 1) {
    sf = (sf_array + sf_i);
    touch((&(sf->future)));
    status_require((sf->status));
    for (sp_time_t ci = 0; (ci < out.channels); ci += 1) {
      for (sp_time_t i = 0; (i < sf->out.size); i += 1) {
        ((out.samples)[ci])[(sf->out_start + i)] += ((sf->out.samples)[ci])[i];
      };
    };
  };
exit:
  if (sf_array) {
    for (sp_time_t i = 0; (i < allocated); i += 1) {
      sp_block_free((&((sf_array[i]).out)));
    };
    free(sf_array);
  };
  status_return;
}
void sp_group_free(sp_event_t* a) {
  sp_event_list_t* sp = a->data;
  sp_event_list_free(sp);
  sp_event_memory_free(a);
}
status_t sp_group_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* a) { return ((sp_seq(start, end, out, ((sp_event_list_t**)(&(a->data)))))); }
status_t sp_group_prepare(sp_event_t* a) {
  status_declare;
  sp_seq_events_prepare(((sp_event_list_t**)(&(a->data))));
  a->generate = sp_group_generate;
  a->free = sp_group_free;
  status_return;
}
status_t sp_group_add(sp_event_t* a, sp_event_t event) {
  status_declare;
  status_require((sp_event_list_add(((sp_event_list_t**)(&(a->data))), event)));
  if (a->end < event.end) {
    a->end = event.end;
  };
exit:
  status_return;
}
status_t sp_group_append(sp_event_t* a, sp_event_t event) {
  event.start += a->end;
  event.end += a->end;
  return ((sp_group_add(a, event)));
}
sp_channel_config_t sp_channel_config(boolean mute, sp_time_t delay, sp_time_t phs, sp_sample_t amp, sp_sample_t* amod) {
  sp_channel_config_t a;
  a.use = 1;
  a.mute = mute;
  a.delay = delay;
  a.phs = phs;
  a.amp = amp;
  a.amod = amod;
  return (a);
}
void sp_channel_config_zero(sp_channel_config_t* a) {
  sp_channel_config_t channel_config = { 0 };
  for (sp_time_t i = 0; (i < sp_channel_limit); i += 1) {
    a[i] = channel_config;
  };
}

/** heap allocates a noise_event_config struct and sets some defaults */
status_t sp_wave_event_config_new(sp_wave_event_config_t** out) {
  status_declare;
  sp_wave_event_config_t* result;
  status_require((sp_malloc_type(1, sp_wave_event_config_t, (&result))));
  (*result).wvf = sp_sine_table;
  (*result).wvf_size = sp_rate;
  (*result).phs = 0;
  (*result).frq = sp_rate;
  (*result).fmod = 0;
  (*result).amp = 1;
  (*result).amod = 0;
  (*result).channels = sp_channels;
  sp_channel_config_zero((result->channel_config));
  *out = result;
exit:
  status_return;
}
void sp_wave_event_free(sp_event_t* a) { free((a->data)); }
status_t sp_wave_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  status_declare;
  sp_time_t i;
  sp_wave_event_state_t* sp;
  sp_wave_event_state_t s;
  sp = event->data;
  s = *sp;
  for (i = start; (i < end); i += 1) {
    (out.samples)[s.channel][(i - start)] += (s.amp * (s.amod)[i] * (s.wvf)[s.phs]);
    s.phs += sp_array_or_fixed((s.fmod), (s.frq), i);
    if (s.phs >= s.wvf_size) {
      s.phs = (s.phs % s.wvf_size);
    };
  };
  sp->phs = s.phs;
  status_return;
}
status_t sp_wave_event_channel(sp_time_t duration, sp_wave_event_config_t config, sp_channel_count_t channel, sp_event_t* out) {
  status_declare;
  sp_wave_event_state_t* data;
  sp_channel_config_t channel_config;
  sp_declare_event(event);
  data = 0;
  channel_config = (config.channel_config)[channel];
  status_require((sp_malloc_type(1, sp_wave_event_state_t, (&data))));
  (*data).wvf = config.wvf;
  (*data).wvf_size = config.wvf_size;
  (*data).phs = (channel_config.use ? channel_config.phs : config.phs);
  (*data).frq = config.frq;
  (*data).fmod = config.fmod;
  (*data).amp = (channel_config.use ? channel_config.amp : config.amp);
  (*data).amod = ((channel_config.use && channel_config.amod) ? channel_config.amod : config.amod);
  (*data).channel = channel;
  event.data = data;
  event.start = (channel_config.use ? channel_config.delay : 0);
  event.end = (channel_config.use ? (duration + channel_config.delay) : duration);
  event.prepare = 0;
  event.generate = sp_wave_event_generate;
  event.free = sp_wave_event_free;
  *out = event;
exit:
  if (status_is_failure) {
    if (data) {
      free(data);
    };
  };
  status_return;
}

/** prepare an event playing waveforms from an array.
   config should have been declared with defaults using sp_declare_wave_event_config.
   event end will be longer if channel config delay is used.
   config:
   * frq (frequency): fixed frequency, only used if fmod is null
   * fmod (frequency): array with hertz values
   * wvf (waveform): array with waveform samples
   * wvf-size: count of waveform samples
   * phs (phase): initial phase offset
   * amp (amplitude): multiplied with amod
   * amod (amplitude): array with sample values
   channel-config (array with one element per channel):
   * use: if zero, config for this channel will not be applied
   * mute: non-zero for silencing this channel
   * delay: integer number of samples
   * amod: uses main amod if zero
   * amp: multiplied with amod */
status_t sp_wave_event_prepare(sp_event_t* event) {
  status_declare;
  sp_declare_event(channel);
  sp_wave_event_config_t config = *((sp_wave_event_config_t*)(event->data));
  event->data = 0;
  event->free = sp_group_free;
  for (sp_time_t ci = 0; (ci < config.channels); ci += 1) {
    if (((config.channel_config)[ci]).mute) {
      continue;
    };
    status_require((sp_wave_event_channel((event->end - event->start), config, ci, (&channel))));
    status_require((sp_group_add(event, channel)));
  };
  status_require((sp_group_prepare(event)));
exit:
  if (status_is_failure) {
    (event->free)(event);
  };
  status_return;
}
typedef struct {
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_sample_t cutl;
  sp_sample_t cuth;
  sp_sample_t trnl;
  sp_sample_t trnh;
  sp_sample_t* cutl_mod;
  sp_sample_t* cuth_mod;
  sp_time_t resolution;
  uint8_t is_reject;
  sp_random_state_t random_state;
  sp_channel_count_t channel;
  sp_convolution_filter_state_t* filter_state;
  sp_sample_t* noise;
  sp_sample_t* temp;
} sp_noise_event_state_t;
/** heap allocates a noise_event_config struct and sets some defaults */
status_t sp_noise_event_config_new(sp_noise_event_config_t** out) {
  status_declare;
  sp_noise_event_config_t* result;
  status_require((sp_malloc_type(1, sp_noise_event_config_t, (&result))));
  (*result).amp = 1;
  (*result).amod = 0;
  (*result).cutl = 0.0;
  (*result).cuth = 0.5;
  (*result).trnl = 0.1;
  (*result).trnh = 0.1;
  (*result).cutl_mod = 0;
  (*result).cuth_mod = 0;
  (*result).resolution = (sp_rate / 10);
  (*result).is_reject = 0;
  (*result).channels = sp_channels;
  sp_channel_config_zero((result->channel_config));
  *out = result;
exit:
  status_return;
}
void sp_noise_event_free(sp_event_t* a) {
  sp_noise_event_state_t* s = a->data;
  sp_convolution_filter_state_free((s->filter_state));
  free(s);
  sp_event_memory_free(a);
}

/** updates filter arguments only every resolution number of samples */
status_t sp_noise_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  status_declare;
  sp_time_t block_count;
  sp_time_t block_i;
  sp_time_t block_offset;
  sp_time_t block_rest;
  sp_time_t duration;
  sp_time_t i;
  sp_noise_event_state_t s;
  sp_noise_event_state_t* sp;
  sp_time_t t;
  sp = event->data;
  s = *sp;
  duration = (end - start);
  block_count = ((duration == s.resolution) ? 1 : sp_cheap_floor_positive((duration / s.resolution)));
  block_rest = (duration % s.resolution);
  for (block_i = 0; (block_i <= block_count); block_i += 1) {
    block_offset = (s.resolution * block_i);
    t = (start + block_offset);
    duration = ((block_count == block_i) ? block_rest : s.resolution);
    sp_samples_random((&(s.random_state)), duration, (s.noise));
    sp_windowed_sinc_bp_br((s.noise), duration, (sp_array_or_fixed((s.cutl_mod), (s.cutl), t)), (sp_array_or_fixed((s.cuth_mod), (s.cuth), t)), (s.trnl), (s.trnh), (s.is_reject), (&(s.filter_state)), (s.temp));
    for (i = 0; (i < duration); i += 1) {
      (out.samples)[s.channel][(block_offset + i)] += (s.amp * (s.amod)[(t + i)] * (s.temp)[i]);
    };
  };
  sp->random_state = s.random_state;
  status_return;
}

/** the result shows a small delay, for example, circa 40 samples for transition 0.07. the size seems to be related to ir-len.
   the filter state is initialised with one unused call to skip the delay. */
status_t sp_noise_event_filter_state(sp_sample_t cutl, sp_sample_t cuth, sp_sample_t trnl, sp_sample_t trnh, boolean is_reject, sp_random_state_t* rs, sp_convolution_filter_state_t** out) {
  status_declare;
  sp_time_t ir_len;
  sp_sample_t* temp;
  sp_sample_t* noise;
  memreg_init(2);
  ir_len = sp_windowed_sinc_lp_hp_ir_length((sp_min(trnl, trnh)));
  sp_malloc_type(ir_len, sp_sample_t, (&noise));
  memreg_add(noise);
  sp_malloc_type(ir_len, sp_sample_t, (&temp));
  memreg_add(temp);
  sp_samples_random(rs, ir_len, noise);
  status_require((sp_windowed_sinc_bp_br(noise, ir_len, cutl, cuth, trnl, trnh, is_reject, out, temp)));
exit:
  memreg_free;
  status_return;
}
status_t sp_noise_event_channel(sp_time_t duration, sp_noise_event_config_t config, sp_channel_count_t channel, sp_random_state_t rs, sp_sample_t* state_noise, sp_sample_t* state_temp, sp_event_t* event) {
  status_declare;
  sp_noise_event_state_t* state;
  sp_channel_config_t channel_config;
  sp_convolution_filter_state_t* filter_state;
  state = 0;
  filter_state = 0;
  channel_config = (config.channel_config)[channel];
  status_require((sp_noise_event_filter_state((sp_array_or_fixed((config.cutl_mod), (config.cutl), 0)), (sp_array_or_fixed((config.cuth_mod), (config.cuth), 0)), (config.trnl), (config.trnh), (config.is_reject), (&rs), (&filter_state))));
  status_require((sp_malloc_type(1, sp_noise_event_state_t, (&state))));
  (*state).amp = (channel_config.use ? channel_config.amp : config.amp);
  (*state).amod = ((channel_config.use && channel_config.amod) ? channel_config.amod : config.amod);
  (*state).cutl = config.cutl;
  (*state).cuth = config.cuth;
  (*state).trnl = config.trnl;
  (*state).trnh = config.trnh;
  (*state).cutl_mod = config.cutl_mod;
  (*state).cuth_mod = config.cuth_mod;
  (*state).resolution = config.resolution;
  (*state).is_reject = config.is_reject;
  (*state).random_state = rs;
  (*state).channel = channel;
  (*state).filter_state = filter_state;
  (*state).noise = state_noise;
  (*state).temp = state_temp;
  (*event).data = state;
  (*event).start = (channel_config.use ? channel_config.delay : 0);
  (*event).end = (channel_config.use ? (duration + channel_config.delay) : duration);
  (*event).generate = sp_noise_event_generate;
  (*event).free = sp_noise_event_free;
exit:
  if (status_is_failure) {
    if (state) {
      free(state);
    };
    if (filter_state) {
      sp_convolution_filter_state_free(filter_state);
    };
  };
  status_return;
}

/** an event for noise filtered by a windowed-sinc filter.
   very processing intensive if parameters change with low resolution.
   memory for event.state will be allocated and then owned by the caller.
   all channels use the same initial random-state */
status_t sp_noise_event_prepare(sp_event_t* event) {
  status_declare;
  sp_time_t duration;
  sp_random_state_t rs;
  sp_sample_t* state_noise;
  sp_sample_t* state_temp;
  sp_declare_event(channel);
  sp_noise_event_config_t config = *((sp_noise_event_config_t*)(event->data));
  event->data = 0;
  event->free = sp_group_free;
  duration = (event->end - event->start);
  config.resolution = (config.resolution ? config.resolution : 96);
  config.resolution = sp_min((config.resolution), duration);
  rs = sp_random_state_new((sp_time_random((&sp_default_random_state))));
  status_require((sp_event_memory_init(event, 2)));
  status_require((sp_malloc_type((config.resolution), sp_sample_t, (&state_noise))));
  sp_event_memory_add1(event, state_noise);
  status_require((sp_malloc_type((config.resolution), sp_sample_t, (&state_temp))));
  sp_event_memory_add1(event, state_temp);
  for (sp_time_t ci = 0; (ci < config.channels); ci += 1) {
    if (((config.channel_config)[ci]).mute) {
      continue;
    };
    status_require((sp_noise_event_channel(duration, config, ci, rs, state_noise, state_temp, (&channel))));
    status_require((sp_group_add(event, channel)));
  };
  status_require((sp_group_prepare(event)));
exit:
  if (status_is_failure) {
    (event->free)(event);
  };
  status_return;
}
typedef struct {
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_sample_t cut;
  sp_sample_t* cut_mod;
  sp_sample_t q_factor;
  sp_time_t passes;
  sp_state_variable_filter_t type;
  sp_random_state_t random_state;
  sp_time_t resolution;
  sp_channel_count_t channel;
  sp_cheap_filter_state_t filter_state;
  sp_sample_t* noise;
  sp_sample_t* temp;
} sp_cheap_noise_event_state_t;
/** heap allocates a noise_event_config struct and sets some defaults */
status_t sp_cheap_noise_event_config_new(sp_cheap_noise_event_config_t** out) {
  status_declare;
  sp_cheap_noise_event_config_t* result;
  status_require((sp_malloc_type(1, sp_cheap_noise_event_config_t, (&result))));
  (*result).amp = 1;
  (*result).amod = 0;
  (*result).cut = 0.5;
  (*result).cut_mod = 0;
  (*result).q_factor = 0.01;
  (*result).passes = 1;
  (*result).type = sp_state_variable_filter_lp;
  (*result).resolution = (sp_rate / 10);
  (*result).channels = sp_channels;
  sp_channel_config_zero((result->channel_config));
  *out = result;
exit:
  status_return;
}
void sp_cheap_noise_event_free(sp_event_t* a) {
  sp_cheap_noise_event_state_t* s = a->data;
  sp_cheap_filter_state_free((&(s->filter_state)));
  free(s);
  sp_event_memory_free(a);
}
status_t sp_cheap_noise_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  status_declare;
  sp_time_t block_count;
  sp_time_t block_i;
  sp_time_t block_offset;
  sp_time_t block_rest;
  sp_time_t duration;
  sp_time_t i;
  sp_cheap_noise_event_state_t s;
  sp_cheap_noise_event_state_t* sp;
  sp_time_t t;
  /* update filter arguments only every resolution number of samples */
  sp = event->data;
  s = *sp;
  duration = (end - start);
  block_count = ((duration == s.resolution) ? 1 : sp_cheap_floor_positive((duration / s.resolution)));
  block_rest = (duration % s.resolution);
  /* total block count is block-count plus rest-block */
  for (block_i = 0; (block_i <= block_count); block_i += 1) {
    block_offset = (s.resolution * block_i);
    t = (start + block_offset);
    duration = ((block_count == block_i) ? block_rest : s.resolution);
    sp_samples_random((&(s.random_state)), duration, (s.noise));
    sp_cheap_filter((s.type), (s.noise), duration, (sp_array_or_fixed((s.cut_mod), (s.cut), t)), (s.passes), (s.q_factor), (&(s.filter_state)), (s.temp));
    for (i = 0; (i < duration); i += 1) {
      (out.samples)[s.channel][(block_offset + i)] += (s.amp * (s.amod)[(t + i)] * (s.temp)[i]);
    };
  };
  sp->random_state = s.random_state;
  status_return;
}
status_t sp_cheap_noise_event_channel(sp_time_t duration, sp_cheap_noise_event_config_t config, sp_channel_count_t channel, sp_random_state_t rs, sp_sample_t* state_noise, sp_sample_t* state_temp, sp_event_t* out) {
  status_declare;
  sp_cheap_noise_event_state_t* data;
  sp_channel_config_t channel_config;
  sp_declare_event(event);
  sp_declare_cheap_filter_state(filter_state);
  data = 0;
  channel_config = (config.channel_config)[channel];
  status_require((sp_malloc_type(1, sp_cheap_noise_event_state_t, (&data))));
  status_require((sp_cheap_filter_state_new((config.resolution), (config.passes), (&filter_state))));
  (*data).amp = (channel_config.use ? channel_config.amp : config.amp);
  (*data).amod = ((channel_config.use && channel_config.amod) ? channel_config.amod : config.amod);
  (*data).cut = config.cut;
  (*data).cut_mod = config.cut_mod;
  (*data).type = config.type;
  (*data).passes = config.passes;
  (*data).q_factor = config.q_factor;
  (*data).resolution = config.resolution;
  (*data).random_state = rs;
  (*data).channel = channel;
  (*data).filter_state = filter_state;
  (*data).noise = state_noise;
  (*data).temp = state_temp;
  event.data = data;
  event.start = (channel_config.use ? channel_config.delay : 0);
  event.end = (channel_config.use ? (duration + channel_config.delay) : duration);
  event.prepare = 0;
  event.generate = sp_cheap_noise_event_generate;
  event.free = sp_cheap_noise_event_free;
  *out = event;
exit:
  if (status_is_failure) {
    if (data) {
      free(data);
    };
    sp_cheap_filter_state_free((&filter_state));
  };
  status_return;
}

/** an event for noise filtered by a state-variable filter.
   lower processing costs even when parameters change with high resolution.
   multiple passes almost multiply performance costs.
   memory for event.state will be allocated and then owned by the caller */
status_t sp_cheap_noise_event_prepare(sp_event_t* event) {
  status_declare;
  sp_random_state_t rs;
  sp_sample_t* state_noise;
  sp_sample_t* state_temp;
  sp_declare_event(channel);
  sp_cheap_noise_event_config_t config = *((sp_cheap_noise_event_config_t*)(event->data));
  event->data = 0;
  event->free = sp_group_free;
  config.passes = (config.passes ? config.passes : 1);
  config.resolution = (config.resolution ? config.resolution : 96);
  config.resolution = sp_min((config.resolution), (event->end - event->start));
  rs = sp_random_state_new((sp_time_random((&sp_default_random_state))));
  status_require((sp_event_memory_init(event, 2)));
  status_require((sp_malloc_type((config.resolution), sp_sample_t, (&state_noise))));
  sp_event_memory_add1(event, state_noise);
  status_require((sp_malloc_type((config.resolution), sp_sample_t, (&state_temp))));
  sp_event_memory_add1(event, state_temp);
  for (sp_time_t ci = 0; (ci < config.channels); ci += 1) {
    if (((config.channel_config)[ci]).mute) {
      continue;
    };
    status_require((sp_cheap_noise_event_channel((event->end - event->start), config, ci, rs, state_noise, state_temp, (&channel))));
    status_require((sp_group_add(event, channel)));
  };
  status_require((sp_group_prepare(event)));
exit:
  status_return;
}

/** ensures that event memory is initialized and can take $additional_size more elements */
status_t sp_event_memory_init(sp_event_t* a, sp_time_t additional_size) {
  status_declare;
  if (a->memory.data) {
    if ((additional_size > array3_unused_size((a->memory))) && sp_memory_resize((&(a->memory)), (array3_max_size((a->memory)) + (additional_size - array3_unused_size((a->memory)))))) {
      sp_memory_error;
    };
  } else {
    if (sp_memory_new(additional_size, (&(a->memory)))) {
      sp_memory_error;
    };
  };
exit:
  status_return;
}
void sp_event_memory_add(sp_event_t* a, void* address, sp_memory_free_t handler) {
  memreg2_t m;
  m.address = address;
  m.handler = handler;
  sp_memory_add((a->memory), m);
}

/** free all registered memory and unitialize the event-memory register */
void sp_event_memory_free(sp_event_t* a) {
  memreg2_t m;
  for (sp_time_t i = 0; (i < array3_size((a->memory))); i += 1) {
    m = array3_get((a->memory), i);
    (m.handler)((m.address));
  };
  array3_free((a->memory));
}
void sp_map_event_free(sp_event_t* event) {
  sp_map_event_state_t* s;
  s = event->data;
  if (s->event.free) {
    (s->event.free)((&(s->event)));
  };
  free(s);
  sp_event_memory_free(event);
}
status_t sp_map_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  status_declare;
  sp_map_event_state_t* s = event->data;
  status_require(((s->event.generate)(start, end, out, (&(s->event)))));
  status_require(((s->map_generate)(start, end, out, out, (s->state))));
exit:
  status_return;
}

/** creates temporary output, lets event write to it, and passes the temporary output to a user function */
status_t sp_map_event_isolated_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  status_declare;
  sp_block_t temp_out;
  sp_map_event_state_t* s = event->data;
  status_require((sp_block_new((out.channels), (out.size), (&temp_out))));
  status_require(((s->event.generate)(start, end, temp_out, (&(s->event)))));
  status_require(((s->map_generate)(start, end, temp_out, out, (s->state))));
exit:
  sp_block_free((&temp_out));
  status_return;
}

/** the wrapped event will be freed with the map-event.
   use cases: filter chains, post processing.
   config:
     map-generate: map function (start end sp_block_t:in sp_block_t:out void*:state)
     state: custom state value passed to f.
     isolate: use a dedicated output buffer for event
       events can be wrapped in multiple sp_map_event with an isolated sp_map_event on top that
       finally writes to main out to mix with other events */
status_t sp_map_event_prepare(sp_event_t* event) {
  status_declare;
  sp_map_event_state_t* state;
  free_on_error_init(1);
  sp_map_event_config_t config = *((sp_map_event_config_t*)(event->data));
  status_require((sp_malloc_type(1, sp_map_event_state_t, (&state))));
  free_on_error1(state);
  status_require(((config.event.prepare)((&(config.event)))));
  state->event = config.event;
  state->state = config.state;
  state->map_generate = config.map_generate;
  event->data = state;
  event->generate = (config.isolate ? sp_map_event_isolated_generate : sp_map_event_generate);
  event->free = sp_map_event_free;
exit:
  if (status_is_failure) {
    free_on_error_free;
  };
  status_return;
}
