
#define define_sp_stat_range(name, value_t) \
  /** out: min, max, range */ \
  uint8_t name(value_t* a, sp_time_t size, sp_sample_t* out) { \
    sp_time_t i; \
    value_t min; \
    value_t max; \
    value_t b; \
    min = a[0]; \
    max = min; \
    for (i = 0; (i < size); i += 1) { \
      b = a[i]; \
      if (b > max) { \
        max = b; \
      } else { \
        if (b < min) { \
          min = b; \
        }; \
      }; \
    }; \
    out[0] = min; \
    out[1] = max; \
    out[2] = (max - min); \
    return (0); \
  }
#define define_sp_stat_deviation(name, stat_mean, value_t) \
  /** standard deviation */ \
  uint8_t name(value_t* a, sp_time_t size, sp_sample_t* out) { \
    sp_time_t i; \
    sp_sample_t sum; \
    sp_sample_t dev; \
    sp_sample_t mean; \
    stat_mean(a, size, (&mean)); \
    sum = 0; \
    for (i = 0; (i < size); i += 1) { \
      dev = (a[i] - mean); \
      sum = (sum + (dev * dev)); \
    }; \
    *out = sqrt((sum / size)); \
    return (0); \
  }
#define define_sp_stat_skewness(name, stat_mean, value_t) \
  /** mean((x - mean(data)) ** 3) / (mean((x - mean(data)) ** 2) ** 3/2) */ \
  uint8_t name(value_t* a, sp_time_t size, sp_sample_t* out) { \
    sp_time_t i; \
    sp_sample_t mean; \
    sp_sample_t m3; \
    sp_sample_t m2; \
    sp_sample_t b; \
    m2 = 0; \
    m3 = 0; \
    stat_mean(a, size, (&mean)); \
    for (i = 0; (i < size); i += 1) { \
      b = (a[i] - mean); \
      m2 = (m2 + (b * b)); \
      m3 = (m3 + (b * b * b)); \
    }; \
    m3 = (m3 / size); \
    m2 = (m2 / size); \
    *out = (m3 / sqrt((m2 * m2 * m2))); \
    return (0); \
  }
#define define_sp_stat_kurtosis(name, stat_mean, value_t) \
  /** mean((x - mean(data)) ** 4) / (mean((x - mean(data)) ** 2) ** 2) */ \
  uint8_t name(value_t* a, sp_time_t size, sp_sample_t* out) { \
    sp_sample_t b; \
    sp_time_t i; \
    sp_sample_t mean; \
    sp_sample_t m2; \
    sp_sample_t m4; \
    m2 = 0; \
    m4 = 0; \
    stat_mean(a, size, (&mean)); \
    for (i = 0; (i < size); i += 1) { \
      b = (a[i] - mean); \
      m2 = (m2 + (b * b)); \
      m4 = (m4 + (b * b * b * b)); \
    }; \
    m4 = (m4 / size); \
    m2 = (m2 / size); \
    *out = (m4 / (m2 * m2)); \
    return (0); \
  }
#define define_sp_stat_median(name, sort_less, sort_swap, value_t) \
  uint8_t name(value_t* a, sp_time_t size, sp_sample_t* out) { \
    value_t* temp; \
    temp = malloc((size * sizeof(value_t))); \
    if (!temp) { \
      return (1); \
    }; \
    memcpy(temp, a, (size * sizeof(value_t))); \
    quicksort(sort_less, sort_swap, temp, 0, (size - 1)); \
    *out = ((size & 1) ? temp[((size / 2) - 1)] : ((temp[(size / 2)] + temp[((size / 2) - 1)]) / 2.0)); \
    return (0); \
  }
