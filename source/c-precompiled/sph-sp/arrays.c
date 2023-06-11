
#include <sph-sp/arrays-template.c>

/* times */
arrays_template(sp_time_t, time, times, sp_inline_no_underflow_subtract, sp_inline_abs);
sp_time_t sp_time_sum(sp_time_t* in, sp_time_t size) {
  sp_time_t sum = 0;
  for (sp_size_t i = 0; (i < size); i += 1) {
    sum += in[i];
  };
  return (sum);
}
sp_time_t sp_times_sum(sp_time_t* a, sp_time_t size) {
  sp_time_t sum = 0;
  for (sp_size_t i = 0; (i < size); i += 1) {
    sum += a[i];
  };
  return (sum);
}

/** display a time array in one line */
void sp_times_display(sp_time_t* in, sp_size_t count) {
  printf("%lu", (in[0]));
  for (sp_size_t i = 1; (i < count); i += 1) {
    printf(" %lu", (in[i]));
  };
  printf("\n");
}

/** generate random integers in the range 0..(cudist-size - 1)
   with probability distribution given via cudist, the cumulative sums of the distribution.
   generates a random number in rango 0..cudist-sum */
void sp_times_random_discrete(sp_time_t* cudist, sp_time_t cudist_size, sp_time_t count, sp_time_t* out) {
  sp_time_t deviate;
  sp_time_t sum;
  sum = cudist[(cudist_size - 1)];
  for (sp_size_t i = 0; (i < count); i += 1) {
    printf("i %lu %lu\n", i, sum);
    deviate = sp_time_random_bounded(sum);
    printf("got deviate");
    for (sp_size_t i1 = 0; (i1 < cudist_size); i1 += 1) {
      if (deviate < cudist[i1]) {
        out[i] = i1;
        break;
      };
    };
  };
}
sp_time_t sp_time_random_discrete(sp_time_t* cudist, sp_time_t cudist_size) {
  sp_time_t deviate = sp_time_random_bounded((cudist[(cudist_size - 1)]));
  for (sp_size_t i = 0; (i < cudist_size); i += 1) {
    if (deviate < cudist[i]) {
      return (i);
    };
  };
  return (cudist_size);
}

/** get a random number in range with a custom probability distribution given by cudist,
   the cumulative sums of the distribution. the resulting number resolution is proportional to cudist-size */
sp_time_t sp_time_random_discrete_bounded(sp_time_t* cudist, sp_time_t cudist_size, sp_time_t range) {
  /* cudist-size minus one because range end is exclusive */
  return ((sp_cheap_round_positive((range * (sp_time_random_discrete(cudist, cudist_size) / ((sp_sample_t)((cudist_size - 1))))))));
}

/** return all permutations of length "size" for "set".
   allocates all needed memory in "out" and passes ownership to caller.
   https://en.wikipedia.org/wiki/Heap's_algorithm */
status_t sp_times_permutations(sp_time_t size, sp_time_t* set, sp_time_t set_size, sp_time_t*** out, sp_time_t* out_size) {
  status_declare;
  sp_time_t* a;
  sp_time_t* b;
  sp_size_t i;
  sp_time_t temp_out_size;
  sp_time_t** temp_out;
  sp_time_t temp_out_used_size;
  sp_time_t* s;
  a = 0;
  b = 0;
  s = 0;
  temp_out = 0;
  i = 0;
  temp_out_size = sp_time_factorial(size);
  srq((sp_times_new(size, (&a))));
  srq((sp_times_new(size, (&s))));
  srq((sp_calloc_type(temp_out_size, sp_time_t*, (&temp_out))));
  srq((sp_times_new(size, (&b))));
  temp_out[0] = b;
  temp_out_used_size = 1;
  sp_times_copy(a, size, set);
  sp_times_copy(b, size, a);
  while ((i < size)) {
    if (s[i] < i) {
      if (i & 1) {
        sp_times_swap(a, (s[i]), i);
      } else {
        sp_times_swap(a, 0, i);
      };
      s[i] += 1;
      i = 0;
      status_require((sph_helper_malloc((size * sizeof(sp_time_t)), (&b))));
      memcpy(b, a, (size * sizeof(sp_time_t)));
      temp_out[temp_out_used_size] = b;
      temp_out_used_size += 1;
    } else {
      s[i] = 0;
      i += 1;
    };
  };
  *out = temp_out;
  *out_size = temp_out_used_size;
exit:
  if (s) {
    free(s);
  };
  if (a) {
    free(a);
  };
  if (status_is_failure) {
    if (temp_out) {
      for (sp_size_t i = 0; (i < temp_out_used_size); i += 1) {
        free((temp_out[i]));
      };
      free(temp_out);
    };
  };
  status_return;
}

