/* routines that modify arrays to change statistical values */
/** decrease repetition of subseqences of size $width to $target or nearest possible.
   $a is modified directly.
   new sequences are created using existing elements (from width 1 sequences) */
status_t sp_stat_times_repetition_decrease(sp_time_t* a, sp_time_t size, sp_time_t width, sp_time_t target) {
  status_declare;
  sp_time_t counts_size;
  sp_times_counted_sequences_t* counts;
  sp_time_t i;
  sp_time_t j;
  sp_time_t jj;
  sp_time_t max_counts_size;
  sp_time_t max_unique_set;
  sp_time_t max_unique;
  sp_time_t max_unique_width;
  sp_time_t measured;
  sp_time_t min_repetition;
  sp_time_t count;
  sp_time_t* seq_new;
  sp_time_t* seq_prev;
  sp_time_t* seq;
  sp_time_t* set_indices;
  sp_time_t set_size;
  sp_time_t* set;
  sp_time_t center_i;
  sp_time_t difference;
  if (1 == width) {
    /* not implemented */
    status_return;
  };
  memreg_init(4);
  status_require((sp_times_new(size, (&set))));
  memreg_add(set);
  status_require((sp_times_deduplicate(a, size, set, (&set_size))));
  max_unique_set = sp_set_sequence_max(set_size, width);
  max_unique_width = sp_stat_unique_max(size, width);
  max_unique = sp_min(max_unique_set, max_unique_width);
  min_repetition = (max_unique_width - max_unique);
  target = sp_max(min_repetition, target);
  max_counts_size = ((max_unique / 2) + 1);
  status_require((sp_times_new(size, (&seq_new))));
  memreg_add(seq_new);
  status_require((sp_times_new(width, (&set_indices))));
  memreg_add(set_indices);
  status_require((sph_helper_malloc((max_counts_size * sizeof(sp_times_counted_sequences_t)), (&counts))));
  memreg_add(counts);
  sp_times_shuffle((&sp_default_random_state), set, set_size);
loop:
  status_require((sp_times_counted_sequences(a, size, width, 1, counts, (&counts_size), (&measured))));
  if (measured <= target) {
    goto exit;
  };
  difference = (measured - target);
  if (counts_size) {
    quicksort(sp_times_counted_sequences_sort_greater, sp_times_counted_sequences_sort_swap, counts, 0, (counts_size - 1));
  };
  for (i = 0; (i < counts_size); i += 1) {
    seq = (counts[i]).sequence;
    count = (counts[i]).count;
    center_i = sp_time_random_bounded((&sp_default_random_state), size);
    /* start at a random index and wrap around */
    for (jj = 0; (jj < size); jj += 1) {
      j = ((center_i + jj) % size);
      if (0 == memcmp((a + j), seq, (width * sizeof(sp_time_t)))) {
        sp_times_select(set, set_indices, width, seq_new);
        sp_times_sequence_increment(set_indices, width, set_size);
        memcpy((a + j), seq_new, (width * sizeof(sp_time_t)));
        if (1 == count) {
          continue;
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
