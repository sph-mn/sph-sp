/* routines on arrays of sp-time-t or sp-sample-t
sp-time-t subtraction is limited to zero.
sp-time-t addition is not limited and large values that might lead to overflows are considered special cases. */
status_t sp_samples_new(sp_time_t size, sp_sample_t** out) { return ((sph_helper_calloc((size * sizeof(sp_sample_t)), out))); }
status_t sp_times_new(sp_time_t size, sp_time_t** out) { return ((sph_helper_calloc((size * sizeof(sp_time_t)), out))); }
void sp_samples_to_times(sp_sample_t* in, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  for (i = 0; (i < size); i = (1 + i)) {
    out[i] = sp_cheap_round_positive((in[i]));
  };
}
/** get the maximum value in samples array, disregarding sign */
sp_sample_t sp_samples_absolute_max(sp_sample_t* in, sp_time_t in_size) {
  sp_sample_t result;
  sp_sample_t a;
  sp_time_t i;
  for (i = 0, result = 0; (i < in_size); i = (1 + i)) {
    a = fabs((in[i]));
    if (a > result) {
      result = a;
    };
  };
  return (result);
}
/** display a sample array in one line */
void sp_samples_display(sp_sample_t* a, sp_time_t size) {
  sp_time_t i;
  printf(("%.5f"), (a[0]));
  for (i = 1; (i < size); i = (1 + i)) {
    printf((" %.5f"), (a[i]));
  };
  printf("\n");
}
/** display a time array in one line */
void sp_times_display(sp_time_t* a, sp_time_t size) {
  sp_time_t i;
  printf("%lu", (a[0]));
  for (i = 1; (i < size); i = (1 + i)) {
    printf(" %lu", (a[i]));
  };
  printf("\n");
}
/** adjust amplitude of out to match the one of in */
void sp_samples_set_unity_gain(sp_sample_t* in, sp_time_t in_size, sp_sample_t* out) {
  sp_time_t i;
  sp_sample_t in_max;
  sp_sample_t out_max;
  sp_sample_t difference;
  sp_sample_t correction;
  in_max = sp_samples_absolute_max(in, in_size);
  out_max = sp_samples_absolute_max(out, in_size);
  if ((0 == in_max) || (0 == out_max)) {
    return;
  };
  difference = (out_max / in_max);
  correction = (1 + ((1 - difference) / difference));
  for (i = 0; (i < in_size); i = (1 + i)) {
    out[i] = (correction * out[i]);
  };
}
#define absolute_difference(a, b) ((a > b) ? (a - b) : (b - a))
#define no_underflow_subtract(a, b) ((a > b) ? (a - b) : 0)
#define no_zero_divide(a, b) ((0 == b) ? 0 : (a / b))
/** functions that work on sp-sample-t and sp-time-t */
#define define_value_functions(prefix, value_t) \
  void prefix##_sort_swap(void* a, ssize_t b, ssize_t c) { \
    value_t d; \
    d = ((value_t*)(a))[b]; \
    ((value_t*)(a))[b] = ((value_t*)(a))[c]; \
    ((value_t*)(a))[c] = d; \
  } \
  uint8_t prefix##_sort_less_p(void* a, ssize_t b, ssize_t c) { return ((((value_t*)(a))[b] < ((value_t*)(a))[c])); }
/** functions that work on sp-samples-t and sp-times-t */
#define define_array_functions(prefix, value_t) \
  /** a/out can not be the same pointer */ \
  void prefix##_reverse(value_t* a, sp_time_t size, value_t* out) { \
    sp_time_t i; \
    for (i = 0; (i < size); i += 1) { \
      out[(size - i)] = a[i]; \
    }; \
  } \
\
  /** set the minimum, maximum and return the differnce between them. \
         array length must be > 0 */ \
  value_t prefix##_range(value_t* a, sp_time_t size, value_t* out_min, value_t* out_max) { \
    sp_time_t i; \
    value_t min; \
    value_t max; \
    value_t b; \
    min = a[0]; \
    max = min; \
    for (i = 1; (i < size); i += 1) { \
      b = a[i]; \
      if (b > max) { \
        max = b; \
      } else { \
        if (b < min) { \
          min = b; \
        }; \
      }; \
    }; \
    *out_min = min; \
    *out_max = max; \
    return ((max - min)); \
  } \
  uint8_t prefix##_equal_1(value_t* a, sp_time_t size, value_t n) { \
    sp_time_t i; \
    for (i = 0; (i < size); i += 1) { \
      if (!(n == a[i])) { \
        return (0); \
      }; \
    }; \
    return (1); \
  } \