/** increment array as if its elements were digits of a written number of base set-size, lower endian */
void sp_times_sequence_increment(sp_time_t* in, sp_size_t size, sp_size_t set_size) {
  for (sp_size_t i = 0; (i < size); i += 1) {
    if (in[i] < (set_size - 1)) {
      in[i] += 1;
      break;
    } else {
      in[i] = 0;
    };
  };
}

/** return all permutations of integers that sum to "sum".
   Kelleher 2006, 'Encoding partitions as ascending compositions' */
status_t sp_times_compositions(sp_time_t sum, sp_time_t*** out, sp_time_t* out_size, sp_time_t** out_sizes) {
  status_declare;
  sp_time_t* a;
  sp_time_t* b;
  sp_time_t k;
  sp_time_t o_size;
  sp_time_t* o_sizes;
  sp_time_t** o;
  sp_time_t o_used;
  sp_time_t sigma;
  sp_time_t b_size;
  sp_time_t x;
  sp_time_t y;
  sigma = 1;
  o = 0;
  o_used = 0;
  a = 0;
  k = 1;
  o_size = sp_time_expt(2, (sum - 1));
  status_require((sph_helper_calloc(((1 + sum) * sizeof(sp_time_t)), (&a))));
  status_require((sph_helper_calloc((o_size * sizeof(sp_time_t*)), (&o))));
  status_require((sph_helper_calloc((o_size * sizeof(sp_time_t)), (&o_sizes))));
  a[1] = sum;
  while (!(0 == k)) {
    x = (a[(k - 1)] + 1);
    y = (a[k] - 1);
    k = (k - 1);
    while ((sigma <= y)) {
      a[k] = x;
      x = sigma;
      y = (y - x);
      k = (k + 1);
    };
    a[k] = (x + y);
    b_size = (k + 1);
    status_require((sph_helper_malloc((b_size * sizeof(sp_time_t)), (&b))));
    memcpy(b, a, (b_size * sizeof(sp_time_t)));
    o[o_used] = b;
    o_sizes[o_used] = b_size;
    o_used = (o_used + 1);
  };
  *out = o;
  *out_size = o_used;
  *out_sizes = o_sizes;
exit:
  if (a) {
    free(a);
  };
  if (status_is_failure) {
    if (o) {
      for (sp_size_t i = 0; (i < o_used); i += 1) {
        free((o[i]));
      };
      free(o);
      free(o_sizes);
    };
  };
  status_return;
}

/** take values at indices and write to out.
   a/out can be the same pointer */
void sp_times_select(sp_time_t* in, sp_time_t* indices, sp_time_t count, sp_time_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = in[indices[i]];
  };
}

/** store the bits of a as uint8 in b. size is the number of bits to take.
   a/out can be the same pointer */
void sp_times_bits_to_times(sp_time_t* a, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  sp_time_t a_i;
  sp_time_t bits_i;
  sp_time_t mask;
  i = 0;
  a_i = 0;
  while ((i < size)) {
    mask = 1;
    bits_i = 0;
    while (((bits_i < (sizeof(sp_time_t) * 8)) && (i < size))) {
      out[i] = ((a[a_i] & mask) ? 1 : 0);
      mask = (mask << 1);
      i += 1;
      bits_i += 1;
    };
    a_i += 1;
  };
}

