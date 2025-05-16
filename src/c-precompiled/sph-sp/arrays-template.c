
/* functions that work the same for sp-sample-t and sp-time-t */
#define arrays_template(value_t, value_type_name, type_name, subtract, abs) \
  void sp_##value_type_name##_sort_swap(void* a, ssize_t b, ssize_t c) { \
    value_t d; \
    d = ((value_t*)(a))[b]; \
    ((value_t*)(a))[b] = ((value_t*)(a))[c]; \
    ((value_t*)(a))[c] = d; \
  } \
  uint8_t sp_##value_type_name##_sort_less(void* a, ssize_t b, ssize_t c) { return ((((value_t*)(a))[b] < ((value_t*)(a))[c])); } \
\
  /** round to the next integer multiple of base  */ \
  value_t sp_##value_type_name##_round_to_multiple(value_t a, value_t base) { return (((0 == a) ? base : sp_cheap_round_positive(((a / ((sp_sample_t)(base))) * base)))); } \
\
  /** count must be greater than zero */ \
  value_t sp_##type_name##_min(value_t* in, sp_size_t count) { \
    value_t out = in[0]; \
    for (sp_size_t i = 1; (i < count); i += 1) { \
      if (in[i] < out) { \
        out = in[i]; \
      }; \
    }; \
    return (out); \
  } \
\
  /** count must be greater than zero */ \
  value_t sp_##type_name##_max(value_t* in, sp_size_t count) { \
    value_t out = in[0]; \
    for (sp_size_t i = 1; (i < count); i += 1) { \
      if (in[i] > out) { \
        out = in[i]; \
      }; \
    }; \
    return (out); \
  } \
\
  /** get the maximum value in samples array, disregarding sign */ \
  value_t sp_##type_name##_absolute_max(value_t* in, sp_size_t count) { \
    sp_time_t temp; \
    sp_time_t max; \
    max = 0; \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      temp = sp_inline_abs((in[i])); \
      if (temp > max) { \
        max = temp; \
      }; \
    }; \
    return (max); \
  } \
  void sp_##type_name##_reverse(value_t* in, sp_size_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[(count - i)] = in[i]; \
    }; \
  } \
  sp_bool_t sp_##type_name##_equal(value_t* in, sp_size_t count, value_t value) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      if (!(value == in[i])) { \
        return (0); \
      }; \
    }; \
    return (1); \
  } \
  void sp_##type_name##_square(value_t* in, sp_size_t count) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in[i] *= in[i]; \
    }; \
  } \
  void sp_##type_name##_add(value_t* in_out, sp_size_t count, value_t value) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[i] += value; \
    }; \
  } \
  void sp_##type_name##_multiply(value_t* in_out, sp_size_t count, value_t value) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[i] *= value; \
    }; \
  } \
  void sp_##type_name##_divide(value_t* in_out, sp_size_t count, value_t value) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[i] /= value; \
    }; \
  } \
  void sp_##type_name##_set(value_t* in_out, sp_size_t count, value_t value) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[i] = value; \
    }; \
  } \
  void sp_##type_name##_subtract(value_t* in_out, sp_size_t count, value_t value) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[i] = subtract((in_out[i]), value); \
    }; \
  } \
  status_t sp_##type_name##_new(sp_size_t count, value_t** out) { return ((sph_helper_calloc((count * sizeof(value_t)), out))); } \
  void sp_##type_name##_copy(value_t* in, sp_size_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = in[i]; \
    }; \
  } \
\
  /** calculate cumulative sums from the given numbers. \
         (a b c ...) -> (a (+ a b) (+ a b c) ...) */ \
  void sp_##type_name##_cusum(value_t* in, value_t count, value_t* out) { \
    value_t sum = in[0]; \
    out[0] = sum; \
    for (sp_time_t i = 1; (i < count); i += 1) { \
      sum += in[i]; \
      out[i] = sum; \
    }; \
  } \
  void sp_##type_name##_swap(sp_time_t* in_out, sp_ssize_t index_1, sp_ssize_t index_2) { \
    sp_time_t temp = in_out[index_1]; \
    in_out[index_1] = in_out[index_2]; \
    in_out[index_2] = temp; \
  } \
  void sp_##type_name##_shuffle(value_t* in, sp_size_t count) { sp_shuffle(((void (*)(void*, sp_size_t, sp_size_t))(sp_##type_name##_swap)), in, count); } \
\
  /** free every element array and the container array */ \
  void sp_##type_name##_array_free(value_t** in, sp_size_t count) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      free((in[i])); \
    }; \
    free(in); \
  } \
\
  /** write count cumulative additions with summand from start to out. \
         with summand, only the nth additions are written. \
         use case: generating harmonic frequency values */ \
  void sp_##type_name##_additions(value_t start, value_t summand, value_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1, start += summand) { \
      out[i] = start; \
    }; \
  } \
  status_t sp_##type_name##_duplicate(value_t* a, sp_size_t count, value_t** out) { \
    status_declare; \
    value_t* temp; \
    status_require((sp_##type_name##_new(count, (&temp)))); \
    memcpy(temp, a, (count * sizeof(value_t))); \
    *out = temp; \
  exit: \
    status_return; \
  } \
\
  /** write into out values from start (inclusively) to end (exclusively) */ \
  void sp_##type_name##_range(value_t* in, sp_size_t start, sp_size_t end, value_t* out) { \
    for (sp_size_t i = 0; (i < (end - start)); i += 1) { \
      out[i] = in[(start + i)]; \
    }; \
  } \