\
  /** if a[i] and b[i] greater than limit, take b[i] else 0 */ \
  void prefix##_and(value_t* a, value_t* b, sp_time_t size, value_t limit, value_t* out) { \
    sp_time_t i; \
    for (i = 0; (i < size); i += 1) { \
      if ((a[i] >= limit) && (a[i] >= limit)) { \
        out[i] = b[i]; \
      } else { \
        out[i] = 0; \
      }; \
    }; \
  } \
\
  /** if a[i] < limit and b[i] > limit, take b[i], else a[i] */ \
  void prefix##_or(value_t* a, value_t* b, sp_time_t size, value_t limit, value_t* out) { \
    sp_time_t i; \
    for (i = 0; (i < size); i += 1) { \
      if (a[i] < limit) { \
        if (b[i] < limit) { \
          out[i] = a[i]; \
        } else { \
          out[i] = b[i]; \
        }; \
      } else { \
        out[i] = a[i]; \
      }; \
    }; \
  } \
\
  /** if a[i] > limit and b[i] > limit then 0 else take the one greater than limit */ \
  void prefix##_xor(value_t* a, value_t* b, sp_time_t size, value_t limit, value_t* out) { \
    sp_time_t i; \
    for (i = 0; (i < size); i += 1) { \
      if ((a[i] >= limit) && (b[i] >= limit)) { \
        out[i] = 0; \
      } else { \
        if (a[i] < limit) { \
          if (b[i] < limit) { \
            out[i] = a[i]; \
          } else { \
            out[i] = b[i]; \
          }; \
        } else { \
          out[i] = a[i]; \
        }; \
      }; \
    }; \
  }
#define define_array_mapper(name, value_t, transfer) \
  void name(value_t* a, sp_time_t size, value_t* out) { \
    sp_time_t i; \
    for (i = 0; (i < size); i += 1) { \
      out[i] = transfer; \
    }; \
  }
#define define_array_combinator(name, value_t, transfer) \
  /** a/out can be the same */ \
  void name(value_t* a, sp_time_t size, value_t* b, value_t* out) { \
    sp_time_t i; \
    for (i = 0; (i < size); i += 1) { \
      out[i] = transfer; \
    }; \
  }
#define define_array_combinator_1(name, value_t, transfer) \
  /** a/out can be the same */ \
  void name(value_t* a, sp_time_t size, value_t n, value_t* out) { \
    sp_time_t i; \
    for (i = 0; (i < size); i += 1) { \
      out[i] = transfer; \
    }; \
  }
