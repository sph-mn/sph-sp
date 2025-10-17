
/* event.free functions must set event.free to null.
   event.prepare and event.generate must be user replaceable.
   event.prepare is to be set to null by callers of event.prepare */
#include <sph/memory.c>
void sp_event_list_display_element(sp_event_list_t* a) { printf("%lu %lu %lu event %lu %lu\n", (a->previous), a, (a->next), (a->event.start), (a->event.end)); }
void sp_event_list_display(sp_event_list_t* a) {
  while (a) {
    sp_event_list_display_element(a);
    a = a->next;
  };
}
void sp_event_list_reverse(sp_event_list_t** a) {
  sp_event_list_t* current;
  sp_event_list_t* next;
  next = *a;
  while (next) {
    current = next;
    next = next->next;
    current->next = current->previous;
    current->previous = next;
  };
  *a = current;
}
void sp_event_list_find_duplicate(sp_event_list_t* a, sp_event_list_t* b) {
  sp_time_t i = 0;
  sp_time_t count = 0;
  while (a) {
    if (a == b) {
      if (1 == count) {
        printf("duplicate list entry %p at index %lu\n", a, i);
        exit(1);
      } else {
        count += 1;
      };
    };
    i += 1;
    a = a->next;
  };
}
void sp_event_list_validate(sp_event_list_t* a) {
  sp_time_t i = 0;
  sp_event_list_t* b = a;
  sp_event_list_t* c = 0;
  while (b) {
    if (!(c == b->previous)) {
      printf("link to previous is invalid at index %lu, element %p\n", i, b);
      exit(1);
    };
    if ((b->next == b->previous) && !(0 == b->next)) {
      printf("circular list entry at index %lu, element %p\n", i, b);
      exit(1);
    };
    sp_event_list_find_duplicate(a, b);
    i += 1;
    c = b;
    b = b->next;
  };
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
        /* middle */
        new->previous = current->previous;
        new->next = current;
        current->previous->next = new;
        current->previous = new;
        goto exit;
      };
    };
    /* last */
    new->next = 0;
    new->previous = current;
    current->next = new;
  };
exit:
  status_return;
}

/** free all list elements and the associated events. update the list head so it becomes the empty list.
   needed if sp_seq will not further process and free currently incomplete events */
void sp_event_list_free(sp_event_list_t** events) {
  sp_event_list_t* temp;
  sp_event_list_t* current;
  current = *events;
  while (current) {
    sp_event_free((current->event));
    temp = current;
    current = current->next;
    free(temp);
  };
  *events = 0;
}

/** calls generate for sub-blocks of at most size resolution */
status_t sp_event_block_generate(sp_time_t resolution, sp_event_block_generate_t generate, sp_time_t start, sp_time_t end, void* out, sp_event_t* event) {
  status_declare;
  sp_time_t block_count;
  sp_time_t block_initial;
  sp_time_t block_rest;
  sp_time_t duration;
  sp_time_t i;
  duration = (end - start);
  if (duration < resolution) {
    resolution = duration;
  };
  block_count = sp_cheap_floor_positive((duration / resolution));
  block_initial = (block_count * resolution);
  block_rest = (duration % resolution);
  for (i = 0; (i < block_initial); i += resolution) {
    status_require((generate(resolution, i, (i + start), out, event)));
  };
  if (block_rest) {
    status_require((generate(block_rest, i, (i + start), out, event)));
  };
exit:
  status_return;
}

/** assumes that event start is set to the beginning, for example 0. copies event */
sp_event_t sp_event_schedule(sp_event_t event, sp_time_t onset, sp_time_t duration, void* config) {
  event.start += onset;
  event.end += (onset + duration);
  if (config) {
    event.config = config;
  };
  return (event);
}

/** event arrays must have been prepared/sorted once with sp_seq_event_prepare for seq to work correctly.
   like for paths, start is inclusive, end is exclusive, so that 0..100 and 100..200 attach seamless.
   events can have three function pointers: prepare, generate and free.
   generate receives event relative start/end times, and output blocks that start at the current block.
   prepare will only be called once and the function pointer will be set to zero afterwards.
   if prepare fails, sp_seq returns immediately.
   past events including the event list elements are freed when processing the following block.
   on error, all events and the event list are freed */