#define define_sp_stat_inharmonicity(name, value_t) \
  /** calculate a value for total inharmonicity >= 0. \
       the value is not normalised for different lengths of input. \
       # formula \
       n1: 0..n; n2: 0..n; half_offset(x) = 0.5 >= x ? x : x - 1; \
       min(map(n1, mean(map(n2, half_offset(x(n2) / x(n1)))))) */ \
  uint8_t name(value_t* a, sp_time_t size, sp_sample_t* out) { \
    sp_time_t i; \
    sp_time_t i2; \
    sp_sample_t b; \
    sp_sample_t sum; \
    sp_sample_t min; \
    sum = 0; \
    min = size; \
    for (i = 0; (i < size); i += 1) { \
      sum = 0; \
      for (i2 = 0; (i2 < size); i2 += 1) { \
        b = (a[i2] / ((sp_sample_t)(a[i]))); \
        b = (b - sp_cheap_floor_positive(b)); \
        b = ((0.5 >= b) ? b : (1 - b)); \
        sum = (sum + b); \
      }; \
      sum = (sum / size); \
      if (sum < min) { \
        min = sum; \
      }; \
    }; \
    *out = min; \
    return (0); \
  }

/** return the maximum number of possible unique overlapping sequences of $width in an array of $size.
   $size must be equal or greater than $width */
sp_time_t sp_stat_unique_max(sp_time_t size, sp_time_t width) { return ((size - (width - 1))); }

/** return the sum of sp_stat_unique_max for all subsequences of width 1 to $size.
   $size must be greater than 0 */
sp_time_t sp_stat_unique_all_max(sp_time_t size) {
  sp_time_t result;
  sp_time_t width;
  result = 0;
  width = 1;
  while ((width <= size)) {
    result += (size - (width - 1));
    width += 1;
  };
  return (result);
}
sp_time_t sp_stat_repetition_all_max(sp_time_t size) { return ((sp_stat_unique_all_max(size) - size)); }
sp_time_t sp_stat_repetition_max(sp_time_t size, sp_time_t width) { return ((sp_stat_unique_max(size, width) - 1)); }

/* times */

/** center of mass. the distribution of mass is balanced around the center of mass, and the average of
   the weighted position coordinates of the distributed mass defines its coordinates.
   sum(n * x(n)) / sum(x(n)) */
uint8_t sp_stat_times_center(sp_time_t* a, sp_time_t size, sp_sample_t* out) {
  sp_time_t i;
  sp_time_t sum;
  sp_time_t index_sum;
  index_sum = 0;
  sum = a[0];
  for (i = 0; (i < size); i += 1) {
    sum += a[i];
    index_sum += (i * a[i]);
  };
  *out = (index_sum / ((sp_sample_t)(sum)));
  return (0);
}
define_sp_stat_range(sp_stat_times_range, sp_time_t)

  /** return in $out the number of repetitions of subsequences of widths 1 to $size.
     examples
       high: 11111 121212
       low: 12345 112212 */
  uint8_t sp_stat_times_repetition_all(sp_time_t* a, sp_time_t size, sp_sample_t* out) {
  sp_time_t count;
  sp_time_t i;
  sp_sequence_set_key_t key;
  sp_sequence_set_t known;
  sp_sequence_set_key_t* value;
  if (sp_sequence_set_new(size, (&known))) {
    return (1);
  };
  count = 0;
  for (key.size = 1; (key.size < size); key.size += 1) {
    for (i = 0; (i < (size - (key.size - 1))); i += 1) {
      key.data = ((uint8_t*)((i + a)));
      value = sp_sequence_set_get(known, key);
      if (value) {
        count += 1;
      } else {
        if (!sp_sequence_set_add(known, key)) {
          sp_sequence_set_free(known);
          return (1);
        };
      };
    };
    sp_sequence_set_clear(known);
  };
  *out = count;
  sp_sequence_set_free(known);
  return (0);
}

