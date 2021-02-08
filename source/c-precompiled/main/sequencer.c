
/* this file defines macros only available in sc, that are used as optional helpers to simplify common tasks where c syntax alone offers no good alternative */
void sp_event_sort_swap(void* a, ssize_t b, ssize_t c) {
  sp_event_t d;
  d = ((sp_event_t*)(a))[b];
  ((sp_event_t*)(a))[b] = ((sp_event_t*)(a))[c];
  ((sp_event_t*)(a))[c] = d;
}
uint8_t sp_event_sort_less_p(void* a, ssize_t b, ssize_t c) { return (((((sp_event_t*)(a))[b]).start < (((sp_event_t*)(a))[c]).start)); }
void sp_seq_events_prepare(sp_events_t* events) { quicksort(sp_event_sort_less_p, sp_event_sort_swap, (events->data), 0, (events->used - 1)); }

/** event arrays must have been prepared/sorted with sp_seq_event_prepare for seq to work correctly.
   like for paths, start is inclusive, end is exclusive, so that 0..100 and 100..200 attach seamless.
   events can have three function pointers: prepare, generate and free.
   generate receives event relative start/end times, and output blocks that start at the current block.
   prepare will only be called once and the function pointer will be set to zero afterwards.
   if prepare fails, sp_seq returns, and events.current will be the event that produced the error.
   events.current is the first index after past events */
status_t sp_seq(sp_time_t start, sp_time_t end, sp_block_t out, sp_events_t* events) {
  status_declare;
  sp_event_t e;
  sp_event_t* ep;
  sp_time_t i;
  for (i = events->current; (i < events->used); i += 1) {
    ep = (events->data + i);
    e = *ep;
    if (e.end <= start) {
      if (e.free) {
        (e.free)(ep);
      };
      array4_forward((*events));
    } else if (end <= e.start) {
      break;
    } else if (e.end > start) {
      if (e.prepare) {
        status_require(((e.prepare)(ep)));
        ep->prepare = 0;
        e.state = ep->state;
      };
      status_require(((e.generate)(((start > e.start) ? (start - e.start) : 0), (sp_min(end, (e.end)) - e.start), ((e.start > start) ? sp_block_with_offset(out, (e.start - start)) : out), (e.state))));
    };
  };
exit:
  status_return;
}

/** free all events starting from the current event. only needed if sp_seq will not further process, and thereby free, the events */
void sp_seq_events_free(sp_events_t* events) {
  sp_events_t a;
  void (*f)(sp_event_t*);
  a = *events;
  while (array4_in_range(a)) {
    f = (array4_get(a)).free;
    if (f) {
      f((a.data + a.current));
    };
    array4_forward(a);
  };
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
  a->status = (a->event->generate)((a->start), (a->end), (a->out), (a->event->state));
  return (0);
}

/** like sp_seq but evaluates events with multiple threads in parallel.
   there is some overhead, as each event gets its own output block */