#define sequence_set_equal(a, b) ((a.size == b.size) && (((0 == a.size) && (0 == b.size)) || !memcmp((a.data), (b.data), (a.size))))
uint8_t sp_samples_every_equal(sp_sample_t* a, sp_time_t size, sp_sample_t n) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    if (!(n == a[i])) {
      return (0);
    };
  };
  return (1);
}
define_value_functions(sp_times, sp_time_t)
  define_value_functions(sp_samples, sp_sample_t)
    define_array_functions(sp_times, sp_time_t)
      define_array_functions(sp_samples, sp_sample_t)
        define_array_mapper(sp_times_square, sp_time_t, (a[i] * a[i]))
          define_array_mapper(sp_samples_square, sp_sample_t, (a[i] * a[i]))
            define_array_combinator(sp_times_add, sp_time_t, (a[i] + b[i]))
              define_array_combinator(sp_samples_add, sp_sample_t, (a[i] + b[i]))
                define_array_combinator(sp_times_subtract, sp_time_t, (no_underflow_subtract((a[i]), (b[i]))))
                  define_array_combinator(sp_samples_subtract, sp_sample_t, (a[i] - b[i]))
                    define_array_combinator(sp_times_multiply, sp_time_t, (a[i] * b[i]))
                      define_array_combinator(sp_samples_multiply, sp_sample_t, (a[i] * b[i]))
                        define_array_combinator(sp_times_divide, sp_time_t, (a[i] / b[i]))
                          define_array_combinator(sp_samples_divide, sp_sample_t, (a[i] / b[i]))
                            define_array_combinator_1(sp_times_add_1, sp_time_t, (a[i] + n))
                              define_array_combinator_1(sp_samples_add_1, sp_sample_t, (a[i] + n))
                                define_array_combinator_1(sp_times_subtract_1, sp_time_t, (no_underflow_subtract((a[i]), n)))
                                  define_array_combinator_1(sp_samples_subtract_1, sp_sample_t, (a[i] - n))
                                    define_array_combinator_1(sp_times_multiply_1, sp_time_t, (a[i] * n))
                                      define_array_combinator_1(sp_times_divide_1, sp_time_t, (a[i] / n))
                                        define_array_combinator_1(sp_times_set_1, sp_time_t, n)
                                          define_array_combinator_1(sp_samples_set_1, sp_sample_t, n)
                                            define_array_combinator_1(sp_samples_multiply_1, sp_sample_t, (a[i] * n))
                                              define_array_combinator_1(sp_samples_divide_1, sp_sample_t, (a[i] / n))
                                                uint64_t sp_u64_from_array(uint8_t* a, sp_time_t size) {
  if (1 == size) {
    return ((*a));
  } else if (2 == size) {
    return ((*((uint16_t*)(a))));
  } else if (3 == size) {
    return ((*((uint16_t*)(a)) + (((uint64_t)(a[2])) << 16)));
  } else if (4 == size) {
    return ((*((uint32_t*)(a))));
  } else if (5 == size) {
    return ((*((uint32_t*)(a)) + (((uint64_t)(a[4])) << 32)));
  } else if (6 == size) {
    return ((*((uint32_t*)(a)) + (((uint64_t)(*((uint16_t*)((4 + a))))) << 32)));
  } else if (7 == size) {
    return ((*((uint32_t*)(a)) + (((uint64_t)(*((uint16_t*)((4 + a))))) << 32) + (((uint64_t)(a[6])) << 48)));
  } else if (8 == size) {
    return ((*((uint64_t*)(a))));
  };
}
typedef struct {
  sp_time_t size;
  uint8_t* data;
} sequence_set_key_t;
sequence_set_key_t sequence_set_null = { 0, 0 };
uint64_t sequence_set_hash(sequence_set_key_t a, sp_time_t memory_size) { (sp_u64_from_array((a.data), (a.size)) % memory_size); }
sph_set_declare_type_nonull(sequence_set, sequence_set_key_t, sequence_set_hash, sequence_set_equal, sequence_set_null, 2);
/* samples */
status_t sp_samples_copy(sp_sample_t* a, sp_time_t size, sp_sample_t** out) {
  status_declare;
  sp_sample_t* temp;
  status_require((sp_samples_new(size, (&temp))));
  memcpy(temp, a, (size * sizeof(sp_sample_t)));
  *out = temp;
exit:
  status_return;
}
sp_sample_t sp_samples_mean(sp_sample_t* a, sp_time_t size) { return ((sp_samples_sum(a, size) / size)); }
sp_sample_t sp_samples_std_dev(sp_sample_t* a, sp_time_t size, sp_sample_t mean) {
  sp_time_t i;
  sp_sample_t sum;
  sp_sample_t dev;
  sum = 0;
  for (i = 0; (i < size); i += 1) {
    dev = (mean - a[i]);
    sum = (sum + (dev * dev));
  };
  return ((sum / size));
}
/** size of a must be equal or greater than b */
sp_sample_t sp_samples_covariance(sp_sample_t* a, sp_time_t size, sp_sample_t* b) {
  sp_time_t i;
  sp_sample_t sum;
  sp_sample_t mean_a;
  sp_sample_t mean_b;
  mean_a = sp_samples_mean(a, size);
  mean_b = sp_samples_mean(b, size);
  for (i = 0; (i < size); i += 1) {
    sum += ((a[i] - mean_a) * (b[i] - mean_b));
  };
  return ((sum / size));
}
sp_sample_t sp_samples_correlation(sp_sample_t* a, sp_time_t size, sp_sample_t* b) { (sp_samples_covariance(a, size, b) / (sp_samples_std_dev(a, size, (sp_samples_mean(a, size))) * sp_samples_std_dev(b, size, (sp_samples_mean(b, size))))); }
/** lag must not be greater then the size of a */
sp_sample_t sp_samples_autocorrelation(sp_sample_t* a, sp_time_t size, sp_time_t lag) {
  sp_sample_t* b;
  return ((sp_samples_correlation(a, (size - lag), (a + lag))));
}
sp_sample_t sp_samples_center_of_mass(sp_sample_t* a, sp_time_t size) {
  sp_time_t i;
  sp_sample_t sum;
  sp_sample_t index_sum;
  index_sum = 0;
  sum = sp_samples_sum(a, size);
  for (i = 1; (i < size); i += 1) {
    index_sum += (i * a[i]);
  };
  return ((index_sum / sum));
}
/** size must be > 1 */
void sp_samples_differences(sp_sample_t* a, sp_time_t size, sp_sample_t* out) {
  sp_time_t i;
  for (i = 1; (i < size); i += 1) {
    out[(i - 1)] = (a[i] - a[(i - 1)]);
  };
}
/** a is copied to temp. size of temp must be >= size */
sp_sample_t sp_samples_median(sp_sample_t* a, sp_time_t size, sp_sample_t* temp) {
  memcpy(temp, a, (size * sizeof(sp_sample_t)));
  quicksort(sp_samples_sort_less_p, sp_samples_sort_swap, temp, 0, (size - 1));
  return (((temp[(size / 2)] + temp[((size / 2) - 1)]) / 2));
}
/* times */
status_t sp_times_copy(sp_time_t a, sp_time_t size, sp_time_t** out) {
  status_declare;
  sp_time_t* temp;
  status_require((sp_times_new(size, (&temp))));
  *out = temp;
exit:
  status_return;
}
/** calculate the arithmetic mean over all values in array.
   array length must be > 0 */
