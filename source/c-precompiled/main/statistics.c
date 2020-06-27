/* calculate statistics for arrays, with the statistics to calculate being selected by an array of identifiers.
   deviation: standard deviation
   complexity: subsequence width with the highest proportion of equal-size unique subsequences */
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
#define define_sp_stat(name, f_array, value_t) \
  /** write to out the statistics requested with sp-stat-type-t indices in stats. \
       out size is expected to be at least sp-stat-types-count */ \
  status_t name(value_t* a, sp_time_t a_size, sp_stat_type_t* stats, sp_time_t size, sp_sample_t* out) { \
    sp_time_t i; \
    status_declare; \
    for (i = 0; (i < size); i += 1) { \
      status_i_require(((f_array[stats[i]])(a, a_size, (out + stats[i])))); \
    }; \
  exit: \
    status_return; \
  }
/* sp-stat-exponential
sp-stat-harmonicity */
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
  uint8_t sp_stat_times_complexity(sp_time_t* a, sp_time_t size, sp_sample_t* out) {
  sequence_set_t known;
  sequence_set_key_t key;
  sp_sample_t max_ratio;
  sp_time_t max_ratio_width;
  sequence_set_key_t* value;
  sp_time_t count;
  sp_time_t i;
  sp_sample_t ratio;
  if (sequence_set_new(size, (&known))) {
    return (1);
  };
  for (key.size = 0; (key.size < (size + 1)); key.size += 1) {
    count = 0;
    for (i = 0; (i < (size - key.size)); i += 1) {
      key.data = ((uint8_t*)((i + a)));
      value = sequence_set_get(known, key);
      if (!value) {
        count += 1;
        if (!sequence_set_add(known, key)) {
          sequence_set_free(known);
          return (1);
        };
      };
    };
    ratio = (count / ((sp_sample_t)((size - key.size))));
    if (ratio >= max_ratio) {
      max_ratio = ratio;
      max_ratio_width = key.size;
    };
    sequence_set_clear(known);
  };
  out[0] = max_ratio;
  out[1] = max_ratio_width;
  sequence_set_free(known);
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
  /** make all values positive then scale by multiplication so that the largest value is max
   then round to integer */
  void sp_samples_scale_to_times(sp_sample_t* a, sp_time_t size, sp_time_t max, sp_time_t* out) {
  sp_time_t i;
  sp_sample_t range[3];
  sp_sample_t addition;
  /* returns range, min, max */
  sp_stat_samples_range(a, size, range);
  addition = ((0 > range[1]) ? fabs((range[1])) : 0);
  for (i = 0; (i < size); i += 1) {
    out[i] = sp_cheap_round_positive(((a[i] + addition) * (max / range[0])));
  };
}
uint8_t sp_stat_samples_complexity(sp_sample_t* a, sp_time_t size, sp_sample_t* out) {
  sp_time_t* b;
  status_declare;
  status_require((sp_times_new(size, (&b))));
  sp_samples_scale_to_times(a, size, 1000, b);
  sp_stat_times_complexity(b, size, out);
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
  /* f-array maps sp-stat-type-t indices to the functions that calculate the corresponding values */
  define_sp_stat(sp_stat_times, sp_stat_times_f_array, sp_time_t)
    define_sp_stat(sp_stat_samples, sp_stat_samples_f_array, sp_sample_t)
