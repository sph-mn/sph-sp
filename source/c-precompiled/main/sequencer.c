void sp_event_sort_swap(void* a, void* b) {
  sp_event_t c;
  c = *((sp_event_t*)(a));
  *((sp_event_t*)(a)) = *((sp_event_t*)(b));
  *((sp_event_t*)(b)) = c;
};
uint8_t sp_event_sort_less_p(void* a, void* b) { return ((((sp_event_t*)(a))->start < ((sp_event_t*)(b))->start)); };
void sp_seq_events_prepare(sp_events_t a) { quicksort(sp_event_sort_less_p, sp_event_sort_swap, (sizeof(sp_event_t)), (a.data), (a.size)); };
/** event arrays must have been prepared/sorted with sp-seq-event-prepare for seq to work correctly */
void sp_seq(sp_count_t start, sp_count_t end, sp_block_t out, sp_events_t events) {
  sp_count_t e_out_start;
  sp_event_t e;
  sp_count_t e_start;
  sp_count_t e_end;
  sp_count_t i;
  for (i = 0; (i < events.size); i = (1 + i)) {
    e = (events.data)[i];
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
};
void sp_events_free(sp_events_t events) {
  sp_count_t i;
  for (i = 0; (i < events.size); i = (1 + i)) {
    (events.data + i)->free;
  };
};
typedef struct {
  sp_count_t start;
  sp_count_t end;
  sp_count_t out_start;
  sp_block_t out;
  sp_event_t* event;
  future_t future;
} sp_seq_future_t;
void* sp_seq_parallel_future_f(void* data) {
  sp_seq_future_t* a = data;
  (a->event->f)((a->start), (a->end), (a->out), (a->event));
};
/** like sp_seq but evaluates events in parallel */
status_t sp_seq_parallel(sp_count_t start, sp_count_t end, sp_block_t out, sp_events_t events) {
  status_declare;
  sp_count_t e_out_start;
  sp_event_t e;
  sp_count_t e_start;
  sp_count_t e_end;
  sp_channel_count_t channel_i;
  sp_count_t events_start;
  sp_count_t events_count;
  sp_seq_future_t* seq_futures;
  sp_seq_future_t* sf;
  sp_count_t i;
  sp_count_t e_i;
  seq_futures = 0;
  /* select active events */
  for (i = 0, events_start = 0, events_count = 0; (i < events.size); i = (1 + i)) {
    e = (events.data)[i];
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
    e = (events.data)[(events_start + i)];
    sf = (i + seq_futures);
    e_out_start = ((e.start > start) ? (e.start - start) : 0);
    e_start = ((start > e.start) ? (start - e.start) : 0);
    e_end = (((e.end < end) ? e.end : end) - e.start);
    status_require((sp_block_new((out.channels), (e_end - e_start), (&(sf->out)))));
    sf->start = e_start;
    sf->end = e_end;
    sf->out_start = e_out_start;
    sf->event = (events_start + i + events.data);
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
  return (status);
};
void sp_synth_event_free(sp_event_t* a) {
  free((((sp_synth_event_state_t*)(a->state))->state));
  free((a->state));
};
void sp_synth_event_f(sp_count_t start, sp_count_t end, sp_block_t out, sp_event_t* event) {
  sp_synth_event_state_t* s = event->state;
  sp_synth(out, start, (end - start), (s->config_len), (s->config), (s->state));
};
/** memory for event.state will be allocated and then owned by the caller.
  config is copied into event.state */
status_t sp_synth_event(sp_count_t start, sp_count_t end, sp_count_t channel_count, sp_count_t config_len, sp_synth_partial_t* config, sp_event_t* out_event) {
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
  return (status);
};
typedef struct {
  sp_sample_t** amp;
  sp_sample_t* cut_h;
  sp_sample_t* cut_l;
  sp_convolution_filter_state_t* filter_state;
  uint8_t is_reject;
  sp_sample_t* noise;
  sp_random_state_t random_state;
  sp_count_t resolution;
  sp_sample_t* temp;
  sp_sample_t* trn_h;
  sp_sample_t* trn_l;
} sp_noise_event_state_t;
void sp_noise_event_f(sp_count_t start, sp_count_t end, sp_block_t out, sp_event_t* event) {
  sp_noise_event_state_t* s;
  sp_count_t block_count;
  sp_count_t i;
  sp_count_t block_i;
  sp_count_t channel_i;
  sp_count_t t;
  sp_count_t block_offset;
  sp_count_t duration;
  sp_count_t block_rest;
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
    s->random_state = sp_random((s->random_state), duration, (s->noise));
    sp_windowed_sinc_bp_br((s->noise), duration, ((s->cut_l)[t]), ((s->cut_h)[t]), ((s->trn_l)[t]), ((s->trn_h)[t]), (s->is_reject), (&(s->filter_state)), (s->temp));
    for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
      for (i = 0; (i < duration); i = (1 + i)) {
        ((out.samples)[channel_i])[(block_offset + i)] = (((out.samples)[channel_i])[(block_offset + i)] + (((s->amp)[channel_i])[(block_offset + i)] * (s->temp)[i]));
      };
    };
  };
};
void sp_noise_event_free(sp_event_t* a) {
  sp_noise_event_state_t* s = a->state;
  free((s->noise));
  free((s->temp));
  sp_convolution_filter_state_free((s->filter_state));
  free((a->state));
};
/** an event for noise filtered by a windowed-sinc filter.
  very processing intensive when parameters change with low resolution.
  memory for event.state will be allocated and then owned by the caller */
