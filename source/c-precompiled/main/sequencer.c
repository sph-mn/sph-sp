void sp_event_sort_swap(void* a, ssize_t b, ssize_t c) {
  sp_event_t d;
  d = ((sp_event_t*)(a))[b];
  ((sp_event_t*)(a))[b] = ((sp_event_t*)(a))[c];
  ((sp_event_t*)(a))[c] = d;
}
uint8_t sp_event_sort_less_p(void* a, ssize_t b, ssize_t c) { return (((((sp_event_t*)(a))[b]).start < (((sp_event_t*)(a))[c]).start)); }
void sp_seq_events_prepare(sp_event_t* data, sp_time_t size) { quicksort(sp_event_sort_less_p, sp_event_sort_swap, data, 0, (size - 1)); }
/** event arrays must have been prepared/sorted with sp-seq-event-prepare for seq to work correctly */
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
      e_end = (((e.end < end) ? e.end : end) - e.start);
      (e.f)(e_start, e_end, (e_out_start ? sp_block_with_offset(out, e_out_start) : out), (&e));
    };
  };
}
void sp_events_free(sp_event_t* events, sp_time_t size) {
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
  sp_channels_t channel_i;
  sp_time_t events_start;
  sp_time_t events_count;
  sp_seq_future_t* seq_futures;
  sp_seq_future_t* sf;
  sp_time_t i;
  sp_time_t e_i;
  seq_futures = 0;
  /* select active events */
  for (i = 0, events_start = 0, events_count = 0; (i < size); i = (1 + i)) {
    e = events[i];
    if (e.end <= start) {
      events_start = (1 + events_start);
    } else if (end <= e.start) {
      break;
    } else {
      events_count = (1 + events_count);
    };
  };
  status_require((sph_helper_malloc((events_count * sizeof(sp_seq_future_t)), (&seq_futures))));
  /* parallelise */
  for (i = 0; (i < events_count); i = (1 + i)) {
    e = events[(events_start + i)];
    sf = (i + seq_futures);
    e_out_start = ((e.start > start) ? (e.start - start) : 0);
    e_start = ((start > e.start) ? (start - e.start) : 0);
    e_end = (((e.end < end) ? e.end : end) - e.start);
    status_require((sp_block_new((out.channels), (e_end - e_start), (&(sf->out)))));
    sf->start = e_start;
    sf->end = e_end;
    sf->out_start = e_out_start;
    sf->event = (events_start + i + events);
    future_new(sp_seq_parallel_future_f, sf, (&(sf->future)));
  };
  /* merge */
  for (e_i = 0; (e_i < events_count); e_i = (1 + e_i)) {
    sf = (e_i + seq_futures);
    touch((&(sf->future)));
    for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
      for (i = 0; (i < sf->out.size); i = (1 + i)) {
        ((out.samples)[channel_i])[(sf->out_start + i)] = (((out.samples)[channel_i])[(sf->out_start + i)] + ((sf->out.samples)[channel_i])[i]);
      };
    };
    sp_block_free((sf->out));
  };
exit:
  free(seq_futures);
  status_return;
}
void sp_synth_event_free(sp_event_t* a) {
  free((((sp_synth_event_state_t*)(a->state))->state));
  free((a->state));
}
void sp_synth_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_synth_event_state_t* s = event->state;
  sp_synth(out, start, (end - start), (s->config_len), (s->config), (s->state));
}
/** memory for event.state is allocated and then owned by the caller.
   config is copied to event.state.config to allow config to be a stack array. the copy will be freed by event.free */
