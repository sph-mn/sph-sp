void sp_event_sort_swap(void* a, ssize_t b, ssize_t c) {
  sp_event_t d;
  d = ((sp_event_t*)(a))[b];
  ((sp_event_t*)(a))[b] = ((sp_event_t*)(a))[c];
  ((sp_event_t*)(a))[c] = d;
}
uint8_t sp_event_sort_less_p(void* a, ssize_t b, ssize_t c) { return (((((sp_event_t*)(a))[b]).start < (((sp_event_t*)(a))[c]).start)); }
void sp_seq_events_prepare(sp_event_t* data, sp_time_t size) { quicksort(sp_event_sort_less_p, sp_event_sort_swap, data, 0, (size - 1)); }
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
      (e.free)(ep);
      array4_forward((*events));
    } else if (end <= e.start) {
      break;
    } else if (e.end > start) {
      if (e.prepare) {
        status_require_return(((e.prepare)(ep)));
        ep->prepare = 0;
        e.state = ep->state;
      };
      (e.generate)(((start > e.start) ? (start - e.start) : 0), (sp_min(end, (e.end)) - e.start), ((e.start > start) ? sp_block_with_offset(out, (e.start - start)) : out), (e.state));
    };
  };
  status_return;
}
/** free all events starting from the current event. only needed if sp_seq will not further process, and thereby free, the events */
void sp_events_free(sp_events_t* events) {
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
  (a->event->generate)((a->start), (a->end), (a->out), (a->event->state));
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
  for (i = events->current; (i < events->used); i += 1) {
    ep = (events->data + i);
    if (end <= ep->start) {
      break;
    } else if (ep->end > start) {
      active += 1;
    };
  };
  status_require((sph_helper_malloc((active * sizeof(sp_seq_future_t)), (&sf_array))));
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
void sp_wave_event_free(sp_event_t* a) {
  free((a->state));
  sp_event_memory_free(a);
}
void sp_wave_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_time_t i;
  sp_wave_event_state_t* state_pointer;
  sp_wave_event_state_t state;
  state_pointer = ((sp_wave_event_state_t*)(event->state));
  state = *state_pointer;
  for (i = 0; (i < (end - start)); i += 1) {
    out[state.config.channel_index][i] += ((state.config.amod)[(start + i)] * (state.config.wvf)[state.config.phs]);
    state.config.phs += sp_modvalue((state.config.frq), (state.config.fmod), mod_index);
    if (state.config.phs >= state.config.wvf_size) {
      state.config.phs = (state.config.phs % state.config.wvf_size);
    };
  };
  state_pointer->config.phs = state.config.phs;
}
/** create an event playing waveforms from an array.
   channel config:
   * settings override the main settings if not 0
   * amod multiplies main amod
   * event end will be longer if delay is used
   if modulation arrays are null, then the fixed value will be used (frq, channel amp) */
