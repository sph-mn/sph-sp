void sp_event_sort_swap(void* a, ssize_t b, ssize_t c) {
  sp_event_t d;
  d = ((sp_event_t*)(a))[b];
  ((sp_event_t*)(a))[b] = ((sp_event_t*)(a))[c];
  ((sp_event_t*)(a))[c] = d;
}
uint8_t sp_event_sort_less_p(void* a, ssize_t b, ssize_t c) { return (((((sp_event_t*)(a))[b]).start < (((sp_event_t*)(a))[c]).start)); }
void sp_seq_events_prepare(sp_event_t* data, sp_time_t size) { quicksort(sp_event_sort_less_p, sp_event_sort_swap, data, 0, (size - 1)); }
/** event arrays must have been prepared/sorted with sp-seq-event-prepare for seq to work correctly.
   event functions receive event relative start/end time and block has index 0 at start.
   as for paths, start is inclusive, end is exclusive, so that 0..100 and 100..200 attach seamless.
   size is events size */
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
  sp_channels_t chn_i;
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
void sp_wave_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) { sp_wave(start, (end - start), (event->state), out); }
void sp_wave_event_free(sp_event_t* event) { free((event->state)); }
/** memory for event.state is allocated and later freed with event.free */
status_t sp_wave_event(sp_time_t start, sp_time_t end, sp_wave_state_t state, sp_event_t* out) {
  status_declare;
  sp_wave_state_t* event_state;
  event_state = 0;
  status_require((sph_helper_malloc((sizeof(sp_wave_state_t)), (&event_state))));
  *event_state = state;
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
  sp_time_t chn_i;
  sp_time_t t;
  sp_time_t block_offset;
  sp_time_t duration;
  sp_time_t block_rest;
  /* update filter arguments only every resolution number of samples */
  s = event->state;
  duration = (end - start);
  block_count = ((duration == s->resolution) ? 1 : sp_cheap_floor_positive((duration / s->resolution)));
  block_rest = (duration % s->resolution);
  for (block_i = 0; (block_i <= block_count); block_i = (1 + block_i)) {
    block_offset = (s->resolution * block_i);
    t = (start + block_offset);
    duration = ((block_count == block_i) ? block_rest : s->resolution);
    sp_samples_random((&(s->random_state)), duration, (s->noise));
    sp_windowed_sinc_bp_br((s->noise), duration, ((s->cut_l)[t]), ((s->cut_h)[t]), ((s->trn_l)[t]), ((s->trn_h)[t]), (s->is_reject), (&(s->filter_state)), (s->temp));
    for (chn_i = 0; (chn_i < out.channels); chn_i = (1 + chn_i)) {
      for (i = 0; (i < duration); i = (1 + i)) {
        (out.samples)[chn_i][(block_offset + i)] += ((s->amp)[chn_i][(t + i)] * (s->temp)[i]);
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
   very processing intensive if parameters change with low resolution.
   memory for event.state will be allocated and then owned by the caller */
status_t sp_noise_event(sp_time_t start, sp_time_t end, sp_sample_t** amp, sp_sample_t* cut_l, sp_sample_t* cut_h, sp_sample_t* trn_l, sp_sample_t* trn_h, uint8_t is_reject, sp_time_t resolution, sp_random_state_t random_state, sp_event_t* out_event) {
  status_declare;
  sp_sample_t* temp;
  sp_sample_t* temp_noise;
  sp_time_t ir_len;
  sp_event_t e;
  sp_noise_event_state_t* s;
  resolution = (resolution ? sp_min(resolution, (end - start)) : 96);
  status_require((sph_helper_malloc((sizeof(sp_noise_event_state_t)), (&s))));
  status_require((sph_helper_malloc((resolution * sizeof(sp_sample_t)), (&(s->noise)))));
  status_require((sph_helper_malloc((resolution * sizeof(sp_sample_t)), (&(s->temp)))));
  /* the result shows a small delay, circa 40 samples for transition 0.07. the size seems to be related to ir-len.
the state is initialised with one unused call to skip the delay.
an added benefit is that the filter-state setup malloc is checked */
  ir_len = sp_windowed_sinc_lp_hp_ir_length((sp_min((*trn_l), (*trn_h))));
  s->filter_state = 0;
  status_require((sph_helper_malloc((ir_len * sizeof(sp_sample_t)), (&temp))));
  status_require((sph_helper_malloc((ir_len * sizeof(sp_sample_t)), (&temp_noise))));
  sp_samples_random((&random_state), ir_len, temp_noise);
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
  sp_time_t chn_i;
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
    sp_samples_random((&(s->random_state)), duration, (s->noise));
    sp_cheap_filter((s->type), (s->noise), duration, ((s->cut)[t]), (s->passes), (s->q_factor), (&(s->filter_state)), (s->temp));
    for (chn_i = 0; (chn_i < out.channels); chn_i = (1 + chn_i)) {
      for (i = 0; (i < duration); i = (1 + i)) {
        (out.samples)[chn_i][(block_offset + i)] += ((s->amp)[chn_i][(t + i)] * (s->temp)[i]);
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
/** an event for noise filtered by a state-variable filter.
   lower processing costs even when parameters change with high resolution.
   multiple passes almost multiply performance costs.
   memory for event.state will be allocated and then owned by the caller */
status_t sp_cheap_noise_event(sp_time_t start, sp_time_t end, sp_sample_t** amp, sp_state_variable_filter_t type, sp_sample_t* cut, sp_time_t passes, sp_sample_t q_factor, sp_time_t resolution, sp_random_state_t random_state, sp_event_t* out_event) {
  status_declare;
  sp_event_t e;
  sp_cheap_noise_event_state_t* s;
  resolution = (resolution ? sp_min(resolution, (end - start)) : 96);
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
#define sp_group_event_free_events(events, end) \
  while ((array4_in_range(events) && ((array4_get(events)).end <= end))) { \
    if ((array4_get(events)).free) { \
      ((array4_get(events)).free)((array4_get_address(events))); \
    }; \
    array4_forward(events); \
  }
/** events.current is used to track freed events */
void sp_group_event_free(sp_event_t* a) {
  sp_group_event_state_t s = *((sp_group_event_state_t*)(a->state));
  while (array4_in_range((s.events))) {
    if ((array4_get((s.events))).free) {
      ((array4_get((s.events))).free)((array4_get_address((s.events))));
    };
    array4_forward((s.events));
  };
  memreg_heap_free_pointers((s.memory));
  array4_free((s.events));
  array4_free((s.memory));
  free((a->state));
}
/** free past events early, they might be sub-group trees */
void sp_group_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_group_event_state_t* s = event->state;
  sp_seq(start, end, out, (array4_get_address((s->events))), (array4_right_size((s->events))));
  sp_group_event_free_events((s->events), end);
}
/** can be used in place of sp-group-event-f.
   seq-parallel can fail if there is not enough memory, but this is ignored currently */
void sp_group_event_parallel_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  sp_group_event_state_t* s = event->state;
  sp_seq_parallel(start, end, out, (array4_get_address((s->events))), (array4_right_size((s->events))));
  sp_group_event_free_events((s->events), end);
}
status_t sp_group_new(sp_time_t start, sp_group_size_t event_size, sp_group_size_t memory_size, sp_event_t* out) {
  status_declare;
  sp_group_event_state_t* s;
  memreg_init(2);
  status_require((sph_helper_malloc((sizeof(sp_group_event_state_t)), (&s))));
  memreg_add(s);
  if (sp_events_new(event_size, (&(s->events)))) {
    sp_memory_error;
  };
  memreg_add((s->events.data));
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
  event.start += a->end;
  event.end += a->end;
  sp_group_add((*a), event);
}
/** creates an event group of sine events based on the given envelope configuration.
   partial envelopes are created by modifying amp and frq with data from changes arrays.
   the result amplitude envelopes are adjusted to in sum follow the given amp path while keeping the specified relations.
   sp_sine_cluster can take arrays that give the volume distribution between partials and the distribution of frequencies.
   arguments:
     amp: the result amplitude envelope and base for partials
     frq: frequency envelope and base for partials
     ax/ay/fx/fy:
       each is an array with the layout ((number:partial_change ...):point_change ...).
       points from sp_path_constant and sp_path_path are not to be included.
       to create envelopes for the different partials, for each partial and path segment point of amp/frq,
       the x or y value of the paths point is multiplied with a value from the corresponding changes array.
     phs: phase offsets in number of samples */
status_t sp_sine_cluster(sp_time_t prt_count, sp_path_t amp, sp_path_t frq, sp_time_t* phs, sp_sample_t** ax, sp_sample_t** ay, sp_sample_t** fx, sp_sample_t** fy, sp_event_t* out) {
  sp_time_t* a_samples_sizes;
  sp_sample_t** a_samples;
  sp_time_t duration;
  sp_event_t event;
  sp_time_t f_times_size;
  sp_time_t* f_times;
  sp_event_t group;
  sp_time_t i;
  status_declare;
  memreg_init(1);
  sp_event_set_null(group);
  a_samples = 0;
  status_require((sp_group_new(0, prt_count, (2 * prt_count), (&group))));
  status_require((sp_path_samples_derivations_normalized(amp, prt_count, ax, ay, (&a_samples), (&a_samples_sizes))));
  for (i = 0; (i < prt_count); i += 1) {
    sp_group_memory_add(group, (a_samples[i]));
    status_require((sp_path_times_derivation(frq, fx, fy, i, (&f_times), (&f_times_size))));
    sp_group_memory_add(group, f_times);
    duration = sp_min((a_samples_sizes[i]), f_times_size);
    status_require((sp_wave_event(0, duration, (sp_sine_state_2(duration, f_times, (a_samples[i]), (a_samples[i]), (phs[i]), (phs[i]))), (&event))));
    sp_group_add(group, event);
  };
  sp_group_prepare(group);
  *out = group;
exit:
  memreg_free;
  if (status_is_failure) {
    sp_group_free(group);
  };
  status_return;
}