/** write to out values that are randomly either 1 or 0 */
status_t sp_times_random_binary(sp_time_t size, sp_time_t* out) {
  sp_time_t random_size;
  sp_time_t* temp;
  status_declare;
  random_size = ((size < (sizeof(sp_time_t) * 8)) ? 1 : ((size / (sizeof(sp_time_t) * 8)) + 1));
  status_require((sp_times_new(random_size, (&temp))));
  sp_times_random(random_size, temp);
  sp_times_bits_to_times(temp, size, out);
  free(temp);
exit:
  status_return;
}

/** write to out indices of a that are greater than n
   a/out can be the same pointer */
void sp_times_gt_indices(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out, sp_time_t* out_size) {
  sp_time_t i2 = 0;
  for (sp_size_t i = 0; (i < size); i += 1) {
    if (n < a[i]) {
      out[i2] = i;
      i2 += 1;
    };
  };
  *out_size = i2;
}

/** a/out can not be the same pointer.
   out-size will be less or equal than size.
   memory is allocated and owned by caller */
void sp_times_select_random(sp_time_t* a, sp_time_t size, sp_time_t* out, sp_time_t* out_size) {
  sp_times_random_binary(size, out);
  sp_times_gt_indices(out, size, 0, out, out_size);
  sp_times_select(a, out, (*out_size), out);
}

/** allocate memory for out and set all elements to value */
status_t sp_times_constant(sp_size_t count, sp_time_t value, sp_time_t** out) {
  status_declare;
  sp_time_t* temp;
  status_require((sp_times_new(count, (&temp))));
  sp_times_set(temp, count, value);
  *out = temp;
exit:
  status_return;
}

/** scales in both dimensions, x and y.
   y is scaled by (y * factor), x is scaled by linear interpolation between elements of in-out.
   out size will be (count - 1) * factor */
status_t sp_times_scale(sp_time_t* in, sp_size_t count, sp_time_t factor, sp_time_t* out) {
  status_declare;
  sp_time_t* temp;
  status_require((sp_times_new(count, (&temp))));
  sp_times_copy(in, count, temp);
  sp_times_multiply(temp, count, factor);
  for (sp_size_t i = 1; (i < count); i += 1) {
    for (sp_size_t i2 = 0; (i2 < factor); i2 += 1) {
      out[((factor * (i - 1)) + i2)] = sp_time_interpolate_linear((temp[(i - 1)]), (temp[i]), (i2 / ((sp_sample_t)(factor))));
    };
  };
  free(temp);
exit:
  status_return;
}

/** a array value swap function that can be used with sp-shuffle */
void sp_times_shuffle_swap(void* a, sp_size_t i1, sp_size_t i2) {
  sp_time_t* b;
  b = ((sp_time_t**)(a))[i1];
  ((sp_time_t**)(a))[i1] = ((sp_time_t**)(a))[i2];
  ((sp_time_t**)(a))[i2] = b;
}

/** adjust all values, keeping relative sizes, so that the sum is n.
   a/out can be the same pointer */
void sp_times_scale_sum(sp_time_t* in, sp_size_t count, sp_time_t sum, sp_time_t* out) {
  sp_sample_t factor = (((sp_sample_t)(sum)) / sp_times_sum(in, count));
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = sp_cheap_round_positive((out[i] * factor));
  };
}

/** write count cumulative multiplications with factor from start to out */
void sp_times_multiplications(sp_time_t start, sp_time_t factor, sp_time_t count, sp_time_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = start;
    start *= factor;
  };
}

/** true if array in contains element value */
uint8_t sp_times_contains(sp_time_t* in, sp_size_t count, sp_time_t value) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    if (value == in[i]) {
      return (1);
    };
  };
  return (0);
}

/** create size number of unique discrete random numbers with the distribution given by cudist */
void sp_times_random_discrete_unique(sp_time_t* cudist, sp_time_t cudist_size, sp_time_t size, sp_time_t* out) {
  sp_time_t a;
  sp_time_t remaining;
  remaining = sp_inline_min(size, cudist_size);
  while (remaining) {
    a = sp_time_random_discrete(cudist, cudist_size);
    if (sp_times_contains(out, (size - remaining), a)) {
      continue;
    };
    out[(size - remaining)] = a;
    remaining -= 1;
  };
}