status_t sp_synth_event(sp_time_t start, sp_time_t end, sp_channels_t channel_count, sp_time_t config_len, sp_synth_partial_t* config, sp_event_t* out_event) {
  status_declare;
  sp_event_t e;
  sp_synth_event_state_t* state;
  state = 0;
  status_require((sph_helper_malloc((channel_count * sizeof(sp_synth_event_state_t)), (&state))));
  status_require((sp_synth_state_new(channel_count, config_len, config, (&(state->state)))));
  memcpy((state->config), config, (config_len * sizeof(sp_synth_partial_t)));
  state->config_len = config_len;
  e.start = start;
  e.end = end;
  e.f = sp_synth_event_f;
  e.free = sp_synth_event_free;
  e.state = state;
  *out_event = e;
exit:
  if (status_is_failure) {
    free(state);
  };
  status_return;
}
typedef struct {
  sp_sample_t** amp;
  sp_sample_t* cut_h;
  sp_sample_t* cut_l;
  sp_convolution_filter_state_t* filter_state;
  uint8_t is_reject;
  sp_sample_t* noise;
  sp_random_state_t random_state;
  sp_time_t resolution;
  sp_sample_t* temp;
  sp_sample_t* trn_h;
  sp_sample_t* trn_l;
} sp_noise_event_state_t;
void sp_noise_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_noise_event_state_t* s;
  sp_time_t block_count;
  sp_time_t i;
  sp_time_t block_i;
  sp_time_t channel_i;
  sp_time_t t;
  sp_time_t block_offset;
  sp_time_t duration;
  sp_time_t block_rest;
  /* update filter arguments only every resolution number of samples */
  s = event->state;
  duration = (end - start);
  block_count = ((duration == s->resolution) ? 1 : sp_cheap_floor_positive((duration / s->resolution)));
  block_rest = (duration % s->resolution);
  /* total block count is block-count plus rest-block */
  for (block_i = 0; (block_i <= block_count); block_i = (1 + block_i)) {
    block_offset = (s->resolution * block_i);
    t = (start + block_offset);
    duration = ((block_count == block_i) ? block_rest : s->resolution);
    sp_random_samples((&(s->random_state)), duration, (s->noise));
    sp_windowed_sinc_bp_br((s->noise), duration, ((s->cut_l)[t]), ((s->cut_h)[t]), ((s->trn_l)[t]), ((s->trn_h)[t]), (s->is_reject), (&(s->filter_state)), (s->temp));
    for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
      for (i = 0; (i < duration); i = (1 + i)) {
        ((out.samples)[channel_i])[(block_offset + i)] = (((out.samples)[channel_i])[(block_offset + i)] + (((s->amp)[channel_i])[(block_offset + i)] * (s->temp)[i]));
      };
    };
  };
}
void sp_noise_event_free(sp_event_t* a) {
  sp_noise_event_state_t* s = a->state;
  free((s->noise));
  free((s->temp));
  sp_convolution_filter_state_free((s->filter_state));
  free((a->state));
}
/** an event for noise filtered by a windowed-sinc filter.
   very processing intensive when parameters change with low resolution.
   memory for event.state will be allocated and then owned by the caller */