status_t sp_noise_event(sp_count_t start, sp_count_t end, sp_sample_t** amp, sp_sample_t* cut_l, sp_sample_t* cut_h, sp_sample_t* trn_l, sp_sample_t* trn_h, uint8_t is_reject, sp_count_t resolution, sp_random_state_t random_state, sp_event_t* out_event) {
  status_declare;
  sp_sample_t* temp;
  sp_sample_t* temp_noise;
  sp_count_t ir_len;
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
  status_require((sph_helper_malloc((ir_len * sizeof(sp_sample_t)), (&temp))));
  status_require((sph_helper_malloc((ir_len * sizeof(sp_sample_t)), (&temp_noise))));
  random_state = sp_random(random_state, ir_len, temp_noise);
  s->filter_state = 0;
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
  return (status);
};
typedef struct {
  sp_sample_t** amp;
  sp_sample_t* cut;
  sp_sample_t q_factor;
  sp_count_t passes;
  sp_cheap_filter_state_t filter_state;
  sp_state_variable_filter_t type;
  sp_sample_t* noise;
  sp_random_state_t random_state;
  sp_count_t resolution;
  sp_sample_t* temp;
} sp_cheap_noise_event_state_t;
void sp_cheap_noise_event_f(sp_count_t start, sp_count_t end, sp_block_t out, sp_event_t* event) {
  sp_count_t block_count;
  sp_count_t block_i;
  sp_count_t block_offset;
  sp_count_t block_rest;
  sp_count_t channel_i;
  sp_count_t duration;
  sp_count_t i;
  sp_cheap_noise_event_state_t* s;
  sp_count_t t;
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
    s->random_state = sp_random((s->random_state), duration, (s->noise));
    sp_cheap_filter((s->type), (s->noise), duration, ((s->cut)[t]), (s->passes), (s->q_factor), 1, (&(s->filter_state)), (s->temp));
    for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
      for (i = 0; (i < duration); i = (1 + i)) {
        ((out.samples)[channel_i])[(block_offset + i)] = (((out.samples)[channel_i])[(block_offset + i)] + (((s->amp)[channel_i])[(block_offset + i)] * (s->temp)[i]));
      };
    };
  };
};
void sp_cheap_noise_event_free(sp_event_t* a) {
  sp_cheap_noise_event_state_t* s = a->state;
  free((s->noise));
  free((s->temp));
  sp_cheap_filter_state_free((&(s->filter_state)));
  free((a->state));
};
/** an event for noise filtered by a state-variable filter. multiple passes currently not implemented.
  lower processing costs even when parameters change with high resolution.
  multiple passes almost multiply performance costs.
  memory for event.state will be allocated and then owned by the caller */
status_t sp_cheap_noise_event(sp_count_t start, sp_count_t end, sp_sample_t** amp, sp_state_variable_filter_t type, sp_sample_t* cut, sp_count_t passes, sp_sample_t q_factor, sp_count_t resolution, sp_random_state_t random_state, sp_event_t* out_event) {
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
  return (status);
};