status_t sp_seq_parallel(sp_time_t start, sp_time_t end, sp_block_t out, sp_events_t* events) {
  status_declare;
  sp_time_t active;
  sp_time_t allocated;
  sp_channel_count_t ci;
  sp_time_t e_end;
  sp_event_t* ep;
  sp_event_t e;
  sp_time_t e_start;
  sp_time_t i;
  sp_seq_future_t* sf_array;
  sp_time_t sf_i;
  sp_seq_future_t* sf;
  sf_array = 0;
  sf_i = 0;
  active = 0;
  allocated = 0;
  for (i = events->current; (i < events->used); i += 1) {
    ep = (events->data + i);
    if (end <= ep->start) {
      break;
    } else if (ep->end > start) {
      active += 1;
    };
  };
  status_require((sp_malloc_type(active, sp_seq_future_t, (&sf_array))));
  /* distribute */
  for (i = events->current; (i < events->used); i += 1) {
    ep = (events->data + i);
    e = *ep;
    if (e.end <= start) {
      if (e.free) {
        (e.free)(ep);
      };
      array4_forward((*events));
    } else if (end <= e.start) {
      break;
    } else if (e.end > start) {
      sf = (sf_array + sf_i);
      e_start = ((start > e.start) ? (start - e.start) : 0);
      e_end = (sp_min(end, (e.end)) - e.start);
      sf->start = e_start;
      sf->end = e_end;
      sf->out_start = ((e.start > start) ? (e.start - start) : 0);
      sf->event = (events->data + i);
      sf->status.id = status_id_success;
      status_require((sp_block_new((out.channels), (e_end - e_start), (&(sf->out)))));
      allocated += 1;
      sf_i += 1;
      future_new(sp_seq_parallel_future_f, sf, (&(sf->future)));
    };
  };
  /* merge */
  for (sf_i = 0; (sf_i < active); sf_i += 1) {
    sf = (sf_array + sf_i);
    touch((&(sf->future)));
    status_require((sf->status));
    for (ci = 0; (ci < out.channels); ci += 1) {
      for (i = 0; (i < sf->out.size); i += 1) {
        ((out.samples)[ci])[(sf->out_start + i)] += ((sf->out.samples)[ci])[i];
      };
    };
  };
exit:
  if (sf_array) {
    for (i = 0; (i < allocated); i += 1) {
      sp_block_free(((sf_array[i]).out));
    };
    free(sf_array);
  };
  status_return;
}
void sp_group_event_free(sp_event_t* a) {
  sp_events_t* sp = a->state;
  sp_seq_events_free(sp);
  array4_free((*sp));
  free(sp);
  sp_event_memory_free(a);
}
status_t sp_group_prepare(sp_event_t* event) {
  status_declare;
  sp_seq_events_prepare(((sp_events_t*)(event->state)));
  status_return;
}
void sp_group_free(sp_event_t* event) {
  if (!event->state) {
    return;
  };
  sp_events_t* events = event->state;
  sp_seq_events_free(events);
  array4_free((*events));
  free(events);
}
status_t sp_group_new(sp_time_t start, sp_group_size_t event_size, sp_event_t* out) {
  status_declare;
  sp_events_t* s;
  s = 0;
  status_require((sp_malloc_type(1, sp_events_t, (&s))));
  if (sp_events_new(event_size, s)) {
    sp_memory_error;
  };
  (*out).state = s;
  (*out).start = start;
  (*out).end = start;
  (*out).prepare = sp_group_prepare;
  (*out).generate = ((sp_event_generate_t)(sp_seq));
  (*out).free = sp_group_free;
exit:
  if (status_is_failure) {
    if (s) {
      free(s);
    };
  };
  status_return;
}
void sp_group_append(sp_event_t* a, sp_event_t event) {
  event.start += a->end;
  event.end += a->end;
  sp_group_add((*a), event);
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
void sp_wave_event_free(sp_event_t* a) { free((a->state)); }
status_t sp_wave_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, void* state) {
  status_declare;
  sp_time_t i;
  sp_wave_event_state_t* sp;
  sp_wave_event_state_t s;
  sp = state;
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
  sp_wave_event_state_t* state;
  sp_channel_config_t channel_config;
  sp_declare_event(event);
  state = 0;
  channel_config = (config.channel_config)[channel];
  status_require((sp_malloc_type(1, sp_wave_event_state_t, (&state))));
  (*state).wvf = config.wvf;
  (*state).wvf_size = config.wvf_size;
  (*state).phs = (channel_config.use ? channel_config.phs : config.phs);
  (*state).frq = config.frq;
  (*state).fmod = config.fmod;
  (*state).amp = (channel_config.use ? channel_config.amp : config.amp);
  (*state).amod = ((channel_config.use && channel_config.amod) ? channel_config.amod : config.amod);
  (*state).channel = channel;
  event.state = state;
  event.start = (channel_config.use ? channel_config.delay : 0);
  event.end = (channel_config.use ? (duration + channel_config.delay) : duration);
  event.prepare = 0;
  event.generate = sp_wave_event_generate;
  event.free = sp_wave_event_free;
  *out = event;
exit:
  if (status_is_failure) {
    if (state) {
      free(state);
    };
  };
  status_return;
}

