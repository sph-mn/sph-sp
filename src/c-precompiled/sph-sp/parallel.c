void* sp_seq_parallel_future_entry(void* data) {
  sp_seq_future_t* future = data;
  future->status = (future->run)(future);
  return (0);
}
status_t sp_seq_parallel_generic(sp_time_t start, sp_time_t end, void* out, sp_event_list_t** events, sp_seq_parallel_generic_config_t* config) {
  status_declare;
  sp_event_list_t* current;
  sp_event_list_t* next;
  sp_event_t* ep;
  sp_event_t e;
  sp_seq_future_t* futures;
  sp_seq_future_t* future;
  sp_size_t count;
  sp_size_t index;
  sp_time_t relative_start;
  sp_time_t relative_end;
  sp_time_t output_start;
  current = *events;
  count = 0;
  futures = 0;
  while (current) {
    if (end <= current->value.start) {
      current = 0;
    } else {
      if (current->value.end > start) {
        count += 1;
      };
      current = current->next;
    };
  };
  status_require((sp_calloc_type(count, sp_seq_future_t, (&futures))));
  current = *events;
  index = 0;
  while (current) {
    ep = &(current->value);
    e = *ep;
    if (e.end <= start) {
      next = current->next;
      sp_event_list_remove(events, current);
      current = next;
    } else {
      if (end <= e.start) {
        current = 0;
      } else {
        relative_start = ((start > e.start) ? (start - e.start) : 0);
        relative_end = (sp_inline_min(end, (e.end)) - e.start);
        output_start = ((e.start > start) ? (e.start - start) : 0);
        future = (futures + index);
        future->start = relative_start;
        future->end = relative_end;
        future->out_start = output_start;
        future->event = ep;
        future->status.id = status_id_success;
        future->run = config->run;
        if (config->make) {
          status_require(((config->make)((relative_end - relative_start), out, (&(future->out)))));
        } else {
          future->out = out;
        };
        sph_future_new(sp_seq_parallel_future_entry, future, (&(future->future)));
        index += 1;
        current = current->next;
      };
    };
  };
  for (sp_size_t k = 0; (k < index); k += 1) {
    future = (futures + k);
    sph_future_touch((&(future->future)));
    status_require((future->status));
  };
  if (config->merge) {
    status_require(((config->merge)(start, end, out, futures, index, (config->context))));
  };
exit:
  if (futures) {
    if (config->free) {
      for (sp_size_t k = 0; (k < index); k += 1) {
        future = (futures + k);
        if (config->make && (future->out != out)) {
          (config->free)((future->out));
        };
      };
    };
    free(futures);
  };
  if (status_is_failure) {
    sp_event_list_free(events);
  };
  status_return;
}
status_t sp_block_future_make(sp_time_t count, void* parent_out, void** product_out) {
  status_declare;
  sp_block_t* parent_block;
  sp_block_t* bp;
  parent_block = parent_out;
  bp = 0;
  status_require((sp_malloc_type(1, sp_block_t, (&bp))));
  status_require((sp_block_new((parent_block->channel_count), count, bp)));
  *product_out = bp;
exit:
  if (status_is_failure && bp) {
    free(bp);
  };
  status_return;
}
void sp_block_future_free(void* product) {
  if (!product) {
    return;
  };
  sp_block_t* bp;
  bp = product;
  sp_block_free(bp);
  free(bp);
}
status_t sp_block_future_run(sp_seq_future_t* future) {
  status_declare;
  sp_event_t* ep;
  sp_event_t e;
  sp_block_t* bp;
  ep = future->event;
  e = *ep;
  bp = future->out;
  sp_event_prepare_optional_srq(e);
  *ep = e;
  status_require(((e.generate)((future->start), (future->end), ((void*)(bp)), ep)));
exit:
  status_return;
}
status_t sp_block_merge_all(sp_time_t start, sp_time_t end, void* parent_out, sp_seq_future_t* futures, sp_size_t count, void* context) {
  status_declare;
  sp_block_t* out_block;
  out_block = parent_out;
  for (sp_size_t k = 0; (k < count); k += 1) {
    sp_seq_future_t* f;
    sp_block_t* b;
    f = (futures + k);
    b = f->out;
    for (sp_size_t ci = 0; (ci < out_block->channel_count); ci += 1) {
      for (sp_size_t i = 0; (i < b->size); i += 1) {
        ((out_block->samples)[ci])[(f->out_start + i)] = (((out_block->samples)[ci])[(f->out_start + i)] + ((b->samples)[ci])[i]);
      };
    };
  };
  status_return;
}
status_t sp_seq_parallel_block(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_list_t** events) {
  status_declare;
  sp_seq_parallel_generic_config_t c;
  c.make = sp_block_future_make;
  c.free = sp_block_future_free;
  c.merge = sp_block_merge_all;
  c.run = sp_block_future_run;
  c.context = 0;
  status_require((sp_seq_parallel_generic(start, end, (&out), events, (&c))));
exit:
  status_return;
}
status_t sp_group_generate_parallel_block(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* group) { return ((sp_seq_parallel_block(start, end, out, ((sp_event_list_t**)(&(group->config)))))); }