status_t sp_seq(sp_time_t start, sp_time_t end, void* out, sp_event_list_t** events) {
  status_declare;
  sp_event_t e;
  sp_event_t* ep;
  sp_event_list_t* next;
  sp_event_list_t* current;
  sp_time_t offset;
  sp_time_t relative_start;
  sp_time_t relative_end;
  sp_block_t shifted;
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
      sp_event_prepare_optional_srq(e);
      *ep = e;
      offset = ((e.start > start) ? (e.start - start) : 0);
      relative_start = (offset + (start - e.start));
      relative_end = (((end - e.start) < (e.end - e.start)) ? (end - e.start) : (e.end - e.start));
      shifted = sp_block_with_offset((*((sp_block_t*)(out))), offset);
      status_require(((e.generate)(relative_start, relative_end, (&shifted), ep)));
    };
    current = current->next;
  };
exit:
  if (status_is_failure) {
    sp_event_list_free(events);
  };
  status_return;
}
void sp_group_free(sp_event_t* group) {
  group->free = 0;
  sp_event_list_free((sp_group_event_list(group)));
  sp_event_memory_free(group);
}
status_t sp_group_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* group) { return ((sp_seq(start, end, out, ((sp_event_list_t**)(&(group->config)))))); }
status_t sp_group_prepare(sp_event_t* group) {
  status_declare;
  if (group->config) {
    sp_seq_events_prepare(((sp_event_list_t**)(&(group->config))));
  };
  group->free = sp_group_free;
  status_return;
}

/** events with end zero will be prepared immediately for cases when event.prepare sets end */
status_t sp_group_add(sp_event_t* group, sp_event_t event) {
  status_declare;
  if (!event.end) {
    sp_event_prepare_optional_srq(event);
  };
  status_require((sp_event_list_add(((sp_event_list_t**)(&(group->config))), event)));
  if (group->end < event.end) {
    group->end = event.end;
  };
exit:
  status_return;
}
status_t sp_group_append(sp_event_t* group, sp_event_t event) {
  status_declare;
  if (!event.end) {
    sp_event_prepare_optional_srq(event);
  };
  event.start += group->end;
  event.end += group->end;
  group->end = event.end;
  status_require((sp_event_list_add(((sp_event_list_t**)(&(group->config))), event)));
exit:
  status_return;
}
status_t sp_map_event_config_new_n(sp_time_t count, sp_map_event_config_t** out) { return ((sp_calloc_type(count, sp_map_event_config_t, out))); }

/** the wrapped event will be freed with the map-event.
   use cases: filter chains, post processing.
   config:
     map-generate: map function (start end sp_block_t:in sp_block_t:out void*:state)
     config: custom state value passed to f.
     isolate: use a dedicated output buffer for event
       events can be wrapped in multiple sp_map_event with an isolated sp_map_event on top that
       finally writes to main out to mix with other events */
status_t sp_map_event_prepare(sp_event_t* event) {
  status_declare;
  sp_map_event_config_t* c;
  c = event->config;
  sp_event_prepare_optional_srq((c->event));
  event->free = sp_map_event_free;
exit:
  status_return;
}
void sp_map_event_free(sp_event_t* event) {
  sp_map_event_config_t* c;
  event->free = 0;
  c = event->config;
  sp_event_free((c->event));
  sp_event_memory_free(event);
}
status_t sp_map_event_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event) {
  status_declare;
  sp_map_event_config_t* c = event->config;
  status_require(((c->event.generate)(start, end, out, (&(c->event)))));
  status_require(((c->map_generate)(start, end, out, out, (c->config))));
exit:
  status_return;
}