status_t sp_wave_event(sp_time_t start, sp_time_t end, sp_wave_event_config_t config, sp_event_t* out) {
  status_declare;
  sp_wave_event_state_t* state;
  status_require((sph_helper_calloc((sizeof(sp_wave_event_state_t)), (&state))));
  state->config = config;
  out->start = start;
  out->end = delayed_end;
  out->f = sp_wave_event_generate;
  out->free = sp_wave_event_free;
  out->state = state;
exit:
  status_return;
}
typedef struct {
  sp_noise_event_config_t config;
  sp_convolution_filter_state_t* filter_state;
  sp_sample_t* noise;
  sp_sample_t* temp;
} sp_noise_event_state_t;
void sp_noise_event_free(sp_event_t* a) {
  sp_noise_event_state_t* s = a->state;
  free((s->noise));
  free((s->temp));
  sp_convolution_filter_state_free((s->filter_state));
  free((a->state));
  sp_event_memory_free(a);
}
void sp_noise_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_sample_t* amod;
  sp_time_t block_count;
  sp_time_t block_i;
  sp_time_t block_offset;
  sp_time_t block_rest;
  sp_time_t chn_i;
  sp_time_t duration;
  sp_time_t i;
  sp_noise_event_state_t s;
  sp_time_t t;
  /* update filter arguments only every resolution number of samples */
  s = *((sp_noise_event_state_t*)(event->state));
  duration = (end - start);
  block_count = ((duration == s.config.resolution) ? 1 : sp_cheap_floor_positive((duration / s.config.resolution)));
  block_rest = (duration % s.config.resolution);
  for (block_i = 0; (block_i <= block_count); block_i = (1 + block_i)) {
    block_offset = (s.config.resolution * block_i);
    t = (start + block_offset);
    duration = ((block_count == block_i) ? block_rest : s.config.resolution);
    sp_samples_random((&(s.config.random_state)), duration, (s.noise));
    sp_windowed_sinc_bp_br((s.noise), duration, (sp_modvalue((s.config.cutl), (s.config.cutl_mod), t)), (sp_modvalue((s.config.cuth), (s.config.cuth_mod), t)), (sp_modvalue((s.config.trnl), (s.config.trnl_mod), t)), (sp_modvalue((s.config.trnh), (s.config.trnh_mod), t)), (s.config.is_reject), (&(s.filter_state)), (s.temp));
    for (chn_i = 0; (chn_i < out.channels); chn_i += 1) {
      amod = (s.config.amod)[chn_i];
      if (!amod) {
        continue;
      };
      for (i = 0; (i < duration); i += 1) {
        (out.samples)[chn_i][(block_offset + i)] += (amod[(t + i)] * (s.temp)[i]);
      };
    };
  };
}
/** an event for noise filtered by a windowed-sinc filter.
   very processing intensive if parameters change with low resolution.
   memory for event.state will be allocated and then owned by the caller */