/** append a series of incremented sequences to out, starting with and not modifying the first element in out.
   out size must be at least digits * set-size or base ** digits.
   starts with sequence at out + 0. first generated element will be in out + digits */
void sp_times_sequences(sp_time_t base, sp_time_t digits, sp_time_t set_size, sp_time_t* out) {
  sp_time_t i;
  for (i = digits; (i < (digits * set_size)); i += digits) {
    memcpy((out + i), (out + (i - digits)), (digits * sizeof(sp_time_t)));
    sp_times_sequence_increment((out + i), digits, base);
  };
}

/** interpolate values between $a and $b with interpolation distance fraction 0..1 */
void sp_times_blend(sp_time_t* a, sp_time_t* b, sp_sample_t fraction, sp_time_t size, sp_time_t* out) {
  for (sp_size_t i = 0; (i < size); i += 1) {
    out[i] = sp_cheap_round_positive((sp_time_interpolate_linear((a[i]), (b[i]), fraction)));
  };
}

/** interpolate values pointwise between $a and $b with interpolation distance 0..1 from $coefficients */
void sp_times_mask(sp_time_t* a, sp_time_t* b, sp_sample_t* coefficients, sp_time_t size, sp_time_t* out) {
  for (sp_size_t i = 0; (i < size); i += 1) {
    out[i] = sp_cheap_round_positive((sp_time_interpolate_linear((a[i]), (b[i]), (coefficients[i]))));
  };
}

/** extract values between min/max inclusive and write to out */
void sp_times_extract_in_range(sp_time_t* a, sp_time_t size, sp_time_t min, sp_time_t max, sp_time_t* out, sp_time_t* out_size) {
  *out_size = 0;
  for (sp_size_t i = 0; (i < size); i += 1) {
    if ((a[i] <= max) || (a[i] >= min)) {
      out[i] = a[i];
      *out_size += 1;
    };
  };
}

/** untested. interpolate (b-count / 2) elements at the end of $a progressively more and more towards their mirrored correspondents in $b.
   example: (1 2 3) (4 5 6), interpolates 1 to 6 at 1/6 distance, then 2 to 6 at 2/6 distance, then 3 to 4 at 3/6 istance (3.5) */
void sp_times_make_seamless_right(sp_time_t* a, sp_time_t a_count, sp_time_t* b, sp_time_t b_count, sp_time_t* out) {
  sp_time_t start;
  sp_size_t count;
  count = sp_inline_min(a_count, (b_count / 2));
  start = (a_count - count);
  for (sp_size_t i = 0; (i < start); i += 1) {
    out[i] = a[i];
  };
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[(i + start)] = sp_time_interpolate_linear((a[(i + start)]), (b[(count - (1 + i))]), ((1 + i) / count));
  };
}

/** untested. like sp_times_make_seamless_right but changing the first elements of $b to match the end of $a */
void sp_times_make_seamless_left(sp_time_t* a, sp_time_t a_count, sp_time_t* b, sp_time_t b_count, sp_time_t* out) {
  sp_time_t start;
  sp_time_t count;
  sp_time_t i;
  count = sp_inline_min(b_count, (a_count / 2));
  start = (a_count - count);
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = sp_time_interpolate_linear((b[i]), (a[(a_count - i)]), ((1 + i) / count));
  };
  for (i = (count - 1); (i < b_count); i += 1) {
    out[i] = b[i];
  };
}

/** set all values greater than n in array to n */
void sp_times_limit(sp_time_t* a, sp_time_t count, sp_time_t n, sp_time_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = ((n < a[i]) ? n : a[i]);
  };
}

/** adjust all values, keeping relative sizes, so that the maximum value is target_y.
   a/out can be the same pointer */
void sp_times_scale_y(sp_time_t* in, sp_size_t count, sp_time_t target_y, sp_time_t* out) {
  sp_time_t max = sp_times_absolute_max(in, count);
  sp_times_multiply(in, count, (target_y / max));
}

