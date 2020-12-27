#define sp_modvalue(fixed, array, index) (array ? array[index] : fixed)
void sp_event_sort_swap(void* a, ssize_t b, ssize_t c) {
  sp_event_t d;
  d = ((sp_event_t*)(a))[b];
  ((sp_event_t*)(a))[b] = ((sp_event_t*)(a))[c];
  ((sp_event_t*)(a))[c] = d;
}
uint8_t sp_event_sort_less_p(void* a, ssize_t b, ssize_t c) { return (((((sp_event_t*)(a))[b]).start < (((sp_event_t*)(a))[c]).start)); }
void sp_seq_events_prepare(sp_event_t* data, sp_time_t size) { quicksort(sp_event_sort_less_p, sp_event_sort_swap, data, 0, (size - 1)); }
/** event arrays must have been prepared/sorted with sp_seq_event_prepare for seq to work correctly.
   event functions receive event relative start/end time, and output blocks begin at event start.
   as for paths, start is inclusive, end is exclusive, so that 0..100 and 100..200 attach seamless.
   $size is the number of events */
void sp_seq(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* events, sp_time_t size) {
  sp_time_t e_out_start;
  sp_event_t e;
  sp_time_t e_start;
  sp_time_t e_end;
  sp_time_t i;
  for (i = 0; (i < size); i = (1 + i)) {
    e = events[i];
    if (e.end <= start) {
      continue;
    } else if (end <= e.start) {
      break;
    } else {
      e_out_start = ((e.start > start) ? (e.start - start) : 0);
      e_start = ((start > e.start) ? (start - e.start) : 0);
      e_end = (sp_min(end, (e.end)) - e.start);
      (e.f)(e_start, e_end, (e_out_start ? sp_block_with_offset(out, e_out_start) : out), (&e));
    };
  };
}
void sp_events_array_free(sp_event_t* events, sp_time_t size) {
  sp_time_t i;
  void (*event_free)(struct sp_event_t*);
  for (i = 0; (i < size); i = (1 + i)) {
    event_free = (events + i)->free;
    if (event_free) {
      event_free((events + i));
    };
  };
}
typedef struct {
  sp_time_t start;
  sp_time_t end;
  sp_time_t out_start;
  sp_block_t out;
  sp_event_t* event;
  future_t future;
} sp_seq_future_t;
void* sp_seq_parallel_future_f(void* data) {
  sp_seq_future_t* a = data;
  (a->event->f)((a->start), (a->end), (a->out), (a->event));
}
/** like sp_seq but evaluates events in parallel */
status_t sp_seq_parallel(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* events, sp_time_t size) {
  status_declare;
  sp_time_t e_out_start;
  sp_event_t e;
  sp_time_t e_start;
  sp_time_t e_end;
  sp_channel_count_t chn_i;
  sp_seq_future_t* seq_futures;
  sp_seq_future_t* sf;
  sp_time_t i;
  sp_time_t ftr_i;
  sp_time_t active_count;
  seq_futures = 0;
  active_count = 0;
  /* count events to allocate future object memory */
  for (i = 0; (i < size); i += 1) {
    e = events[i];
    if (e.end <= start) {
      continue;
    } else if (end <= e.start) {
      break;
    } else {
      active_count += 1;
    };
  };
  status_require((sph_helper_malloc((active_count * sizeof(sp_seq_future_t)), (&seq_futures))));
  /* parallelise */
  ftr_i = 0;
  for (i = 0; (i < size); i += 1) {
    e = events[i];
    if (e.end <= start) {
      continue;
    } else if (end <= e.start) {
      break;
    } else {
      sf = (seq_futures + ftr_i);
      e_out_start = ((e.start > start) ? (e.start - start) : 0);
      e_start = ((start > e.start) ? (start - e.start) : 0);
      e_end = (sp_min(end, (e.end)) - e.start);
      ftr_i += 1;
      status_require((sp_block_new((out.channels), (e_end - e_start), (&(sf->out)))));
      sf->start = e_start;
      sf->end = e_end;
      sf->out_start = e_out_start;
      sf->event = (i + events);
      future_new(sp_seq_parallel_future_f, sf, (&(sf->future)));
    };
  };
  /* merge */
  for (ftr_i = 0; (ftr_i < active_count); ftr_i += 1) {
    sf = (ftr_i + seq_futures);
    touch((&(sf->future)));
    for (chn_i = 0; (chn_i < out.channels); chn_i = (1 + chn_i)) {
      for (i = 0; (i < sf->out.size); i = (1 + i)) {
        ((out.samples)[chn_i])[(sf->out_start + i)] += ((sf->out.samples)[chn_i])[i];
      };
    };
    sp_block_free((sf->out));
  };
exit:
  free(seq_futures);
  status_return;
}
sp_wave_event_state_t sp_wave_event_state_1(sp_wave_state_t wave_state) {
  sp_wave_event_state_t a;
  a.channels = 1;
  (a.wave_states)[0] = wave_state;
  return (a);
}
sp_wave_event_state_t sp_wave_event_state_2(sp_wave_state_t wave_state_1, sp_wave_state_t wave_state_2) {
  sp_wave_event_state_t a;
  a.channels = 2;
  (a.wave_states)[0] = wave_state_1;
  (a.wave_states)[1] = wave_state_2;
  return (a);
}
void sp_wave_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_channel_count_t chn_i;
  sp_wave_event_state_t* state;
  sp_wave_state_t* wave_state;
  state = ((sp_wave_event_state_t*)(event->state));
  for (chn_i = 0; (chn_i < state->channels); chn_i = (1 + chn_i)) {
    wave_state = (state->wave_states + chn_i);
    if (!wave_state->amod) {
      continue;
    };
    sp_wave(start, (end - start), wave_state, ((out.samples)[chn_i]));
  };
}
void sp_wave_event_free(sp_event_t* a) {
  free((a->state));
  sp_event_memory_free(a);
}
/** in wave_event_state, unset wave states or wave states with amp set to null will generate no output.
   the state struct is copied and freed with event.free so that stack declared structs can be used.
   in wave-event-state, wave-state.amod can be 0 to skip channels.
   sp_wave_event, sp_wave_event_f and sp_wave_event_free are a good example for custom events */