/** creates temporary output, lets event write to it, and passes the temporary output to a user function */
status_t sp_map_event_isolated_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event) {
  status_declare;
  sp_declare_block(temp_out);
  sp_map_event_config_t* c;
  c = event->config;
  status_require((sp_block_new((((sp_block_t*)(out))->channel_count), (((sp_block_t*)(out))->size), (&temp_out))));
  status_require(((c->event.generate)(start, end, (&temp_out), (&(c->event)))));
  status_require(((c->map_generate)(start, end, (&temp_out), out, (c->config))));
exit:
  sp_block_free((&temp_out));
  status_return;
}
sp_wave_event_config_t sp_wave_event_config_defaults() {
  sp_wave_event_config_t out = { 0 };
  out.channel_config->use = 1;
  out.channel_count = sp_channel_count_limit;
  out.wvf = sp_sine_table;
  out.wvf_size = sp_rate;
  for (sp_size_t i = 0; (i < sp_channel_count_limit); i += 1) {
    ((out.channel_config)[i]).amp = 1;
    ((out.channel_config)[i]).channel = i;
  };
  return (out);
}

/** allocate sp_wave_event_config_t and set to defaults */
status_t sp_wave_event_config_new_n(sp_time_t count, sp_wave_event_config_t** out) {
  status_declare;
  sp_wave_event_config_t defaults = sp_wave_event_config_defaults();
  status_require((sp_malloc_type(count, sp_wave_event_config_t, out)));
  for (sp_size_t i = 0; (i < count); i += 1) {
    (*out)[i] = defaults;
  };
exit:
  status_return;
}

/** prepare an event playing waveforms from an array.
   event end will be longer if channel config delay is used.
   expects sp_wave_event_config_t at event.config.
   config:
   * frq (frequency): fixed base frequency. added to fmod if fmod is set
   * fmod (frequency): array with hertz values that will add to frq. note that it currently does not support negative frequencies
   * wvf (waveform): array with waveform samples
   * wvf-size: count of waveform samples
   * phs (phase): initial phase offset
   * pmod (phase): phase offset per sample
   * amp (amplitude): multiplied with amod
   * amod (amplitude): array with sample values */
status_t sp_wave_event_prepare(sp_event_t* event) {
  status_declare;
  sp_wave_event_config_t* c;
  sp_wave_event_channel_config_t* cc;
  sp_channel_count_t ci;
  c = event->config;
  for (sp_channel_count_t i = 1; (i < c->channel_count); i += 1) {
    cc = (c->channel_config + i);
    if (cc->use) {
      sp_inline_default((cc->amod), (c->channel_config->amod));
      sp_inline_default((cc->pmod), (c->channel_config->pmod));
      sp_inline_default((cc->fmod), (c->channel_config->fmod));
      sp_inline_default((cc->frq), (c->channel_config->frq));
    } else {
      ci = cc->channel;
      *cc = *(c->channel_config);
      cc->channel = ci;
    };
  };
  status_return;
}
status_t sp_wave_event_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event) {
  status_declare;
  sp_wave_event_config_t* c;
  sp_wave_event_channel_config_t cc;
  sp_wave_event_channel_config_t* ccp;
  sp_time_t phsn;
  c = event->config;
  for (sp_channel_count_t cci = 0; (cci < c->channel_count); cci += 1) {
    ccp = (c->channel_config + cci);
    cc = *ccp;
    for (sp_time_t i = start; (i < end); i += 1) {
      if (cc.pmod) {
        phsn = (cc.phs + (cc.pmod)[i]);
        if (phsn >= c->wvf_size) {
          phsn = (phsn % c->wvf_size);
        };
      } else {
        phsn = cc.phs;
      };
      (((sp_block_t*)(out))->samples)[cc.channel][(i - start)] += (cc.amp * (cc.amod)[i] * (c->wvf)[phsn]);
      cc.phs += (cc.fmod ? (cc.frq + (cc.fmod)[i]) : cc.frq);
      if (cc.phs >= c->wvf_size) {
        cc.phs = (cc.phs % c->wvf_size);
      };
    };
    ccp->phs = cc.phs;
  };
  status_return;
}
sp_noise_event_config_t sp_noise_event_config_defaults() {
  sp_noise_event_config_t out = { 0 };
  out.channel_config->use = 1;
  out.channel_count = sp_channel_count_limit;
  out.resolution = sp_default_resolution;
  for (sp_size_t i = 0; (i < sp_channel_count_limit); i += 1) {
    ((out.channel_config)[i]).amp = 1;
    ((out.channel_config)[i]).channel = i;
    ((out.channel_config)[i]).wdt = sp_max_frq;
    ((out.channel_config)[i]).trnl = (sp_rate * 0.08);
    ((out.channel_config)[i]).trnh = (sp_rate * 0.08);
  };
  return (out);
}
status_t sp_noise_event_config_new_n(sp_time_t count, sp_noise_event_config_t** out) {
  status_declare;
  sp_noise_event_config_t defaults = sp_noise_event_config_defaults();
  status_require((sp_malloc_type(count, sp_noise_event_config_t, out)));
  for (sp_size_t i = 0; (i < count); i += 1) {
    (*out)[i] = defaults;
  };
exit:
  status_return;
}

