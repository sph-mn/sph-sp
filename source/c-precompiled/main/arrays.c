/* utilities for arrays of sp-time-t or sp-sample-t.
sp-time-t subtraction is limited to zero.
sp-time-t addition is not limited. */
/** generic shuffle that works on any array type. fisher-yates algorithm */
void sp_shuffle(sp_random_state_t* state, void (*swap)(void*, size_t, size_t), void* a, size_t size) {
  size_t i;
  size_t j;
  for (i = 0; (i < size); i += 1) {
    j = (i + sp_time_random_bounded(state, (size - i)));
    swap(a, i, j);
  };
}
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
/** scale amplitude of out to match the one of in */
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
    out[i] *= correction;
  };
}
/** functions that work on single sp-sample-t and sp-time-t */
#define define_value_functions(prefix, value_t) \
  void prefix##_sort_swap(void* a, ssize_t b, ssize_t c) { \
    value_t d; \
    d = ((value_t*)(a))[b]; \
    ((value_t*)(a))[b] = ((value_t*)(a))[c]; \
    ((value_t*)(a))[c] = d; \
  } \
  uint8_t prefix##_sort_less(void* a, ssize_t b, ssize_t c) { return ((((value_t*)(a))[b] < ((value_t*)(a))[c])); }
/** functions that work the same on sp-sample-t* and sp-time-t* */
#define define_array_functions(prefix, value_t) \
  /** a/out can not be the same pointer */ \
  void prefix##_reverse(value_t* a, sp_time_t size, value_t* out) { \
    sp_time_t i; \
    for (i = 0; (i < size); i += 1) { \
      out[(size - i)] = a[i]; \
    }; \
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
                define_array_combinator(sp_times_subtract, sp_time_t, (sp_no_underflow_subtract((a[i]), (b[i]))))
                  define_array_combinator(sp_samples_subtract, sp_sample_t, (a[i] - b[i]))
                    define_array_combinator(sp_times_multiply, sp_time_t, (a[i] * b[i]))
                      define_array_combinator(sp_samples_multiply, sp_sample_t, (a[i] * b[i]))
                        define_array_combinator(sp_times_divide, sp_time_t, (a[i] / b[i]))
                          define_array_combinator(sp_samples_divide, sp_sample_t, (a[i] / b[i]))
                            define_array_combinator_1(sp_times_add_1, sp_time_t, (a[i] + n))
                              define_array_combinator_1(sp_samples_add_1, sp_sample_t, (a[i] + n))
                                define_array_combinator_1(sp_times_subtract_1, sp_time_t, (sp_no_underflow_subtract((a[i]), n)))
                                  define_array_combinator_1(sp_samples_subtract_1, sp_sample_t, (a[i] - n))
                                    define_array_combinator_1(sp_times_multiply_1, sp_time_t, (a[i] * n))
                                      define_array_combinator_1(sp_times_divide_1, sp_time_t, (a[i] / n))
                                        define_array_combinator_1(sp_times_set_1, sp_time_t, n)
                                          define_array_combinator_1(sp_samples_set_1, sp_sample_t, n)
                                            define_array_combinator_1(sp_samples_multiply_1, sp_sample_t, (a[i] * n))
                                              define_array_combinator_1(sp_samples_divide_1, sp_sample_t, (a[i] / n))
  /** lower value parts of large types are preferred if
   the system byte order is as expected little-endian */
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
// samples
status_t sp_samples_duplicate(sp_sample_t* a, sp_time_t size, sp_sample_t** out) {
  status_declare;
  sp_sample_t* temp;
  status_require((sp_samples_new(size, (&temp))));
  memcpy(temp, a, (size * sizeof(sp_sample_t)));
  *out = temp;
exit:
  status_return;
}
/** write to out the differences between subsequent values of a.
   size must be > 1.
   out-size will be size - 1 */
void sp_samples_differences(sp_sample_t* a, sp_time_t size, sp_sample_t* out) {
  sp_time_t i;
  for (i = 1; (i < size); i += 1) {
    out[(i - 1)] = (a[i] - a[(i - 1)]);
  };
}
// times
status_t sp_times_duplicate(sp_time_t a, sp_time_t size, sp_time_t** out) {
  status_declare;
  sp_time_t* temp;
  status_require((sp_times_new(size, (&temp))));
  *out = temp;
exit:
  status_return;
}
/** a.size must be > 1. a.size minus 1 elements will be written to out */
void sp_times_differences(sp_time_t* a, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  for (i = 1; (i < size); i += 1) {
    out[(i - 1)] = sp_absolute_difference((a[i]), (a[(i - 1)]));
  };
}
/** calculate cumulative sums from the given numbers.
   (a b c ...) -> (a (+ a b) (+ a b c) ...) */