sp_sample_t sp_times_mean(sp_time_t* a, sp_time_t size) {
  sp_time_t i;
  sp_time_t sum;
  sum = 0;
  for (i = 0; (i < size); i += 1) {
    sum += a[i];
  };
  return ((sum / ((sp_sample_t)(size))));
}
sp_sample_t sp_times_std_dev(sp_time_t* a, sp_time_t size, sp_time_t mean) {
  sp_time_t i;
  sp_time_t sum;
  sp_time_t dev;
  sum = 0;
  for (i = 0; (i < size); i += 1) {
    dev = absolute_difference(mean, (a[i]));
    sum = (sum + (dev * dev));
  };
  return ((sum / ((sp_sample_t)(size))));
}
sp_sample_t sp_times_center_of_mass(sp_time_t* a, sp_time_t size) {
  sp_time_t i;
  sp_time_t sum;
  sp_time_t index_sum;
  index_sum = 0;
  sum = a[0];
  for (i = 1; (i < size); i += 1) {
    sum += a[i];
    index_sum += (i * a[i]);
  };
  return ((index_sum / ((sp_sample_t)(sum))));
}
/** a.size must be > 1. a.size minus 1 elements will be written to out */
void sp_times_differences(sp_time_t* a, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  for (i = 1; (i < size); i += 1) {
    out[(i - 1)] = absolute_difference((a[i]), (a[(i - 1)]));
  };
}
/** a is copied to temp. size of temp must be >= size */
sp_sample_t sp_times_median(sp_time_t* a, sp_time_t size, sp_time_t* temp) {
  memcpy(temp, a, (size * sizeof(sp_time_t)));
  quicksort(sp_times_sort_less_p, sp_times_sort_swap, temp, 0, (size - 1));
  return (((size & 1) ? temp[((size / 2) - 1)] : ((temp[(size / 2)] + temp[((size / 2) - 1)]) / 2.0)));
}
/** calculate cumulative sums from the given numbers.
   (a b c ...) -> (a (+ a b) (+ a b c) ...) */