status_t sp_noise_event(sp_time_t start, sp_time_t end, sp_noise_event_config_t config, sp_event_t* out_event) {
  status_declare;
  sp_sample_t* temp;
  sp_sample_t* temp_noise;
  sp_time_t ir_len;
  sp_noise_event_state_t* state;
  sp_sample_t trnl;
  sp_sample_t trnh;
  sp_declare_event(event);
  config.resolution = sp_min((config.resolution), (end - start));
  trnl = sp_modvalue((config.trnl), (config.trnl_mod), 0);
  trnh = sp_modvalue((config.trnh), (config.trnh_mod), 0);
  status_require((sph_helper_malloc((sizeof(sp_noise_event_state_t)), (&state))));
  status_require((sph_helper_malloc((config.resolution * sizeof(sp_sample_t)), (&(state->noise)))));
  status_require((sph_helper_malloc((config.resolution * sizeof(sp_sample_t)), (&(state->temp)))));
  /* the result shows a small delay, circa 40 samples for transition 0.07. the size seems to be related to ir-len.
the state is initialised with one unused call to skip the delay.
an added benefit is that the filter-state setup malloc is checked */
  ir_len = sp_windowed_sinc_lp_hp_ir_length((sp_min(trnl, trnh)));
  state->filter_state = 0;
  status_require((sph_helper_malloc((ir_len * sizeof(sp_sample_t)), (&temp))));
  status_require((sph_helper_malloc((ir_len * sizeof(sp_sample_t)), (&temp_noise))));
  sp_samples_random((&(config.random_state)), ir_len, temp_noise);
  status_require((sp_windowed_sinc_bp_br(temp_noise, ir_len, (sp_modvalue((config.cutl), (config.cutl_mod), 0)), (sp_modvalue((config.cuth), (config.cuth_mod), 0)), trnl, trnh, (config.is_reject), (&(state->filter_state)), temp)));
  free(temp);
  free(temp_noise);
  state->config = config;
  event.start = start;
  event.end = end;
  event.generate = sp_noise_event_generate;
  event.free = sp_noise_event_free;
  event.state = state;
  *out_event = event;
exit:
  status_return;
}
typedef struct {
  sp_cheap_noise_event_config_t config;
  sp_cheap_filter_state_t filter_state;
  sp_sample_t* noise;
  sp_sample_t* temp;
} sp_cheap_noise_event_state_t;
void sp_cheap_noise_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_sample_t* amod;
  sp_time_t block_count;
  sp_time_t block_i;
  sp_time_t block_offset;
  sp_time_t block_rest;
  sp_time_t chn_i;
  sp_time_t duration;
  sp_time_t i;
  sp_cheap_noise_event_state_t s;
  sp_time_t t;
  /* update filter arguments only every resolution number of samples */
  s = *((sp_cheap_noise_event_state_t*)(event->state));
  if (!s.config.resolution) {
    exit(1);
  };
  duration = (end - start);
  block_count = ((duration == s.config.resolution) ? 1 : sp_cheap_floor_positive((duration / s.config.resolution)));
  block_rest = (duration % s.config.resolution);
  /* total block count is block-count plus rest-block */
  for (block_i = 0; (block_i <= block_count); block_i = (1 + block_i)) {
    block_offset = (s.config.resolution * block_i);
    t = (start + block_offset);
    duration = ((block_count == block_i) ? block_rest : s.config.resolution);
    sp_samples_random((&(s.config.random_state)), duration, (s.noise));
    sp_cheap_filter((s.config.type), (s.noise), duration, (sp_modvalue((s.config.cut), (s.config.cut_mod), t)), (s.config.passes), (s.config.q_factor), (&(s.filter_state)), (s.temp));
    for (chn_i = 0; (chn_i < out.channels); chn_i = (1 + chn_i)) {
      amod = (s.config.amod)[chn_i];
      if (!amod) {
        continue;
      };
      for (i = 0; (i < duration); i = (1 + i)) {
        (out.samples)[chn_i][(block_offset + i)] += (amod[(t + i)] * (s.temp)[i]);
      };
    };
  };
}
void sp_cheap_noise_event_free(sp_event_t* a) {
  sp_cheap_noise_event_state_t* s = a->state;
  free((s->noise));
  free((s->temp));
  sp_cheap_filter_state_free((&(s->filter_state)));
  free((a->state));
  sp_event_memory_free(a);
}
/** an event for noise filtered by a state-variable filter.
   lower processing costs even when parameters change with high resolution.
   multiple passes almost multiply performance costs.
   memory for event.state will be allocated and then owned by the caller */
status_t sp_cheap_noise_event(sp_time_t start, sp_time_t end, sp_cheap_noise_event_config_t config, sp_event_t* out_event) {
  status_declare;
  sp_cheap_noise_event_state_t* state;
  sp_declare_event(event);
  config.resolution = sp_min((config.resolution), (end - start));
  status_require((sph_helper_malloc((sizeof(sp_cheap_noise_event_state_t)), (&state))));
  status_require((sph_helper_malloc((config.resolution * sizeof(sp_sample_t)), (&(state->noise)))));
  status_require((sph_helper_malloc((config.resolution * sizeof(sp_sample_t)), (&(state->temp)))));
  status_require((sp_cheap_filter_state_new((config.resolution), (config.passes), (&(state->filter_state)))));
  state->config = config;
  event.start = start;
  event.end = end;
  event.generate = sp_cheap_noise_event_generate;
  event.free = sp_cheap_noise_event_free;
  event.state = state;
  *out_event = event;
exit:
  status_return;
}
/** to free events while the group itself is not finished */
#define sp_group_event_free_events(events, end) \
  while ((array4_in_range(events) && ((array4_get(events)).end <= end))) { \
    if ((array4_get(events)).free) { \
      ((array4_get(events)).free)((array4_get_address(events))); \
    }; \
    array4_forward(events); \
  }
