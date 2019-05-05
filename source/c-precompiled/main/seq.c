void sp_event_sort_swap(void* a, void* b) {
  sp_event_t c;
  c = *((sp_event_t*)(a));
  *((sp_event_t*)(b)) = *((sp_event_t*)(a));
  *((sp_event_t*)(b)) = c;
};
uint8_t sp_event_sort_less_p(void* a, void* b) { return ((((sp_event_t*)(a))->start < ((sp_event_t*)(b))->start)); };
void sp_seq_events_prepare(sp_event_t* a, sp_count_t size) { quicksort(sp_event_sort_less_p, sp_event_sort_swap, (sizeof(sp_event_t)), a, size); };
/** event arrays must have been prepared/sorted with sp-seq-event-prepare for seq to work correctly */
void sp_seq(sp_count_t start, sp_count_t end, sp_block_t out, sp_count_t out_start, sp_event_t* events, sp_count_t events_size) {
  sp_count_t e_out_start;
  sp_event_t e;
  sp_count_t e_start;
  sp_count_t e_end;
  sp_count_t i;
  if (out_start) {
    out = sp_block_with_offset(out, out_start);
  };
  for (i = 0; (i < events_size); i = (1 + i)) {
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
status_t sp_seq_parallel(sp_count_t start, sp_count_t end, sp_block_t out, sp_count_t out_start, sp_event_t* events, sp_count_t events_size) {
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
  if (out_start) {
    out = sp_block_with_offset(out, out_start);
  };
  /* select active events */
  for (i = 0, events_start = 0, events_count = 0; (i < events_size); i = (1 + i)) {
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
  return (status);
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
  status_require((sph_helper_malloc((channel_count * sizeof(sp_synth_event_state_t)), (&state))));
  status_require((sp_synth_state_new(channel_count, config_len, config, (&(state->state)))));
  memcpy((state->config), config, (config_len * sizeof(sp_synth_partial_t)));
  state->config_len = config_len;
  e.start = start;
  e.end = end;
  e.f = sp_synth_event_f;
  e.state = state;
  *out_event = e;
exit:
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
  sp_noise_event_state_t* s = event->state;
  s->random_state = sp_random((s->random_state), (end - start), (s->noise));
  sp_windowed_sinc_bp_br((s->noise), (s->resolution), (*(s->cut_l)), (*(s->cut_h)), (*(s->trn_l)), (*(s->trn_h)), (s->is_reject), (&(s->filter_state)), (s->temp));
};
void sp_noise_event_free(sp_event_t* a) {
  sp_noise_event_state_t* s = a->state;
  free((s->noise));
  free((s->temp));
  sp_convolution_filter_state_free((s->filter_state));
  free((a->state));
};
/** memory for event.state will be allocated and then owned by the caller */
status_t sp_noise_event(sp_count_t start, sp_count_t end, sp_sample_t** amp, sp_sample_t* cut_l, sp_sample_t* cut_h, sp_sample_t* trn_l, sp_sample_t* trn_h, uint8_t is_reject, sp_count_t resolution, sp_random_state_t random_state, sp_event_t* out_event) {
  status_declare;
  sp_event_t e;
  sp_noise_event_state_t* s;
  status_require((sph_helper_malloc((sizeof(sp_noise_event_state_t)), (&s))));
  status_require((sph_helper_malloc((resolution * sizeof(sp_sample_t)), (&(s->noise)))));
  status_require((sph_helper_malloc((resolution * sizeof(sp_sample_t)), (&(s->temp)))));
  s->cut_l = cut_l;
  s->cut_h = cut_h;
  s->trn_l = trn_l;
  s->trn_h = trn_h;
  s->is_reject = is_reject;
  s->resolution = resolution;
  s->filter_state = 0;
  s->random_state = random_state;
  s->amp = amp;
  e.start = start;
  e.end = end;
  e.f = sp_noise_event_f;
  e.state = s;
  *out_event = e;
exit:
  return (status);
};