\
  /** if a[i] and b[i] greater than limit, take b[i] else 0 */ \
  void sp_##type_name##_and_##type_name(value_t* a, value_t* b, sp_size_t count, value_t limit, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      if ((a[i] >= limit) && (a[i] >= limit)) { \
        out[i] = b[i]; \
      } else { \
        out[i] = 0; \
      }; \
    }; \
  } \
\
  /** if a[i] < limit and b[i] > limit, take b[i], else a[i] */ \
  void sp_##type_name##_or_##type_name(value_t* a, value_t* b, sp_size_t count, value_t limit, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
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
  void sp_##type_name##_xor_##type_name(value_t* a, value_t* b, sp_size_t count, value_t limit, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
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
  } \
  void sp_##type_name##_multiply_##type_name(value_t* in_out, sp_size_t count, value_t* in) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[i] *= in[i]; \
    }; \
  } \
  void sp_##type_name##_divide_##type_name(value_t* in_out, sp_size_t count, value_t* in) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[i] /= in[i]; \
    }; \
  } \
  void sp_##type_name##_add_##type_name(value_t* in_out, sp_size_t count, value_t* in) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[i] = (in_out[i] + in[i]); \
    }; \
  } \
  void sp_##type_name##_subtract_##type_name(value_t* in_out, sp_size_t count, value_t* in) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[i] = subtract((in_out[i]), (in[i])); \
    }; \
  } \
  void sp_##type_name##_set_##type_name(value_t* in_out, sp_size_t count, value_t* in) { memcpy(in_out, in, (count * sizeof(value_t))); } \
  void sp_##type_name##_set_##type_name##_left(value_t* in_out, sp_size_t count, value_t* in) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      in_out[(-1 * i)] = in[i]; \
    }; \
  } \
\
  /** f(n) = base * (n+1) */ \
  void sp_##type_name##_harmonic(value_t base, sp_time_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base * (1 + i)); \
    }; \
  } \
\
  /** f(n) = base * (2·n+1) */ \
  void sp_##type_name##_odd_harmonics(value_t base, sp_time_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base * (1 + (2 * i))); \
    }; \
  } \
\
  /** f(n) = base * 2·(n+1) */ \
  void sp_##type_name##_even_harmonics(value_t base, sp_time_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base * 2 * (1 + i)); \
    }; \
  } \
\
  /** f(n) = base * k * (n+1) */ \
  void sp_##type_name##_nth_harmonics(value_t base, value_t k, sp_time_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base * k * (1 + i)); \
    }; \
  } \
\
  /** f(n) = f(n–1) + deltas[n] */ \
  void sp_##type_name##_cumulative(value_t base, value_t* deltas, sp_time_t count, value_t* out) { \
    value_t acc = base; \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      acc = (acc + deltas[i]); \
      out[i] = acc; \
    }; \
  } \
\
  /** f(n) = f(n–1) – deltas[n] */ \
  void sp_##type_name##_decumulative(value_t base, value_t* deltas, sp_time_t count, value_t* out) { \
    value_t acc = base; \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      acc = (acc - deltas[i]); \
      out[i] = acc; \
    }; \
  } \
\
  /** f(n) = base * pₙ, pₙ = nth prime */ \
  void sp_##type_name##_prime_indexed(value_t base, sp_time_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < sp_time_min(count, (sizeof(sp_primes) / sizeof(sp_time_t)))); i += 1) { \
      out[i] = (base * sp_primes[i]); \
    }; \
  } \
\
  /** f(n) = base + (n mod mod)·delta */ \
  void sp_##type_name##_modular_series(value_t base, sp_time_t mod, sp_sample_t delta, sp_time_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base + (fmod(i, mod) * delta)); \
    }; \
  } \
\
  /** f(n) = base * ratios[n] */ \
  void sp_##type_name##_fixed_sets(value_t base, value_t* ratios, value_t len, value_t* out) { \
    for (sp_size_t i = 0; (i < len); i += 1) { \
      out[i] = (base * ratios[i]); \
    }; \
  } \
\
  /** f(n) = center + spread·(n–(count–1)/2) */ \
  void sp_##type_name##_clustered(value_t center, value_t spread, value_t count, value_t* out) { \
    value_t half = ((count - 1) / 2); \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (center + (spread * (i - half))); \
    }; \
  } \
\
  /** a(n) = base – k·n */ \
  void sp_##type_name##_linear(value_t base, value_t k, value_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base - (k * i)); \
    }; \
  } \
\
  /** a(n) = base·e^(–k·n) */ \
  void sp_##type_name##_exponential(value_t base, value_t k, value_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base * exp((-(k * i)))); \
    }; \
  } \
\
  /** a(n) = base·e^(–((n–centre)²)/(2·width²)) */ \
  void sp_##type_name##_gaussian(value_t base, value_t centre, value_t width, value_t count, value_t* out) { \
    value_t denom = (2 * (width * width)); \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      value_t d = (i - centre); \
      out[i] = (base * exp(((-(d * d)) / denom))); \
    }; \
  } \
\
  /** a(n) = base·(n+1)^(–p) */ \
  void sp_##type_name##_power(value_t base, value_t p, value_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base * pow((1 + i), (-p))); \
    }; \
  } \
\
  /** a(n) = base·j0(n) */ \
  void sp_##type_name##_bessel(value_t base, value_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base * j0(i)); \
    }; \
  } \
\
  /** a(n) = base / (1 + e^(k·(n–mid))) */ \
  void sp_##type_name##_logistic(value_t base, value_t k, value_t count, value_t* out) { \
    value_t mid = ((count - 1) / 2); \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base / (1 + exp((k * (i - mid))))); \
    }; \
  }