/** events.current is used to track freed events */
void sp_group_event_free(sp_event_t* a) {
  sp_events_t s = *((sp_events_t*)(a->state));
  while (array4_in_range(s)) {
    if ((array4_get(s)).free) {
      ((array4_get(s)).free)((array4_get_address(s)));
    };
    array4_forward(s);
  };
  array4_free(s);
  free((a->state));
  sp_event_memory_free(a);
}
/** free past events early, they might be sub-group trees */
void sp_group_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_events_t* s = event->state;
  sp_seq(start, end, out, (array4_get_address((*s))), (array4_right_size((*s))));
  sp_group_event_free_events((*s), end);
}
/** can be used in place of sp-group-event-generate.
   seq-parallel can fail if there is not enough memory, but this is ignored currently */
void sp_group_event_parallel_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_events_t* s = event->state;
  sp_seq_parallel(start, end, out, (array4_get_address((*s))), (array4_right_size((*s))));
  sp_group_event_free_events((*s), end);
}
status_t sp_group_new(sp_time_t start, sp_group_size_t event_size, sp_event_t* out) {
  status_declare;
  sp_events_t* s;
  memreg_init(2);
  status_require((sph_helper_malloc((sizeof(sp_events_t)), (&s))));
  memreg_add(s);
  if (sp_events_new(event_size, s)) {
    sp_memory_error;
  };
  memreg_add((s->data));
  (*out).state = s;
  (*out).start = start;
  (*out).end = start;
  (*out).f = sp_group_event_generate;
  (*out).free = sp_group_event_free;
exit:
  if (status_is_failure) {
    memreg_free;
  };
  return (status);
}
void sp_group_append(sp_event_t* a, sp_event_t event) {
  event.start += a->end;
  event.end += a->end;
  sp_group_add((*a), event);
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
void sp_event_array_set_null(sp_event_t* a, sp_time_t size) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    sp_event_set_null((a[i]));
  };
}
typedef void (*sp_map_event_generate_t)(sp_time_t, sp_time_t, sp_block_t, sp_block_t, sp_event_t*, void*);
typedef struct {
  sp_event_t event;
  sp_map_event_generate_t generate;
  void* state;
} sp_map_event_state_t;
void sp_map_event_free(sp_event_t* a) {
  sp_map_event_state_t* s;
  s = a->state;
  if (s->event.free) {
    (s->event.free)((&(s->event)));
  };
  free((a->state));
}
/** creates temporary output, lets event write to it, and passes the result to a user function */
void sp_map_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_map_event_state_t* s;
  s = event->state;
  (s->event.generate)(start, end, out, (&(s->event)));
  (s->f)(start, end, out, out, (&(s->event)), (s->state));
}
/** creates temporary output, lets event write to it, and passes the result to a user function */
void sp_map_event_isolated_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_map_event_state_t* s;
  sp_block_t temp_out;
  status_declare;
  status = sp_block_new((out.channels), (out.size), (&temp_out));
  if (status_is_failure) {
    return;
  };
  s = event->state;
  (s->event.generate)(start, end, temp_out, (&(s->event)));
  (s->generate)(start, end, temp_out, out, (&(s->event)), (s->state));
  sp_block_free(temp_out);
}
/** f: function(start end sp_block_t:in sp_block_t:out sp_event_t*:event void*:state)
   isolate: use a dedicated output buffer for event
     events can be wrapped in multiple sp_map_event with an isolated sp_map_event on top that
     finally writes to main out to mix with other events
   use cases: filter chains, post processing */
status_t sp_map_event(sp_event_t event, sp_map_event_generate_t f, void* state, uint8_t isolate, sp_event_t* out) {
  status_declare;
  sp_map_event_state_t* s;
  sp_declare_event(e);
  status_require((sph_helper_malloc((sizeof(sp_map_event_state_t)), (&s))));
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