/** return in $out the number of repetitions of subsequences of $width */
uint8_t sp_stat_times_repetition(sp_time_t* a, sp_time_t size, sp_time_t width, sp_sample_t* out) {
  sp_time_t count;
  sp_time_t i;
  sp_sequence_set_key_t key;
  sp_sequence_set_t known;
  sp_sequence_set_key_t* value;
  if (sp_sequence_set_new((sp_stat_unique_max(size, width)), (&known))) {
    return (1);
  };
  count = 0;
  key.size = width;
  for (i = 0; (i < (size - (width - 1))); i += 1) {
    key.data = ((uint8_t*)((i + a)));
    value = sp_sequence_set_get(known, key);
    if (value) {
      count += 1;
    } else {
      if (!sp_sequence_set_add(known, key)) {
        sp_sequence_set_free(known);
        return (1);
      };
    };
  };
  *out = count;
  sp_sequence_set_free(known);
  return (0);
}
uint8_t sp_stat_times_mean(sp_time_t* a, sp_time_t size, sp_sample_t* out) {
  sp_time_t i;
  sp_time_t sum;
  sum = 0;
  for (i = 0; (i < size); i += 1) {
    sum += a[i];
  };
  *out = (sum / ((sp_sample_t)(size)));
  return (0);
}
define_sp_stat_deviation(sp_stat_times_deviation, sp_stat_times_mean, sp_time_t)
  define_sp_stat_median(sp_stat_times_median, sp_times_sort_less, sp_times_sort_swap, sp_time_t)
    define_sp_stat_skewness(sp_stat_times_skewness, sp_stat_times_mean, sp_time_t)
      define_sp_stat_kurtosis(sp_stat_times_kurtosis, sp_stat_times_mean, sp_time_t)

  /* samples */
  uint8_t sp_stat_samples_center(sp_sample_t* a, sp_time_t size, sp_sample_t* out) {
  sp_time_t i;
  sp_sample_t sum;
  sp_sample_t index_sum;
  index_sum = 0;
  sum = sp_samples_sum(a, size);
  for (i = 0; (i < size); i += 1) {
    index_sum += (i * a[i]);
  };
  *out = (index_sum / sum);
  return (0);
}
define_sp_stat_range(sp_stat_samples_range, sp_sample_t)
  define_sp_stat_inharmonicity(sp_stat_times_inharmonicity, sp_time_t)

  /** map input samples into the time range 0..max.
     makes all values positive by adding the absolute minimum
     then scales with multiplication so that the largest value is max
     then rounds to sp-time-t */
  void sp_samples_scale_to_times(sp_sample_t* a, sp_time_t size, sp_time_t max, sp_time_t* out) {
  sp_time_t i;
  sp_sample_t range[3];
  sp_sample_t addition;
  /* returns min, max, range */
  sp_stat_samples_range(a, size, range);
  addition = ((0 > range[0]) ? fabs((range[0])) : 0);
  for (i = 0; (i < size); i += 1) {
    out[i] = sp_cheap_round_positive(((a[i] + addition) * (max / range[2])));
  };
}
uint8_t sp_stat_samples_repetition_all(sp_sample_t* a, sp_time_t size, sp_sample_t* out) {
  sp_time_t* b;
  status_declare;
  status_require((sp_times_new(size, (&b))));
  sp_samples_scale_to_times(a, size, 1000, b);
  sp_stat_times_repetition_all(b, size, out);
  free(b);
exit:
  return ((status.id));
}
uint8_t sp_stat_samples_mean(sp_sample_t* a, sp_time_t size, sp_sample_t* out) {
  *out = (sp_samples_sum(a, size) / size);
  return (0);
}
define_sp_stat_deviation(sp_stat_samples_deviation, sp_stat_samples_mean, sp_sample_t)
  define_sp_stat_median(sp_stat_samples_median, sp_samples_sort_less, sp_samples_sort_swap, sp_sample_t)
    define_sp_stat_skewness(sp_stat_samples_skewness, sp_stat_samples_mean, sp_sample_t)
      define_sp_stat_kurtosis(sp_stat_samples_kurtosis, sp_stat_samples_mean, sp_sample_t)
        define_sp_stat_inharmonicity(sp_stat_samples_inharmonicity, sp_sample_t)
