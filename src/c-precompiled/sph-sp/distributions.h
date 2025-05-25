
#define distributions_template_h(value_t, value_type_name, type_name) \
  void sp_##type_name##_range(value_t* in, sp_size_t start, sp_size_t end, value_t* out); \
  void sp_##type_name##_additions(value_t start, value_t summand, value_t count, value_t* out); \
  void sp_##type_name##_multiplications(value_t base, sp_time_t count, value_t* out); \
  void sp_##type_name##_odd_harmonics(value_t base, sp_time_t count, value_t* out); \
  void sp_##type_name##_even_harmonics(value_t base, sp_time_t count, value_t* out); \
  void sp_##type_name##_nth_harmonics(value_t base, value_t k, sp_time_t count, value_t* out); \
  void sp_##type_name##_cumulative(value_t base, value_t* deltas, sp_time_t count, value_t* out); \
  void sp_##type_name##_decumulative(value_t base, value_t* deltas, sp_time_t count, value_t* out); \
  void sp_##type_name##_prime_indexed(value_t base, sp_time_t count, value_t* out); \
  void sp_##type_name##_modular_series(value_t base, sp_time_t mod, sp_sample_t delta, sp_time_t count, value_t* out); \
  void sp_##type_name##_fixed_sets(value_t base, value_t* ratios, value_t len, value_t* out); \
  void sp_##type_name##_clustered(value_t center, value_t spread, value_t count, value_t* out); \
  void sp_##type_name##_linear(value_t base, value_t k, value_t count, value_t* out); \
  void sp_##type_name##_exponential(value_t base, value_t k, value_t count, value_t* out); \
  void sp_##type_name##_gaussian(value_t base, value_t centre, value_t width, value_t count, value_t* out); \
  void sp_##type_name##_power(value_t base, value_t p, value_t count, value_t* out); \
  void sp_##type_name##_bessel(value_t base, value_t count, value_t* out); \
  void sp_##type_name##_logistic(value_t base, value_t k, value_t count, value_t* out)
distributions_template_h(sp_time_t, time, times);
distributions_template_h(sp_sample_t, sample, samples);
sp_sample_t sp_sample_random_discrete_bounded(sp_time_t* cudist, sp_time_t cudist_size, sp_sample_t range);
void sp_samples_geometric(sp_sample_t base, sp_sample_t ratio, sp_time_t count, sp_sample_t* out);
void sp_samples_logarithmic(sp_sample_t base, sp_sample_t scale, sp_time_t count, sp_sample_t* out);
void sp_times_geometric(sp_time_t base, sp_time_t ratio, sp_time_t count, sp_time_t* out);
void sp_times_logarithmic(sp_time_t base, sp_sample_t scale, sp_time_t count, sp_time_t* out);
void sp_times_random_discrete_unique(sp_time_t* cudist, sp_time_t cudist_size, sp_time_t size, sp_time_t* out);
status_t sp_times_random_binary(sp_time_t size, sp_time_t* out);
sp_time_t sp_time_random_discrete(sp_time_t* cudist, sp_time_t cudist_size);
sp_time_t sp_time_random_discrete_bounded(sp_time_t* cudist, sp_time_t cudist_size, sp_time_t range);
void sp_times_random_discrete(sp_time_t* cudist, sp_time_t cudist_size, sp_time_t count, sp_time_t* out);