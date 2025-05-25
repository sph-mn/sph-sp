
#define distributions_template(value_t, value_type_name, type_name, subtract, abs) \
  /** write count cumulative additions with summand from start to out. \
         with summand, only the nth additions are written. \
         use case: generating harmonic frequency values */ \
  void sp_##type_name##_additions(value_t start, value_t summand, value_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1, start += summand) { \
      out[i] = start; \
    }; \
  } \
\
  /** a(n) = base – k·n */ \
  void sp_##type_name##_linear_decreasing(value_t base, value_t k, value_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base - (k * i)); \
    }; \
  } \
  void sp_##type_name##_linear_series(value_t base, value_t offset, value_t spacing, sp_time_t count, value_t* out) { \
    for (sp_size_t i = 0; (i < count); i += 1) { \
      out[i] = (base * (offset + (spacing * i))); \
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
distributions_template(sp_time_t, time, times, sp_inline_no_underflow_subtract, sp_inline_abs)
  distributions_template(sp_sample_t, sample, samples, sp_subtract, fabs) void sp_times_geometric(sp_time_t base, sp_time_t ratio, sp_time_t count, sp_time_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    sp_time_t r_pow = 1;
    for (sp_size_t j = 0; (j < i); j += 1) {
      r_pow *= ratio;
    };
    out[i] = (base * r_pow);
  };
}
void sp_times_logarithmic(sp_time_t base, sp_sample_t scale, sp_time_t count, sp_time_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    sp_sample_t v = (base * log1p((scale * (i + 1))));
    out[i] = llround(v);
  };
}
void sp_samples_geometric(sp_sample_t base, sp_sample_t ratio, sp_time_t count, sp_sample_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = (base * pow(ratio, i));
  };
}
void sp_samples_logarithmic(sp_sample_t base, sp_sample_t scale, sp_time_t count, sp_sample_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = (base * log1p((scale * (i + 1))));
  };
}
void sp_samples_beta_distribution(sp_sample_t base, sp_sample_t alpha, sp_sample_t beta_param, sp_time_t count, sp_sample_t* out) {
  sp_sample_t max_val = 0;
  for (sp_size_t i = 0; (i < count); i += 1) {
    sp_sample_t x = (i / (count - 1));
    sp_sample_t val = (pow(x, (alpha - 1)) * pow((1 - x), (beta_param - 1)));
    out[i] = val;
    if (val > max_val) {
      max_val = val;
    };
  };
  sp_sample_t scale = (base / max_val);
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = (out[i] * scale);
  };
}
void sp_samples_binary_mask(sp_sample_t base, uint8_t* pattern, sp_time_t pattern_len, sp_time_t count, sp_sample_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    sp_sample_t mask_val = (pattern[(i % pattern_len)] ? base : 0);
    out[i] = mask_val;
  };
}
void sp_samples_random_cluster(sp_sample_t base, sp_time_t cluster_size, sp_time_t count, sp_sample_t* out) {
  for (sp_size_t i = 0; (i < count); i += 1) {
    out[i] = 0;
  };
  for (sp_size_t k = 0; (k < cluster_size); k += 1) {
    out[(rand() % count)] = base;
  };
}
void sp_samples_segment_steps(sp_sample_t base, sp_sample_t* levels, sp_time_t segments, sp_time_t count, sp_sample_t* out) {
  sp_time_t segment_len = (count / segments);
  sp_time_t remainder = (count % segments);
  sp_time_t idx = 0;
  for (sp_size_t s = 0; (s < segments); s += 1) {
    sp_time_t len = (segment_len + ((s < remainder) ? 1 : 0));
    for (sp_size_t j = 0; (j < len); j += 1) {
      out[idx] = (base * levels[s]);
      idx = (idx + 1);
    };
  };
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
sp_sample_t sp_sample_random_discrete_bounded(sp_time_t* cudist, sp_time_t cudist_size, sp_sample_t range) { return ((range * (sp_time_random_discrete(cudist, cudist_size) / ((sp_sample_t)(cudist_size))))); }