/** remove count subsequent elements at index from in and write the result to out */
void sp_times_remove(sp_time_t* in, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out) {
  if (0 == index) {
    memcpy(out, (in + count), ((size - count) * sizeof(sp_time_t)));
  } else if ((size - 1) == index) {
    memcpy(out, in, ((size - count) * sizeof(sp_time_t)));
  } else {
    memcpy(out, in, (index * sizeof(sp_time_t)));
    memcpy((out + index), (in + index + count), ((size - index - count) * sizeof(sp_time_t)));
  };
}

/** insert an new area of unset elements before index while copying from in to out */
void sp_times_insert_space(sp_time_t* in, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out) {
  if (0 == index) {
    memcpy((out + count), in, (size * sizeof(sp_time_t)));
  } else {
    memcpy(out, in, (index * sizeof(sp_time_t)));
    memcpy((out + index + count), (in + index), ((size - index) * sizeof(sp_time_t)));
  };
}

/** insert equally spaced values between the two values at $index and $index + 1.
   a:(4 16) index:0 count:3 -> out:(4 7 10 13 16)
   the second value must be greater than the first.
   $index must not be the last index.
   $out size must be at least $size + $count */
void sp_times_subdivide_difference(sp_time_t* a, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out) {
  sp_size_t i;
  sp_time_t interval;
  sp_time_t value;
  sp_times_insert_space(a, size, (index + 1), count, out);
  interval = (a[(index + 1)] - a[index]);
  value = (interval / (count + 1));
  for (i = (index + 1); (i < (index + (count + 1))); i += 1) {
    out[i] = (out[(i - 1)] + value);
  };
}
void sp_times_to_samples(sp_time_t* in, sp_size_t count, sp_sample_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = in[i];
  };
}
status_t sp_times_to_samples_replace(sp_time_t* in, sp_size_t count, sp_sample_t** out) {
  status_declare;
  sp_define_samples_new_srq(temp, count);
  for (sp_size_t i = 0; (i < count); i += 1) {
    temp[i] = in[i];
  };
  free(in);
  *out = temp;
exit:
  status_return;
}

/* samples */
arrays_template(sp_sample_t, sample, samples, sp_subtract, fabs);
/** display a sample array in one line */
void sp_samples_display(sp_sample_t* in, sp_size_t count) {
  printf(("%.5f"), (in[0]));
  for (sp_size_t i = 1; (i < count); i += 1) {
    printf((" %.5f"), (in[i]));
  };
  printf("\n");
}
void sp_samples_to_times(sp_sample_t* in, sp_size_t count, sp_time_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = sp_sample_to_time((in[i]));
  };
}
void sp_samples_to_units(sp_sample_t* in_out, sp_size_t count) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    in_out[i] = ((in_out[i] + 1) / 2);
  };
}
status_t sp_samples_to_times_replace(sp_sample_t* in, sp_size_t count, sp_time_t** out) {
  status_declare;
  sp_define_times_srq(temp, count);
  for (sp_size_t i = 0; (i < count); i += 1) {
    temp[i] = sp_sample_to_time((in[i]));
  };
  free(in);
  *out = temp;
exit:
  status_return;
}
void sp_samples_set_gain(sp_sample_t* in_out, sp_size_t count, sp_sample_t amp) {
  sp_sample_t in_max;
  sp_sample_t difference;
  sp_sample_t correction;
  in_max = sp_samples_absolute_max(in_out, count);
  if ((0 == amp) || (0 == in_max)) {
    return;
  };
  difference = (in_max / amp);
  correction = (1 + ((1 - difference) / difference));
  for (sp_size_t i = 0; (i < count); i += 1) {
    in_out[i] *= correction;
  };
}

/** scale amplitude of in-out to match the one of reference.
   no change if any array is zero */
void sp_samples_set_unity_gain(sp_sample_t* in_out, sp_sample_t* reference, sp_size_t count) { sp_samples_set_gain(in_out, count, (sp_samples_absolute_max(reference, count))); }