void sp_times_cusum(sp_time_t* a, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  sp_time_t sum;
  sum = a[0];
  out[0] = sum;
  for (i = 1; (i < size); i += 1) {
    sum = (sum + a[i]);
    out[i] = sum;
  };
}
/** generate random integers in the range 0..(cudist-size - 1)
   with probability distribution given via cudist, the cumulative sums of the distribution */
void sp_times_random_discrete(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size, sp_time_t count, sp_time_t* out) {
  sp_time_t deviate;
  sp_time_t sum;
  sp_time_t i;
  sp_time_t i1;
  sum = cudist[(cudist_size - 1)];
  for (i = 0; (i < count); i += 1) {
    deviate = sp_time_random_bounded(state, sum);
    for (i1 = 0; (i1 < cudist_size); i1 += 1) {
      if (deviate < cudist[i1]) {
        out[i] = i1;
        break;
      };
    };
  };
}
sp_time_t sp_time_random_discrete(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size) {
  sp_time_t deviate;
  sp_time_t i;
  deviate = sp_time_random_bounded(state, (cudist[(cudist_size - 1)]));
  for (i = 0; (i < cudist_size); i += 1) {
    if (deviate < cudist[i]) {
      return (i);
    };
  };
  return (cudist_size);
}
/** get a random number in range with a custom probability distribution given by cudist,
   the cumulative sums of the distribution. the resulting number resolution is proportional to cudist-size */
sp_time_t sp_time_random_custom(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size, sp_time_t range) {
  // cudist-size minus one because range end is exclusive
  return ((sp_cheap_round_positive((range * (sp_time_random_discrete(state, cudist, cudist_size) / ((sp_sample_t)((cudist_size - 1))))))));
}
sp_sample_t sp_sample_random_custom(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size, sp_sample_t range) { return ((range * (sp_time_random_discrete(state, cudist, cudist_size) / ((sp_sample_t)(cudist_size))))); }
void sp_times_swap(sp_time_t* a, ssize_t i1, ssize_t i2) {
  sp_time_t temp;
  temp = a[i1];
  a[i1] = a[i2];
  a[i2] = temp;
}
/** increment array as if its elements were digits of a written number of base set-size, lower endian */
void sp_times_sequence_increment(sp_time_t* a, sp_time_t size, sp_time_t set_size) {
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
  // ensure that new b are always added to o
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
/** adjust all values, keeping relative sizes, so that the maximum value is n.
   a/out can be the same pointer */
void sp_samples_scale_y(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out) {
  sp_sample_t max = sp_samples_absolute_max(a, size);
  sp_samples_multiply_1(a, size, (n / max), a);
}
/** adjust all values, keeping relative sizes, so that the sum is n.
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
/** take values at indices and write to out.
   a/out can be the same pointer */
void sp_times_select(sp_time_t* a, sp_time_t* indices, sp_time_t size, sp_time_t* out) {
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
/** modern yates shuffle.
   https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm */
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
/** write to out indices of a that are greater than n
   a/out can be the same pointer */
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
   out-size will be less or equal than size.
   memory is allocated and owned by caller */
void sp_times_select_random(sp_random_state_t* state, sp_time_t* a, sp_time_t size, sp_time_t* out, sp_time_t* out_size) {
  status_declare;
  sp_times_random_binary(state, size, out);
  sp_times_gt_indices(out, size, 0, out, out_size);
  sp_times_select(a, out, (*out_size), out);
}
/** allocate memory for out and set all elements to value */
status_t sp_times_constant(sp_time_t a, sp_time_t size, sp_time_t value, sp_time_t** out) {
  sp_time_t* temp;
  status_declare;
  status_require((sp_times_new(size, (&temp))));
  sp_times_set_1(temp, size, value, temp);
  *out = temp;
exit:
  status_return;
}
/** expand by factor. y is scaled by (y * factor), x is scaled by linear interpolation between elements of a.
   out size will be (a-size - 1) * factor */
status_t sp_times_scale(sp_time_t* a, sp_time_t a_size, sp_time_t factor, sp_time_t* out) {
  status_declare;
  sp_time_t i;
  sp_time_t i2;
  sp_time_t* aa;
  status_require((sp_times_new(a_size, (&aa))));
  sp_times_multiply_1(a, a_size, factor, aa);
  for (i = 1; (i < a_size); i += 1) {
    for (i2 = 0; (i2 < factor); i2 += 1) {
      out[((factor * (i - 1)) + i2)] = sp_time_interpolate_linear((aa[(i - 1)]), (aa[i]), (i2 / ((sp_sample_t)(factor))));
    };
  };
  free(aa);
exit:
  status_return;
}
/** a array value swap function that can be used with sp-shuffle */
void sp_times_shuffle_swap(void* a, size_t i1, size_t i2) {
  sp_time_t* b;
  b = ((sp_time_t**)(a))[i1];
  ((sp_time_t**)(a))[i1] = ((sp_time_t**)(a))[i2];
  ((sp_time_t**)(a))[i2] = b;
}
/** free every element array and the container array */
void sp_times_array_free(sp_time_t** a, sp_time_t size) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    free((a[i]));
  };
  free(a);
}
/** free every element array and the container array */
void sp_samples_array_free(sp_sample_t** a, sp_time_t size) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    free((a[i]));
  };
  free(a);
}
/** true if array a contains element b */
uint8_t sp_times_contains(sp_time_t* a, sp_time_t size, sp_time_t b) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    if (b == a[i]) {
      return (1);
    };
  };
  return (0);
}
/** create size number of discrete random numbers corresponding to the distribution given by cudist
   without duplicates. cudist-size must be equal to a-size.
   a/out should not be the same pointer. out is managed by the caller.
   size is the requested size of generated output values and should be smaller than a-size.
   size must not be greater than the maximum possible count of unique discrete random values
   (non-null values in the probability distribution) */