void sp_times_cusum(sp_time_t* a, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  sp_time_t sum;
  sum = a[0];
  out[0] = sum;
  for (i = 0; (i < size); i += 1) {
    sum = (sum + a[i]);
    out[i] = sum;
  };
}
/** create random numbers with a given probability distribution */
void sp_times_random_discrete(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size, sp_time_t count, sp_time_t* out) {
  sp_time_t deviate;
  sp_time_t sum;
  sp_time_t i;
  sp_time_t i1;
  sum = cudist[(cudist_size - 1)];
  for (i = 0; (i < count); i += 1) {
    sp_times_random(state, 1, (&deviate));
    deviate = (deviate % sum);
    for (i1 = 0; (i1 < cudist_size); i1 += 1) {
      if (deviate < cudist[i1]) {
        out[i] = i1;
        break;
      };
    };
  };
}
/** count unique subsequences */
status_t sp_times_sequence_count(sp_time_t* a, sp_time_t size, sp_time_t min_width, sp_time_t max_width, sp_time_t step_width, sp_time_t* out) {
  status_declare;
  sp_time_t width;
  sp_time_t i;
  sequence_set_t known;
  sequence_set_key_t key;
  sequence_set_key_t* value;
  sp_time_t result;
  if (sequence_set_new(size, (&known))) {
    sp_memory_error;
  };
  for (width = min_width, result = 0; (width <= max_width); width += step_width) {
    key.size = width;
    for (i = 0; (i <= (size - width)); i += 1) {
      key.data = ((uint8_t*)((i + a)));
      value = sequence_set_get(known, key);
      if (!value) {
        result += 1;
        if (!sequence_set_add(known, key)) {
          status_set_goto(sp_s_group_sp, sp_s_id_undefined);
        };
      };
    };
    sequence_set_clear(known);
  };
  *out = result;
exit:
  status_return;
}
void sp_times_swap(sp_time_t* a, ssize_t i1, ssize_t i2) {
  sp_time_t temp;
  temp = a[i1];
  a[i1] = a[i2];
  a[i2] = temp;
}
/** increment array as if its elements were digits of a written number of base set-size */
void sp_times_sequence_increment_le(sp_time_t* a, sp_time_t size, sp_time_t set_size) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    if (a[i] < (set_size - 1)) {
      a[i] += 1;
      break;
    } else {
      a[i] = 0;
    };
  };
}
/** return all permutations of integers that sum to "sum".
   Kelleher 2006, 'Encoding partitions as ascending compositions' */
status_t sp_times_compositions(sp_time_t sum, sp_time_t*** out, sp_time_t* out_size, sp_time_t** out_sizes) {
  status_declare;
  sp_time_t* a;
  sp_time_t* b;
  sp_time_t i;
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
      for (i = 0; (i < o_used); i += 1) {
        free((o[i]));
      };
      free(o);
      free(o_sizes);
    };
  };
  status_return;
}
/** return all permutations of length "size" for "set".
   allocates all needed memory in "out" and passes ownership to caller.
   https://en.wikipedia.org/wiki/Heap's_algorithm */
status_t sp_times_permutations(sp_time_t size, sp_time_t* set, sp_time_t set_size, sp_time_t*** out, sp_time_t* out_size) {
  sp_time_t* a;
  sp_time_t* b;
  sp_time_t i;
  sp_time_t o_size;
  sp_time_t** o;
  sp_time_t o_used;
  sp_time_t* s;
  status_declare;
  a = 0;
  b = 0;
  s = 0;
  o = 0;
  i = 0;
  o_used = 0;
  o_size = sp_time_factorial(size);
  status_require((sph_helper_malloc((size * sizeof(sp_time_t)), (&a))));
  status_require((sph_helper_calloc((size * sizeof(sp_time_t)), (&s))));
  status_require((sph_helper_calloc((o_size * sizeof(sp_time_t*)), (&o))));
  /* ensure that new b are always added to o */
  status_require((sph_helper_malloc((size * sizeof(sp_time_t)), (&b))));
  o[o_used] = b;
  o_used = (o_used + 1);
  memcpy(a, set, (size * sizeof(sp_time_t)));
  memcpy(b, a, (size * sizeof(sp_time_t)));
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
      o[o_used] = b;
      o_used = (o_used + 1);
    } else {
      s[i] = 0;
      i = (i + 1);
    };
  };
  *out = o;
  *out_size = o_used;
