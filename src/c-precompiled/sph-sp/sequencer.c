
/* event.free functions must set event.free to null.
   event.prepare and event.generate must be user replaceable.
   event.prepare is to be set to null by callers of event.prepare */
void sp_event_list_display_element(sp_event_list_t* list) { printf(("%p <- %p -> %p event %zu %zu\n"), ((void*)(list->previous)), ((void*)(list)), ((void*)(list->next)), ((size_t)(list->value.start)), ((size_t)(list->value.end))); }
void sp_event_list_remove(sp_event_list_t** head_pointer, sp_event_list_t* list) {
  if (!list) {
    return;
  };
  sp_event_list_unlink(head_pointer, list);
  if (list->value.free) {
    (list->value.free)((&(list->value)));
  };
  free(list);
}
status_t sp_event_list_add(sp_event_list_t** head_pointer, sp_event_t event) {
  status_declare;
  sp_event_list_t* current;
  sp_event_list_t* new;
  status_require((sp_malloc_type(1, sp_event_list_t, (&new))));
  new->value = event;
  current = *head_pointer;
  if (current) {
    if (current->value.start <= event.start) {
      new->previous = 0;
      new->next = current;
      current->previous = new;
      *head_pointer = new;
    } else {
      while ((current->next && (current->next->value.start > event.start))) {
        current = current->next;
      };
      new->next = current->next;
      new->previous = current;
      if (current->next) {
        current->next->previous = new;
      };
      current->next = new;
    };
  } else {
    new->previous = 0;
    new->next = 0;
    *head_pointer = new;
  };
exit:
  status_return;
}
void sp_event_list_free(sp_event_list_t** head_pointer) {
  sp_event_list_t* current;
  sp_event_list_t* next;
  current = *head_pointer;
  while (current) {
    if (current->value.free) {
      (current->value.free)((&(current->value)));
    };
    next = current->next;
    free(current);
    current = next;
  };
  *head_pointer = 0;
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
    ep = &(current->value);
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
sp_resonator_event_config_t sp_resonator_event_config_defaults(void) {
  sp_resonator_event_config_t out;
  sp_size_t channel_index;
  sp_resonator_event_channel_config_t* channel_config;
  out.random_state = sp_random_state_new((sp_time_random_primitive((&sp_random_state))));
  out.resolution = sp_default_resolution;
  out.noise_in = 0;
  out.noise_out = 0;
  out.bandwidth_threshold = 0.0;
  out.channel_count = sp_channel_count_limit;
  channel_index = 0;
  while ((channel_index < sp_channel_count_limit)) {
    channel_config = (out.channel_config + channel_index);
    channel_config->amod = 0;
    channel_config->amp = 1.0;
    channel_config->channel = channel_index;
    channel_config->filter_state = 0;
    channel_config->frq = 0.0;
    channel_config->fmod = 0;
    channel_config->wdt = 0.0;
    channel_config->wmod = 0;
    channel_config->phs = 0;
    channel_config->pmod = 0;
    channel_config->wvf = sp_sine_table;
    channel_config->wvf_size = sp_rate;
    channel_config->use = (channel_index == 0);
    channel_index = (channel_index + 1);
  };
  return (out);
}
status_t sp_resonator_event_config_new_n(sp_time_t count, sp_resonator_event_config_t** out) {
  status_declare;
  sp_resonator_event_config_t defaults_value;
  sp_time_t index;
  defaults_value = sp_resonator_event_config_defaults();
  status_require((sp_malloc_type(count, sp_resonator_event_config_t, out)));
  index = 0;
  while ((index < count)) {
    (*out)[index] = defaults_value;
    index = (index + 1);
  };
exit:
  status_return;
}
void sp_resonator_event_free(sp_event_t* event) {
  sp_resonator_event_config_t* config;
  sp_resonator_event_channel_config_t* channel_config;
  sp_channel_count_t channel_index;
  config = event->config;
  if (!config) {
    return;
  };
  channel_index = 0;
  while ((channel_index < config->channel_count)) {
    channel_config = (config->channel_config + channel_index);
    if (channel_config->filter_state) {
      sp_convolution_filter_state_free((channel_config->filter_state));
      channel_config->filter_state = 0;
    };
    channel_index = (channel_index + 1);
  };
  if (config->noise_in) {
    free((config->noise_in));
    config->noise_in = 0;
  };
  if (config->noise_out) {
    free((config->noise_out));
    config->noise_out = 0;
  };
}
status_t sp_resonator_event_prepare(sp_event_t* event) {
  status_declare;
  sp_resonator_event_config_t* config;
  sp_resonator_event_channel_config_t* base_channel;
  sp_resonator_event_channel_config_t* channel_config;
  sp_channel_count_t channel_index;
  sp_time_t duration;
  sp_time_t temp_length;
  sp_channel_count_t keep_channel;
  config = event->config;
  base_channel = config->channel_config;
  if (!base_channel->amod || !base_channel->wvf || (base_channel->wvf_size <= 0)) {
    sp_status_set_goto(sp_s_id_invalid_argument);
  };
  duration = (event->end - event->start);
  if (config->resolution <= 0) {
    config->resolution = duration;
  };
  temp_length = config->resolution;
  if (temp_length > ((sp_time_t)((sp_render_block_seconds * sp_rate)))) {
    temp_length = ((sp_time_t)((sp_render_block_seconds * sp_rate)));
  };
  if (temp_length <= 0) {
    temp_length = 1;
  };
  config->random_state = sp_random_state_new((sp_time_random_primitive((&sp_random_state))));
  status_require((sp_malloc_type(temp_length, sp_sample_t, (&(config->noise_in)))));
  status_require((sp_malloc_type(temp_length, sp_sample_t, (&(config->noise_out)))));
  channel_index = 0;
  while ((channel_index < config->channel_count)) {
    channel_config = (config->channel_config + channel_index);
    if (!channel_config->use) {
      keep_channel = channel_config->channel;
      *channel_config = *base_channel;
      channel_config->channel = keep_channel;
    } else {
      if (!channel_config->amod) {
        channel_config->amod = base_channel->amod;
      };
      if (!channel_config->wvf) {
        channel_config->wvf = base_channel->wvf;
        channel_config->wvf_size = base_channel->wvf_size;
      };
      if (channel_config->wvf_size <= 0) {
        channel_config->wvf = base_channel->wvf;
        channel_config->wvf_size = base_channel->wvf_size;
      };
      if (!channel_config->fmod) {
        channel_config->fmod = base_channel->fmod;
      };
      if (!channel_config->wmod) {
        channel_config->wmod = base_channel->wmod;
      };
      if (!channel_config->pmod) {
        channel_config->pmod = base_channel->pmod;
      };
    };
    channel_config->filter_state = 0;
    channel_index = (channel_index + 1);
  };
exit:
  status_return;
}
status_t sp_resonator_event_generate_block(sp_time_t duration, sp_time_t block_i, sp_time_t event_i, void* out, sp_event_t* event) {
  status_declare;
  sp_resonator_event_config_t* config;
  sp_resonator_event_channel_config_t* channel_config;
  sp_resonator_event_channel_config_t channel_value;
  sp_channel_count_t channel_index;
  sp_sample_t* noise_in;
  sp_sample_t* noise_out;
  sp_frq_t frq_value;
  sp_frq_t bandwidth_value;
  sp_sample_t cutoff_low_factor;
  sp_sample_t cutoff_high_factor;
  sp_sample_t transition_factor;
  uint8_t arguments_buffer[(3 * sizeof(sp_sample_t))];
  uint8_t arguments_length;
  sp_time_t sample_index;
  sp_time_t phase_value;
  sp_sample_t amplitude_value;
  sp_sample_t sine_value;
  sp_sample_t noise_value;
  sp_sample_t out_value;
  config = event->config;
  noise_in = config->noise_in;
  noise_out = config->noise_out;
  sp_samples_random_primitive((&(config->random_state)), duration, noise_in);
  channel_index = 0;
  while ((channel_index < config->channel_count)) {
    channel_config = (config->channel_config + channel_index);
    channel_value = *channel_config;
    if (!channel_value.use) {
      channel_index += 1;
      continue;
    };
    frq_value = sp_optional_array_get((channel_value.fmod), (channel_value.frq), event_i);
    bandwidth_value = sp_optional_array_get((channel_value.wmod), (channel_value.wdt), event_i);
    if (bandwidth_value < config->bandwidth_threshold) {
      sample_index = 0;
      while ((sample_index < duration)) {
        phase_value = channel_value.phs;
        if (channel_value.pmod) {
          phase_value += (channel_value.pmod)[(event_i + sample_index)];
          if (phase_value >= channel_value.wvf_size) {
            phase_value -= (channel_value.wvf_size * floor((phase_value / channel_value.wvf_size)));
          };
        };
        amplitude_value = (channel_value.amp * (channel_value.amod)[(event_i + sample_index)]);
        sine_value = (channel_value.wvf)[((sp_time_t)(phase_value))];
        out_value = (amplitude_value * sine_value);
        (((sp_block_t*)(out))->samples)[channel_value.channel][(block_i + sample_index)] += sp_inline_limit(out_value, -1, 1);
        channel_value.phs = (channel_value.phs + frq_value);
        if (channel_value.phs >= channel_value.wvf_size) {
          channel_value.phs -= (channel_value.wvf_size * floor((channel_value.phs / channel_value.wvf_size)));
        };
        sample_index += 1;
      };
      channel_config->phs = channel_value.phs;
    } else {
      cutoff_low_factor = sp_hz_to_factor((frq_value - (0.5 * bandwidth_value)));
      cutoff_high_factor = sp_hz_to_factor((frq_value + (0.5 * bandwidth_value)));
      if (cutoff_low_factor < 0.0) {
        cutoff_low_factor = 0.0;
      };
      if (cutoff_high_factor > 0.5) {
        cutoff_high_factor = 0.5;
      };
      transition_factor = (0.5 * sp_hz_to_factor(bandwidth_value));
      arguments_length = (3 * sizeof(sp_sample_t));
      *((sp_sample_t*)(arguments_buffer)) = cutoff_low_factor;
      *(((sp_sample_t*)(arguments_buffer)) + 1) = cutoff_high_factor;
      *(((sp_sample_t*)(arguments_buffer)) + 2) = transition_factor;
      status_require((sp_convolution_filter(noise_in, duration, sp_resonator_ir_f, arguments_buffer, arguments_length, (&(channel_config->filter_state)), noise_out)));
      sample_index = 0;
      while ((sample_index < duration)) {
        amplitude_value = (channel_value.amp * (channel_value.amod)[(event_i + sample_index)]);
        noise_value = noise_out[sample_index];
        out_value = (amplitude_value * noise_value);
        (((sp_block_t*)(out))->samples)[channel_value.channel][(block_i + sample_index)] += sp_inline_limit(out_value, -1, 1);
        sample_index += 1;
      };
    };
    channel_index += 1;
  };
exit:
  status_return;
}
status_t sp_resonator_event_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event) { return ((sp_event_block_generate((((sp_resonator_event_config_t*)(event->config))->resolution), sp_resonator_event_generate_block, start, end, out, event))); }