/** the initial filter output shows a small delay of circa 40 samples for transition 0.07. the size seems to be related to ir-len.
   the filter state is initialized with one unused call to skip the delay. */
status_t sp_noise_event_filter_state(sp_noise_event_config_t* c, sp_noise_event_channel_config_t* cc, sp_time_t ir_length) {
  sp_frq_t frqn;
  sp_samples_random_primitive((&(c->random_state)), ir_length, (*(c->temp)));
  frqn = sp_optional_array_get((cc->fmod), (cc->frq), 0);
  return ((sp_windowed_sinc_bp_br((*(c->temp)), ir_length, (sp_hz_to_factor(frqn)), (sp_hz_to_factor((frqn + sp_optional_array_get((cc->wmod), (cc->wdt), 0)))), (sp_hz_to_factor((cc->trnl))), (sp_hz_to_factor((cc->trnh))), (c->is_reject), (&(cc->filter_state)), ((c->temp)[1]))));
}
void sp_noise_event_free(sp_event_t* event) {
  sp_noise_event_config_t* c;
  sp_noise_event_channel_config_t* cc;
  event->free = 0;
  c = event->config;
  for (sp_size_t i = 0; (i < c->channel_count); i += 1) {
    cc = (c->channel_config + i);
    if (cc->filter_state) {
      sp_convolution_filter_state_free((cc->filter_state));
    };
  };
  free(((c->temp)[0]));
  free(((c->temp)[1]));
  free(((c->temp)[2]));
  sp_event_memory_free(event);
}

/** an event for noise filtered by a windowed-sinc filter.
   very processing intensive if parameters change with low resolution.
   event:config.resolution is how often the filter parameters should be updated.
   filter parameters are updated at least for each generate call */