exit:
  if (s) {
    free(s);
  };
  if (a) {
    free(a);
  };
  if (status_is_failure) {
    if (o) {
      for (i = 0; (i < o_used); i += 1) {
        free((o[i]));
      };
      free(o);
    };
  };
  status_return;
}
void sp_samples_divisions(sp_sample_t start, sp_sample_t n, sp_time_t count, sp_sample_t* out) {
  sp_time_t i;
  for (i = 0; (i < count); i += 1) {
    out[i] = start;
    start /= n;
  };
}
/** adjust all values, keeping relative sizes, so that the maximum value is not greater than n.
   a/out can be the same pointer */
void sp_samples_scale(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out) {
  sp_sample_t max = sp_samples_absolute_max(a, size);
  sp_samples_multiply_1(a, size, (n / max), a);
}
/** adjust all values, keeping relative sizes, so that the sum is not greater than n.
   a/out can be the same pointer */
void sp_samples_scale_sum(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out) {
  sp_sample_t sum = sp_samples_sum(a, size);
  sp_samples_multiply_1(a, size, (n / sum), a);
}
/** write count cumulative multiplications with factor from start to out */
void sp_times_multiplications(sp_time_t start, sp_time_t factor, sp_time_t count, sp_time_t* out) {
  sp_time_t i;
  for (i = 0; (i < count); i += 1) {
    out[i] = start;
    start *= factor;
  };
}
/** write count cumulative additions with summand from start to out */
void sp_times_additions(sp_time_t start, sp_time_t summand, sp_time_t count, sp_time_t* out) {
  sp_time_t i;
  for (i = 0; (i < count); i += 1) {
    out[i] = start;
    start += summand;
  };
}
/** write count cumulative additions with summand from start to out */
void sp_samples_additions(sp_sample_t start, sp_sample_t summand, sp_time_t count, sp_sample_t* out) {
  sp_time_t i;
  for (i = 0; (i < count); i += 1) {
    out[i] = start;
    start += summand;
  };
}
/** take values at indices and write in order to out.
   a/out can be the same pointer */
void sp_times_extract_at_indices(sp_time_t* a, sp_time_t* indices, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    out[i] = a[indices[i]];
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
/** https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm */
void sp_times_shuffle(sp_random_state_t* state, sp_time_t* a, sp_time_t size) {
  sp_time_t i;
  sp_time_t j;
  sp_time_t t;
  for (i = 0; (i < size); i += 1) {
    j = (i + sp_time_random_bounded(state, (size - i)));
    t = a[i];
    a[i] = a[j];
    a[j] = t;
  };
}
/** write to out values that are randomly either 1 or 0 */
status_t sp_times_random_binary(sp_random_state_t* state, sp_time_t size, sp_time_t* out) {
  sp_time_t random_size;
  sp_time_t* temp;
  status_declare;
  random_size = ((size < (sizeof(sp_time_t) * 8)) ? 1 : ((size / (sizeof(sp_time_t) * 8)) + 1));
  status_require((sp_times_new(random_size, (&temp))));
  sp_times_random(state, random_size, temp);
  sp_times_bits_to_times(temp, size, out);
  free(temp);
exit:
  status_return;
}
/** out can be "a" */
void sp_times_gt_indices(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out, sp_time_t* out_size) {
  sp_time_t i;
  sp_time_t i2;
  for (i = 0, i2 = 0; (i < size); i += 1) {
    if (n < a[i]) {
      out[i2] = i;
      i2 = (i2 + 1);
    };
  };
  *out_size = i2;
}
/** a/out can not be the same pointer.
   out-size will be less or equal than size */
void sp_times_extract_random(sp_random_state_t* state, sp_time_t* a, sp_time_t size, sp_time_t* out, sp_time_t* out_size) {
  status_declare;
  sp_times_random_binary(state, size, out);
  sp_times_gt_indices(out, size, 0, out, out_size);
  sp_times_extract_at_indices(a, out, (*out_size), out);
}
status_t sp_times_constant(sp_time_t a, sp_time_t size, sp_time_t** out) {
  sp_time_t* temp;
  status_declare;
  status_require((sp_times_new(size, (&temp))));
  sp_times_set_1(temp, size, 4, temp);
  *out = temp;
exit:
  status_return;
}