void sp_times_random_discrete_unique(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size, sp_time_t size, sp_time_t* out) {
  status_declare;
  sp_time_t i;
  sp_time_t a;
  sp_time_t remaining;
  remaining = sp_min(size, cudist_size);
  while (remaining) {
    a = sp_time_random_discrete(state, cudist, cudist_size);
    if (sp_times_contains(out, (size - remaining), a)) {
      continue;
    };
    out[(size - remaining)] = a;
    remaining -= 1;
  };
}
/** out-size must be at least digits * size or base ** digits.
   starts from out + 0. first generated element will be in out + digits.
   out size will contain the sequences appended */
void sp_times_sequences(sp_time_t base, sp_time_t digits, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  for (i = digits; (i < (digits * size)); i += digits) {
    memcpy((out + i), (out + (i - digits)), (digits * sizeof(sp_time_t)));
    sp_times_sequence_increment((out + i), digits, base);
  };
}
/** write into out values from start (inclusively) to end (exclusively) */
void sp_times_range(sp_time_t start, sp_time_t end, sp_time_t* out) {
  sp_time_t i;
  for (i = start; (i < end); i += 1) {
    out[(i - start)] = i;
  };
}
/** round to the next integer multiple of base  */
sp_time_t sp_time_round_to_multiple(sp_time_t a, sp_time_t base) { return (((0 == a) ? base : sp_cheap_round_positive(((a / ((sp_sample_t)(base))) * base)))); }
/** writes the unique elements of a to out.
   out is lend by the owner. out size should be equal to a-size.
   out-size will be set to the number of unique elements */
