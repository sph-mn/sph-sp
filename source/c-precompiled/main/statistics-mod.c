/* routines that modify arrays to change statistical values */
/** try to increase repetition of subseqences of size $width to $target or nearest possible.
   $a is modified directly.
   uncommon sequences are replaced by more common ones */
status_t sp_stat_times_repetition_increase(sp_time_t* a, sp_time_t size, sp_time_t width, sp_time_t target) {
  status_declare;
  sp_time_t* a_copy;
  sp_time_t count;
  sp_time_t counts_size;
  sp_times_counted_sequences_t* counts;
  sp_time_t difference;
  sp_time_t i;
  sp_time_t j;
  sp_sequence_hashtable_t known;
  sp_time_t loop_limit;
  sp_time_t max_unique;
  sp_time_t measured;
  sp_time_t min_repetition;
  sp_time_t* seq_a;
  sp_time_t* seq_b;
  sp_time_t* seq;
  memreg_init(2);
  loop_limit = (size * 10);
  max_unique = sp_stat_unique_max(size, width);
  target = sp_min(target, (sp_stat_repetition_max(size, width)));
  known.size = 0;
  status_require((sph_helper_malloc((max_unique * sizeof(sp_times_counted_sequences_t)), (&counts))));
  memreg_add(counts);
  status_require((sp_times_new(size, (&a_copy))));
  memreg_add(a_copy);
loop:
  if (1 == loop_limit) {
    goto exit;
  };
  loop_limit -= 1;
  if (known.size) {
    sp_sequence_hashtable_clear(known);
  } else {
    sp_sequence_hashtable_new(max_unique, (&known));
  };
  memcpy(a_copy, a, (size * sizeof(sp_time_t)));
  sp_times_counted_sequences_hash(a_copy, size, 2, known);
  sp_times_counted_sequences(known, 0, counts, (&counts_size));
  measured = (max_unique - counts_size);
  if (measured >= target) {
    goto exit;
  };
  difference = (target - measured);
  if (1 < counts_size) {
    quicksort(sp_times_counted_sequences_sort_less, sp_times_counted_sequences_sort_swap, counts, 0, (counts_size - 1));
  };
  for (i = 1; (i < counts_size); i += 1) {
    seq_a = (counts[(i - 1)]).sequence;
    seq_b = (counts[(counts_size - i)]).sequence;
    count = (counts[(i - 1)]).count;
    for (j = 0; (j < size); j += 1) {
      seq = (a + j);
      if (0 == memcmp(seq, seq_a, (width * sizeof(sp_time_t)))) {
        memcpy(seq, seq_b, (width * sizeof(sp_time_t)));
        if (1 == count) {
          break;
        } else {
          count -= 1;
        };
        if (1 == difference) {
          goto loop;
        } else {
          difference -= 1;
        };
      };
    };
  };
  goto loop;
exit:
  memreg_free;
  status_return;
}
/** try to decrease repetition of subseqences of size $width to count $target or nearest possible.
   $a is modified directly.
   new sequences are created using existing elements only (width 1 sequences) */
status_t sp_stat_times_repetition_decrease(sp_time_t* a, sp_time_t size, sp_time_t width, sp_time_t target) {
  status_declare;
  sp_time_t* a_copy;
  sp_time_t counts_size;
  sp_times_counted_sequences_t* counts;
  sp_time_t i;
  sp_time_t j;
  sp_time_t jj;
  sp_sequence_hashtable_t known;
  sp_time_t max_unique_set;
  sp_time_t max_unique;
  sp_time_t max_unique_width;
  sp_time_t measured;
  sp_time_t min_repetition;
  sp_time_t count;
  sp_time_t* seq_new;
  sp_time_t* seq;
  sp_time_t loop_limit;
  sp_time_t* set_indices;
  sp_time_t set_size;
  sp_time_t* set;
  sp_sequence_set_key_t key;
  sp_time_t* value;
  sp_time_t center_i;
  sp_time_t difference;
  if (1 == width) {
    /* not implemented */
    goto exit;
  };
  memreg_init(5);
  status_require((sp_times_new(size, (&set))));
  memreg_add(set);
  status_require((sp_times_deduplicate(a, size, set, (&set_size))));
  max_unique_set = sp_set_sequence_max(set_size, width);
  max_unique_width = sp_stat_unique_max(size, width);
  max_unique = sp_min(max_unique_set, max_unique_width);
  min_repetition = (max_unique_width - max_unique);
  target = sp_max(min_repetition, target);
  known.size = 0;
  key.size = width;
  loop_limit = (size * 10);
  status_require((sp_times_new(width, (&seq_new))));
  memreg_add(seq_new);
  key.data = ((uint8_t*)(seq_new));
  status_require((sp_times_new(width, (&set_indices))));
  memreg_add(set_indices);
  status_require((sph_helper_malloc((max_unique * sizeof(sp_times_counted_sequences_t)), (&counts))));
  memreg_add(counts);
  sp_times_shuffle((&sp_default_random_state), set, set_size);
  status_require((sp_times_new(size, (&a_copy))));
  memreg_add(a_copy);
loop:
  if (1 == loop_limit) {
    goto exit;
  };
  loop_limit -= 1;
  if (known.size) {
    sp_sequence_hashtable_clear(known);
  } else {
    sp_sequence_hashtable_new(max_unique, (&known));
  };
  memcpy(a_copy, a, (size * sizeof(sp_time_t)));
  sp_times_counted_sequences_hash(a_copy, size, width, known);
  sp_times_counted_sequences(known, 0, counts, (&counts_size));
  measured = (max_unique_width - counts_size);
  if (measured <= target) {
    goto exit;
  };
  difference = (measured - target);
  if (1 < counts_size) {
    quicksort(sp_times_counted_sequences_sort_greater, sp_times_counted_sequences_sort_swap, counts, 0, (counts_size - 1));
  };
  for (i = 0; (i < counts_size); i += 1) {
    count = (counts[i]).count;
    if (2 > count) {
      continue;
    };
    seq = (counts[i]).sequence;
    center_i = sp_time_random_bounded((&sp_default_random_state), size);
    /* start at a random index and wrap around to replace more evenly */
    for (jj = 0; (jj < size); jj += 1) {
      j = ((center_i + jj) % size);
      if (0 == memcmp((a + j), seq, (width * sizeof(sp_time_t)))) {
        /* find a new sequence not included */
        do {
          sp_times_select(set, set_indices, width, seq_new);
          sp_times_sequence_increment(set_indices, width, set_size);
          value = sp_sequence_hashtable_get(known, key);
        } while (value);
        memcpy((a + j), seq_new, (width * sizeof(sp_time_t)));
        if (1 == count) {
          break;
        } else {
          count -= 1;
        };
        if (1 == difference) {
          goto loop;
        } else {
          difference -= 1;
        };
      };
    };
  };
  goto loop;
exit:
  memreg_free;
  if (known.size) {
    sp_sequence_hashtable_free(known);
  };
  status_return;
}