status_t sp_noise_event_prepare(sp_event_t* event) {
  status_declare;
  sp_noise_event_channel_config_t* cc;
  sp_channel_count_t ci;
  sp_noise_event_config_t* c;
  sp_time_t duration;
  sp_bool_t filter_mod;
  sp_bool_t filter_states;
  sp_time_t ir_lengths[sp_channel_count_limit] = { 0 };
  sp_time_t temp_length;
  error_memory_init((3 + sp_channel_count_limit));
  c = event->config;
  c->random_state = sp_random_state_new((sp_time_random_primitive((&sp_random_state))));
  cc = c->channel_config;
  duration = (event->end - event->start);
  filter_mod = (cc->fmod || cc->wmod);
  filter_states = 0;
  temp_length = sp_windowed_sinc_lp_hp_ir_length((sp_hz_to_factor((sp_inline_min((cc->trnl), (cc->trnh))))));
  ir_lengths[0] = temp_length;
  /* the first channel is always required */
  if (!c->channel_config->amod) {
    sp_status_set_goto(sp_s_id_invalid_argument);
  };
  /* set channel defaults and collect size information for temporary buffers */
  for (sp_channel_count_t cci = 1; (cci < c->channel_count); cci += 1) {
    cc = (c->channel_config + cci);
    if (cc->use) {
      if (!cc->amod) {
        cc->amod = c->channel_config->amod;
      };
      if (cc->frq || cc->wdt || cc->fmod || cc->wmod || cc->trnl || cc->trnh) {
        filter_states = 1;
        if (cc->fmod || cc->wmod) {
          filter_mod = 1;
        };
        if (!cc->fmod) {
          cc->fmod = c->channel_config->fmod;
        };
        if (!cc->trnh) {
          cc->trnh = c->channel_config->trnh;
        };
        if (!cc->trnl) {
          cc->trnl = c->channel_config->trnl;
        };
        if (!cc->wdt) {
          cc->wdt = c->channel_config->wdt;
        };
        if (!cc->wmod) {
          cc->wmod = c->channel_config->wmod;
        };
        ir_lengths[cci] = sp_windowed_sinc_lp_hp_ir_length((sp_hz_to_factor((sp_inline_min((cc->trnl), (cc->trnh))))));
        if (temp_length < ir_lengths[cci]) {
          temp_length = ir_lengths[cci];
        };
      };
    } else {
      ci = cc->channel;
      *cc = *(c->channel_config);
      cc->channel = ci;
    };
  };
  /* no updates necessary if parameters do not change */
  if (!filter_mod) {
    c->resolution = duration;
  };
  /* this limits the largest block that can be safely requested */
  temp_length = sp_inline_max(temp_length, (c->resolution));
  temp_length = sp_inline_min(temp_length, (sp_render_block_seconds * sp_rate));
  /* three buffers: one for the white noise, one for the first channel filter result that may be shared with other channels,
       and one for other channel filter results */
  status_require((sp_malloc_type(temp_length, sp_sample_t, (c->temp))));
  error_memory_add(((c->temp)[0]));
  status_require((sp_malloc_type(temp_length, sp_sample_t, (1 + c->temp))));
  error_memory_add(((c->temp)[1]));
  /* allocate and initialize the filter-states using the buffers allocated above */
  cc = c->channel_config;
  status_require((sp_noise_event_filter_state(c, cc, (ir_lengths[0]))));
  error_memory_add2((cc->filter_state), sp_convolution_filter_state_free);
  if (filter_states) {
    status_require((sp_malloc_type(temp_length, sp_sample_t, (2 + c->temp))));
    error_memory_add(((c->temp)[2]));
    for (sp_channel_count_t cci = 1; (cci < c->channel_count); cci += 1) {
      cc = (c->channel_config + cci);
      if (ir_lengths[cci]) {
        status_require((sp_noise_event_filter_state(c, cc, (ir_lengths[cci]))));
        error_memory_add2((cc->filter_state), sp_convolution_filter_state_free);
      };
    };
  };
exit:
  if (status_is_failure) {
    error_memory_free;
  };
  status_return;
}
status_t sp_noise_event_generate_block(sp_time_t duration, sp_time_t block_i, sp_time_t event_i, void* out, sp_event_t* event) {
  status_declare;
  sp_noise_event_config_t* c;
  sp_noise_event_channel_config_t cc;
  sp_noise_event_channel_config_t* ccp;
  sp_frq_t frqn;
  sp_sample_t outn;
  sp_sample_t* temp;
  c = event->config;
  sp_samples_random_primitive((&(c->random_state)), duration, (*(c->temp)));
  for (sp_channel_count_t cci = 0; (cci < c->channel_count); cci += 1) {
    ccp = (c->channel_config + cci);
    cc = *ccp;
    if (cc.use && cc.filter_state) {
      temp = (c->temp)[(1 + (0 < cci))];
      frqn = sp_optional_array_get((cc.fmod), (cc.frq), event_i);
      status_require((sp_windowed_sinc_bp_br((*(c->temp)), duration, (sp_hz_to_factor(frqn)), (sp_hz_to_factor((frqn + sp_optional_array_get((cc.wmod), (cc.wdt), event_i)))), (sp_hz_to_factor((cc.trnl))), (sp_hz_to_factor((cc.trnh))), (c->is_reject), (&(ccp->filter_state)), temp)));
    } else {
      temp = (c->temp)[1];
    };
    for (sp_size_t i = 0; (i < duration); i += 1) {
      outn = (cc.amp * (cc.amod)[(event_i + i)] * temp[i]);
      (((sp_block_t*)(out))->samples)[cc.channel][(block_i + i)] += sp_inline_limit(outn, -1, 1);
    };
  };
exit:
  status_return;
}
status_t sp_noise_event_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event) { return ((sp_event_block_generate((((sp_noise_event_config_t*)(event->config))->resolution), sp_noise_event_generate_block, start, end, out, event))); }