status_t sp_wave_event(sp_time_t start, sp_time_t end, sp_wave_event_state_t state, sp_event_t* out) {
  status_declare;
  sp_wave_state_t* event_state;
  event_state = 0;
  status_require((sph_helper_calloc((sizeof(sp_wave_event_state_t)), (&event_state))));
  memcpy(event_state, (&state), (sizeof(sp_wave_event_state_t)));
  out->start = start;
  out->end = end;
  out->f = sp_wave_event_f;
  out->free = sp_wave_event_free;
  out->state = event_state;
exit:
  if (status_is_failure) {
    free(event_state);
  };
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
void sp_noise_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
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
  event.f = sp_noise_event_f;
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
void sp_cheap_noise_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
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
  event.f = sp_cheap_noise_event_f;
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
void sp_group_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_events_t* s = event->state;
  sp_seq(start, end, out, (array4_get_address((*s))), (array4_right_size((*s))));
  sp_group_event_free_events((*s), end);
}
/** can be used in place of sp-group-event-f.
   seq-parallel can fail if there is not enough memory, but this is ignored currently */
void sp_group_event_parallel_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
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
  (*out).f = sp_group_event_f;
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
typedef void (*sp_map_event_f_t)(sp_time_t, sp_time_t, sp_block_t, sp_block_t, sp_event_t*, void*);
typedef struct {
  sp_event_t event;
  sp_map_event_f_t f;
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
void sp_map_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_map_event_state_t* s;
  s = event->state;
  (s->event.f)(start, end, out, (&(s->event)));
  (s->f)(start, end, out, out, (&(s->event)), (s->state));
}
/** creates temporary output, lets event write to it, and passes the result to a user function */
void sp_map_event_isolated_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_map_event_state_t* s;
  sp_block_t temp_out;
  status_declare;
  status = sp_block_new((out.channels), (out.size), (&temp_out));
  if (status_is_failure) {
    return;
  };
  s = event->state;
  (s->event.f)(start, end, temp_out, (&(s->event)));
  (s->f)(start, end, temp_out, out, (&(s->event)), (s->state));
  sp_block_free(temp_out);
}
/** f: function(start end sp_block_t:in sp_block_t:out sp_event_t*:event void*:state)
   isolate: use a dedicated output buffer for event
     events can be wrapped in multiple sp_map_event with an isolated sp_map_event on top that
     finally writes to main out to mix with other events
   use cases: filter chains, post processing */
status_t sp_map_event(sp_event_t event, sp_map_event_f_t f, void* state, uint8_t isolate, sp_event_t* out) {
  status_declare;
  sp_map_event_state_t* s;
  sp_declare_event(e);
  status_require((sph_helper_malloc((sizeof(sp_map_event_state_t)), (&s))));
  s->event = event;
  s->state = state;
  s->f = f;
  e.state = s;
  e.start = event.start;
  e.end = event.end;
  e.f = (isolate ? sp_map_event_isolated_f : sp_map_event_f);
  e.free = sp_map_event_free;
  *out = e;
exit:
  status_return;
}