status_t sp_times_deduplicate(sp_time_t* a, sp_time_t size, sp_time_t* out, sp_time_t* out_size) {
  status_declare;
  sp_time_set_t unique;
  sp_time_t i;
  sp_time_t unique_count;
  unique.size = 0;
  if (sp_time_set_new(size, (&unique))) {
    sp_memory_error;
  };
  unique_count = 0;
  for (i = 0; (i < size); i += 1) {
    if (!sp_time_set_get(unique, (a[i]))) {
      if (!sp_time_set_add(unique, (a[i]))) {
        sp_memory_error;
      };
      out[unique_count] = a[i];
      unique_count += 1;
    };
  };
  *out_size = unique_count;
exit:
  if (unique.size) {
    sp_time_set_free(unique);
  };
  status_return;
}
void sp_times_counted_sequences_sort_swap(void* a, ssize_t b, ssize_t c) {
  sp_times_counted_sequences_t d;
  d = ((sp_times_counted_sequences_t*)(a))[b];
  ((sp_times_counted_sequences_t*)(a))[b] = ((sp_times_counted_sequences_t*)(a))[c];
  ((sp_times_counted_sequences_t*)(a))[c] = d;
}
uint8_t sp_times_counted_sequences_sort_less(void* a, ssize_t b, ssize_t c) { return (((((sp_times_counted_sequences_t*)(a))[b]).count < (((sp_times_counted_sequences_t*)(a))[c]).count)); }
uint8_t sp_times_counted_sequences_sort_greater(void* a, ssize_t b, ssize_t c) { return (((((sp_times_counted_sequences_t*)(a))[b]).count > (((sp_times_counted_sequences_t*)(a))[c]).count)); }
/** associate in hash table $out sub-sequences of $width with their count in $a.
   memory for $out is lend and should be allocated with sp_sequence_hashtable_new(size - (width - 1), &out) */
void sp_times_counted_sequences_hash(sp_time_t* a, sp_time_t size, sp_time_t width, sp_sequence_hashtable_t out) {
  sp_time_t i;
  sp_sequence_set_key_t key;
  sp_time_t* value;
  key.size = width;
  for (i = 0; (i < (size - (width - 1))); i += 1) {
    key.data = ((uint8_t*)((i + a)));
    value = sp_sequence_hashtable_get(out, key);
    // full-hashtable-error is ignored
    if (value) {
      *value += 1;
    } else {
      sp_sequence_hashtable_set(out, key, 1);
    };
  };
}
/** extract counts from a counted-sequences-hash and return as an array of structs */
void sp_times_counted_sequences(sp_sequence_hashtable_t known, sp_time_t limit, sp_times_counted_sequences_t* out, sp_time_t* out_size) {
  sp_time_t i;
  sp_time_t count;
  count = 0;
  for (i = 0; (i < known.size); i += 1) {
    if ((known.flags)[i] && (limit < (known.values)[i])) {
      (out[count]).count = (known.values)[i];
      (out[count]).sequence = ((sp_time_t*)(((known.keys)[i]).data));
      count += 1;
    };
  };
  *out_size = count;
}
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
/** insert unset elements before index */
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
void sp_times_subdivide(sp_time_t* a, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out) {
  sp_time_t i;
  sp_time_t interval;
  sp_time_t value;
  sp_time_t previous;
  sp_times_insert_space(a, size, (index + 1), count, out);
  interval = (a[(index + 1)] - a[index]);
  value = (interval / (count + 1));
  for (i = (index + 1); (i < (index + (count + 1))); i += 1) {
    out[i] = (out[(i - 1)] + value);
  };
}
/** interpolate values between $a and $b with interpolation distance 0..1 */
void sp_times_blend(sp_time_t* a, sp_time_t* b, sp_sample_t fraction, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    out[i] = sp_cheap_round_positive((sp_time_interpolate_linear((a[i]), (b[i]), fraction)));
  };
}
/** interpolate values between $a and $b with interpolation distance 0..1 from $coefficients */
void sp_times_mask(sp_time_t* a, sp_time_t* b, sp_sample_t* coefficients, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    out[i] = sp_cheap_round_positive((sp_time_interpolate_linear((a[i]), (b[i]), (coefficients[i]))));
  };
}
/** interpolate values between $a and $b with interpolation distance 0..1 */
void sp_samples_blend(sp_sample_t* a, sp_sample_t* b, sp_sample_t fraction, sp_time_t size, sp_sample_t* out) {
  sp_time_t i;
  for (i = 0; (i < size); i += 1) {
    out[i] = sp_sample_interpolate_linear((a[i]), (b[i]), fraction);
  };
}
void sp_times_limit(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out) {
  sp_time_t i;
  "set all values greater than n in array to n";
  for (i = 0; (i < size); i += 1) {
    out[i] = ((n < a[i]) ? n : a[i]);
  };
}
void sp_samples_limit_abs(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out) {
  sp_time_t i;
  sp_sample_t v;
  "set all values greater than n in array to n";
  for (i = 0; (i < size); i += 1) {
    v = a[i];
    if (0 > v) {
      out[i] = (((-1 * n) > v) ? (-1 * n) : v);
    } else {
      out[i] = ((n < v) ? n : v);
    };
  };
}