status_t sp_noise_event(sp_time_t start, sp_time_t end, sp_sample_t** amp, sp_sample_t* cut_l, sp_sample_t* cut_h, sp_sample_t* trn_l, sp_sample_t* trn_h, uint8_t is_reject, sp_time_t resolution, sp_random_state_t random_state, sp_event_t* out_event) {
  status_declare;
  sp_sample_t* temp;
  sp_sample_t* temp_noise;
  sp_time_t ir_len;
  sp_event_t e;
  sp_noise_event_state_t* s;
  resolution = (resolution ? min(resolution, (end - start)) : 96);
  status_require((sph_helper_malloc((sizeof(sp_noise_event_state_t)), (&s))));
  status_require((sph_helper_malloc((resolution * sizeof(sp_sample_t)), (&(s->noise)))));
  status_require((sph_helper_malloc((resolution * sizeof(sp_sample_t)), (&(s->temp)))));
  /* the result shows a small delay, circa 40 samples for transition 0.07. the size seems to be related to ir-len.
the state is initialised with one unused call to skip the delay.
an added benefit is that the filter-state setup malloc is checked */
  ir_len = sp_windowed_sinc_lp_hp_ir_length((min((*trn_l), (*trn_h))));
  s->filter_state = 0;
  status_require((sph_helper_malloc((ir_len * sizeof(sp_sample_t)), (&temp))));
  status_require((sph_helper_malloc((ir_len * sizeof(sp_sample_t)), (&temp_noise))));
  sp_random_samples((&random_state), ir_len, temp_noise);
  status_require((sp_windowed_sinc_bp_br(temp_noise, ir_len, (*cut_l), (*cut_h), (*trn_l), (*trn_h), is_reject, (&(s->filter_state)), temp)));
  free(temp);
  free(temp_noise);
  s->cut_l = cut_l;
  s->cut_h = cut_h;
  s->trn_l = trn_l;
  s->trn_h = trn_h;
  s->is_reject = is_reject;
  s->resolution = resolution;
  s->random_state = random_state;
  s->amp = amp;
  e.start = start;
  e.end = end;
  e.f = sp_noise_event_f;
  e.free = sp_noise_event_free;
  e.state = s;
  *out_event = e;
exit:
  status_return;
}
typedef struct {
  sp_sample_t** amp;
  sp_sample_t* cut;
  sp_sample_t q_factor;
  sp_time_t passes;
  sp_cheap_filter_state_t filter_state;
  sp_state_variable_filter_t type;
  sp_sample_t* noise;
  sp_random_state_t random_state;
  sp_time_t resolution;
  sp_sample_t* temp;
} sp_cheap_noise_event_state_t;
void sp_cheap_noise_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_time_t block_count;
  sp_time_t block_i;
  sp_time_t block_offset;
  sp_time_t block_rest;
  sp_time_t channel_i;
  sp_time_t duration;
  sp_time_t i;
  sp_cheap_noise_event_state_t* s;
  sp_time_t t;
  /* update filter arguments only every resolution number of samples */
  s = event->state;
  duration = (end - start);
  block_count = ((duration == s->resolution) ? 1 : sp_cheap_floor_positive((duration / s->resolution)));
  block_rest = (duration % s->resolution);
  /* total block count is block-count plus rest-block */
  for (block_i = 0; (block_i <= block_count); block_i = (1 + block_i)) {
    block_offset = (s->resolution * block_i);
    t = (start + block_offset);
    duration = ((block_count == block_i) ? block_rest : s->resolution);
    sp_random_samples((&(s->random_state)), duration, (s->noise));
    sp_cheap_filter((s->type), (s->noise), duration, ((s->cut)[t]), (s->passes), (s->q_factor), 1, (&(s->filter_state)), (s->temp));
    for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
      for (i = 0; (i < duration); i = (1 + i)) {
        ((out.samples)[channel_i])[(block_offset + i)] = (((out.samples)[channel_i])[(block_offset + i)] + (((s->amp)[channel_i])[(block_offset + i)] * (s->temp)[i]));
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
}
/** an event for noise filtered by a state-variable filter. multiple passes currently not implemented.
   lower processing costs even when parameters change with high resolution.
   multiple passes almost multiply performance costs.
   memory for event.state will be allocated and then owned by the caller */
status_t sp_cheap_noise_event(sp_time_t start, sp_time_t end, sp_sample_t** amp, sp_state_variable_filter_t type, sp_sample_t* cut, sp_time_t passes, sp_sample_t q_factor, sp_time_t resolution, sp_random_state_t random_state, sp_event_t* out_event) {
  status_declare;
  sp_event_t e;
  sp_cheap_noise_event_state_t* s;
  resolution = (resolution ? min(resolution, (end - start)) : 96);
  status_require((sph_helper_malloc((sizeof(sp_cheap_noise_event_state_t)), (&s))));
  status_require((sph_helper_malloc((resolution * sizeof(sp_sample_t)), (&(s->noise)))));
  status_require((sph_helper_malloc((resolution * sizeof(sp_sample_t)), (&(s->temp)))));
  status_require((sp_cheap_filter_state_new(resolution, passes, (&(s->filter_state)))));
  s->cut = cut;
  s->q_factor = q_factor;
  s->passes = passes;
  s->resolution = resolution;
  s->random_state = random_state;
  s->amp = amp;
  s->type = type;
  e.start = start;
  e.end = end;
  e.f = sp_cheap_noise_event_f;
  e.free = sp_cheap_noise_event_free;
  e.state = s;
  *out_event = e;
exit:
  status_return;
}
void sp_group_event_free(sp_event_t* a) {
  sp_group_event_state_t s = *((sp_group_event_state_t*)(a->state));
  i_array_rewind((s.events));
  i_array_rewind((s.memory));
  while (i_array_in_range((s.events))) {
    ((s.events.current)->free)((s.events.current));
    i_array_forward((s.events));
  };
  memreg_heap_free_pointers((s.memory));
  i_array_free((s.events));
  i_array_free((s.memory));
  free((a->state));
}
void sp_group_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) { printf("group is running\n"); }
status_t sp_group_new(time_t start, sp_group_size_t event_size, sp_group_size_t memory_size, sp_event_t* out) {
  status_declare;
  sp_group_event_state_t* s;
  memreg_init(2);
  status_require((sph_helper_malloc((sizeof(sp_group_event_state_t)), (&s))));
  memreg_add(s);
  if (sp_events_new(event_size, (&(s->events)))) {
    sp_memory_error;
  };
  memreg_add((s->events.start));
  if (memreg_heap_new(memory_size, (&(s->memory)))) {
    sp_memory_error;
  };
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
  event.start = a->end;
  sp_group_add((*a), event);
}