/** create an event playing waveforms from an array.
   config should have been declared with defaults using sp-declare-wave-event-config.
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
status_t sp_wave_event(sp_time_t start, sp_time_t end, sp_wave_event_config_t config, sp_event_t* out) {
  status_declare;
  sp_channel_count_t ci;
  sp_event_t event;
  sp_declare_event(group);
  status_require((sp_group_new(0, (config.channels), (&group))));
  for (ci = 0; (ci < config.channels); ci += 1) {
    if (((config.channel_config)[ci]).mute) {
      continue;
    };
    status_require((sp_wave_event_channel((end - start), config, ci, (&event))));
    sp_group_add(group, event);
  };
  *out = group;
exit:
  if (status_is_failure) {
    sp_group_free((&group));
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
void sp_noise_event_free(sp_event_t* a) {
  sp_noise_event_state_t* s = a->state;
  sp_convolution_filter_state_free((s->filter_state));
  free((a->state));
  sp_event_memory_free(a);
}

/** updates filter arguments only every resolution number of samples */
status_t sp_noise_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, void* state) {
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
  sp = state;
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
status_t sp_noise_event_channel(sp_time_t duration, sp_noise_event_config_t config, sp_channel_count_t channel, sp_random_state_t rs, sp_sample_t* state_noise, sp_sample_t* state_temp, sp_event_t* out) {
  status_declare;
  sp_noise_event_state_t* state;
  sp_channel_config_t channel_config;
  sp_convolution_filter_state_t* filter_state;
  sp_declare_event(event);
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
  event.state = state;
  event.start = (channel_config.use ? channel_config.delay : 0);
  event.end = (channel_config.use ? (duration + channel_config.delay) : duration);
  event.prepare = 0;
  event.generate = sp_noise_event_generate;
  event.free = sp_noise_event_free;
  *out = event;
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
status_t sp_noise_event(sp_time_t start, sp_time_t end, sp_noise_event_config_t config, sp_event_t* out) {
  status_declare;
  sp_channel_count_t ci;
  sp_time_t duration;
  sp_event_t event;
  sp_random_state_t rs;
  sp_sample_t* state_noise;
  sp_sample_t* state_temp;
  sp_declare_event(group);
  duration = (end - start);
  config.resolution = (config.resolution ? config.resolution : 96);
  config.resolution = sp_min((config.resolution), duration);
  rs = sp_random_state_new((sp_time_random((&sp_default_random_state))));
  status_require((sp_group_new(start, (config.channels), (&group))));
  status_require((sp_event_memory_init(group, 2)));
  status_require((sp_malloc_type((config.resolution), sp_sample_t, (&state_noise))));
  sp_event_memory_add(group, state_noise);
  status_require((sp_malloc_type((config.resolution), sp_sample_t, (&state_temp))));
  sp_event_memory_add(group, state_temp);
  for (ci = 0; (ci < config.channels); ci += 1) {
    if (((config.channel_config)[ci]).mute) {
      continue;
    };
    status_require((sp_noise_event_channel(duration, config, ci, rs, state_noise, state_temp, (&event))));
    sp_group_add(group, event);
  };
  *out = group;
exit:
  if (status_is_failure) {
    sp_group_free((&group));
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
void sp_cheap_noise_event_free(sp_event_t* a) {
  sp_cheap_noise_event_state_t* s = a->state;
  sp_cheap_filter_state_free((&(s->filter_state)));
  free((a->state));
  sp_event_memory_free(a);
}
status_t sp_cheap_noise_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, void* state) {
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
  sp = state;
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
  sp_cheap_noise_event_state_t* state;
  sp_channel_config_t channel_config;
  sp_declare_event(event);
  sp_declare_cheap_filter_state(filter_state);
  state = 0;
  channel_config = (config.channel_config)[channel];
  status_require((sp_malloc_type(1, sp_cheap_noise_event_state_t, (&state))));
  status_require((sp_cheap_filter_state_new((config.resolution), (config.passes), (&filter_state))));
  (*state).amp = (channel_config.use ? channel_config.amp : config.amp);
  (*state).amod = ((channel_config.use && channel_config.amod) ? channel_config.amod : config.amod);
  (*state).cut = config.cut;
  (*state).cut_mod = config.cut_mod;
  (*state).type = config.type;
  (*state).passes = config.passes;
  (*state).q_factor = config.q_factor;
  (*state).resolution = config.resolution;
  (*state).random_state = rs;
  (*state).channel = channel;
  (*state).filter_state = filter_state;
  (*state).noise = state_noise;
  (*state).temp = state_temp;
  event.state = state;
  event.start = (channel_config.use ? channel_config.delay : 0);
  event.end = (channel_config.use ? (duration + channel_config.delay) : duration);
  event.prepare = 0;
  event.generate = sp_cheap_noise_event_generate;
  event.free = sp_cheap_noise_event_free;
  *out = event;
exit:
  if (status_is_failure) {
    if (state) {
      free(state);
    };
    sp_cheap_filter_state_free((&filter_state));
  };
  status_return;
}

/** an event for noise filtered by a state-variable filter.
   lower processing costs even when parameters change with high resolution.
   multiple passes almost multiply performance costs.
   memory for event.state will be allocated and then owned by the caller */
status_t sp_cheap_noise_event(sp_time_t start, sp_time_t end, sp_cheap_noise_event_config_t config, sp_event_t* out) {
  status_declare;
  sp_channel_count_t ci;
  sp_event_t event;
  sp_random_state_t rs;
  sp_sample_t* state_noise;
  sp_sample_t* state_temp;
  sp_declare_event(group);
  config.passes = (config.passes ? config.passes : 1);
  config.resolution = (config.resolution ? config.resolution : 96);
  config.resolution = sp_min((config.resolution), (end - start));
  rs = sp_random_state_new((sp_time_random((&sp_default_random_state))));
  status_require((sp_group_new(start, (config.channels), (&group))));
  status_require((sp_event_memory_init(group, 2)));
  status_require((sp_malloc_type((config.resolution), sp_sample_t, (&state_noise))));
  sp_event_memory_add(group, state_noise);
  status_require((sp_malloc_type((config.resolution), sp_sample_t, (&state_temp))));
  sp_event_memory_add(group, state_temp);
  for (ci = 0; (ci < config.channels); ci += 1) {
    if (((config.channel_config)[ci]).mute) {
      continue;
    };
    status_require((sp_cheap_noise_event_channel((end - start), config, ci, rs, state_noise, state_temp, (&event))));
    sp_group_add(group, event);
  };
  *out = group;
exit:
  if (status_is_failure) {
    sp_group_free((&group));
  };
  status_return;
}
void sp_event_memory_free(sp_event_t* event) {
  sp_time_half_t i;
  sp_memory_t m;
  for (i = 0; (i < event->memory_used); i += 1) {
    m = (event->memory)[i];
    (m.free)((m.data));
  };
  free((event->memory));
}
void sp_map_event_free(sp_event_t* a) {
  sp_map_event_state_t* s;
  s = a->state;
  if (s->event.free) {
    (s->event.free)((&(s->event)));
  };
  free(s);
  sp_event_memory_free(a);
}
status_t sp_map_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, void* state) {
  status_declare;
  sp_map_event_state_t* s;
  s = state;
  status_require_return(((s->event.generate)(start, end, out, (&(s->event)))));
  return (((s->generate)(start, end, out, out, (s->state))));
}

/** creates temporary output, lets event write to it, and passes the temporary output to a user function */
status_t sp_map_event_isolated_generate(sp_time_t start, sp_time_t end, sp_block_t out, void* state) {
  status_declare;
  sp_map_event_state_t* s;
  sp_block_t temp_out;
  status_require_return((sp_block_new((out.channels), (out.size), (&temp_out))));
  s = state;
  status_require(((s->event.generate)(start, end, temp_out, (s->event.state))));
  status_require(((s->generate)(start, end, temp_out, out, (s->state))));
exit:
  sp_block_free(temp_out);
  status_return;
}

/** f: map function (start end sp_block_t:in sp_block_t:out void*:state)
   state: custom state value passed to f.
   the wrapped event will be freed with the map-event.
   isolate: use a dedicated output buffer for event
     events can be wrapped in multiple sp_map_event with an isolated sp_map_event on top that
     finally writes to main out to mix with other events
   use cases: filter chains, post processing */
status_t sp_map_event(sp_event_t event, sp_map_event_generate_t f, void* state, uint8_t isolate, sp_event_t* out) {
  status_declare;
  sp_map_event_state_t* s;
  sp_declare_event(e);
  status_require((sp_malloc_type(1, sp_map_event_state_t, (&s))));
  s->event = event;
  s->state = state;
  s->generate = f;
  e.state = s;
  e.start = event.start;
  e.end = event.end;
  e.generate = (isolate ? sp_map_event_isolated_generate : sp_map_event_generate);
  e.free = sp_map_event_free;
  *out = e;
exit:
  status_return;
}