/** write to out the differences between subsequent values of a.
   count must be > 1.
   out length will be count - 1 */
void sp_samples_differences(sp_sample_t* a, sp_time_t count, sp_sample_t* out) {
  for (sp_size_t i = 1; (i < count); i += 1) {
    out[(i - 1)] = (a[i] - a[(i - 1)]);
  };
}
sp_sample_t sp_sample_random_discrete_bounded(sp_time_t* cudist, sp_time_t cudist_size, sp_sample_t range) { return ((range * (sp_time_random_discrete(cudist, cudist_size) / ((sp_sample_t)(cudist_size))))); }
void sp_samples_divisions(sp_sample_t start, sp_sample_t divisor, sp_time_t count, sp_sample_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = start;
    start /= divisor;
  };
}

/** adjust all values, keeping relative sizes, so that the maximum value is target_y.
   a/out can be the same pointer */
void sp_samples_scale_y(sp_sample_t* in, sp_time_t count, sp_sample_t target_y) {
  sp_sample_t max = sp_samples_absolute_max(in, count);
  sp_samples_multiply(in, count, (count / max));
}

/** adjust all values, keeping relative sizes, so that the sum is n.
   a/out can be the same pointer */
void sp_samples_scale_sum(sp_sample_t* in, sp_size_t count, sp_sample_t target_y, sp_sample_t* out) {
  sp_sample_t sum = sp_samples_sum(in, count);
  sp_samples_multiply(in, count, (target_y / sum));
}

/** interpolate values pointwise between $a and $b with fraction as a fixed interpolation distance 0..1 */
void sp_samples_blend(sp_sample_t* a, sp_sample_t* b, sp_sample_t fraction, sp_time_t size, sp_sample_t* out) {
  for (sp_size_t i = 0; (i < size); i += 1) {
    out[i] = sp_sample_interpolate_linear((a[i]), (b[i]), fraction);
  };
}

/** set all abs(values) greater than abs(limit) in array to limit, keeping sign.
   in and out can be the same address */
void sp_samples_limit_abs(sp_sample_t* in, sp_time_t count, sp_sample_t limit, sp_sample_t* out) {
  sp_sample_t value;
  for (sp_size_t i = 0; (i < count); i += 1) {
    value = in[i];
    if (0 > value) {
      out[i] = (((-1 * limit) > value) ? (-1 * limit) : value);
    } else {
      out[i] = ((limit < value) ? limit : value);
    };
  };
}
void sp_samples_limit(sp_sample_t* in_out, sp_time_t count, sp_sample_t limit) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    if (limit < in_out[i]) {
      in_out[i] = limit;
    };
  };
}

/* other */

/** generic shuffle that works on any array type. fisher-yates algorithm */
void sp_shuffle(void (*swap)(void*, sp_size_t, sp_size_t), void* in, sp_size_t count) {
  sp_size_t j;
  for (sp_size_t i = 0; (i < count); i += 1) {
    j = (i + sp_time_random_bounded((count - i)));
    swap(in, i, j);
  };
}

/** lower value parts of large types are preferred if
   the system byte order is as expected little-endian */
uint64_t sp_u64_from_array(uint8_t* a, sp_time_t count) {
  if (1 == count) {
    return ((*a));
  } else if (2 == count) {
    return ((*((uint16_t*)(a))));
  } else if (3 == count) {
    return ((*((uint16_t*)(a)) + (((uint64_t)(a[2])) << 16)));
  } else if (4 == count) {
    return ((*((uint32_t*)(a))));
  } else if (5 == count) {
    return ((*((uint32_t*)(a)) + (((uint64_t)(a[4])) << 32)));
  } else if (6 == count) {
    return ((*((uint32_t*)(a)) + (((uint64_t)(*((uint16_t*)((4 + a))))) << 32)));
  } else if (7 == count) {
    return ((*((uint32_t*)(a)) + (((uint64_t)(*((uint16_t*)((4 + a))))) << 32) + (((uint64_t)(a[6])) << 48)));
  } else if (8 == count) {
    return ((*((uint64_t*)(a))));
  };
}
