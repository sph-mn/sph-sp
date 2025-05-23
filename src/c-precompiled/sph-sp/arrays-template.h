
#define arrays_template_h(value_t, value_type_name, type_name) \
  void sp_##value_type_name##_sort_swap(void* a, ssize_t b, ssize_t c); \
  uint8_t sp_##value_type_name##_sort_less(void* a, ssize_t b, ssize_t c); \
  value_t sp_##value_type_name##_round_to_multiple(value_t a, value_t base); \
  value_t sp_##type_name##_min(value_t* in, sp_size_t count); \
  value_t sp_##type_name##_max(value_t* in, sp_size_t count); \
  value_t sp_##type_name##_absolute_max(value_t* in, sp_size_t count); \
  void sp_##type_name##_reverse(value_t* in, sp_size_t count, value_t* out); \
  sp_bool_t sp_##type_name##_equal(value_t* in, sp_size_t count, value_t value); \
  void sp_##type_name##_square(value_t* in, sp_size_t count); \
  void sp_##type_name##_add(value_t* in_out, sp_size_t count, value_t value); \
  void sp_##type_name##_multiply(value_t* in_out, sp_size_t count, value_t value); \
  void sp_##type_name##_divide(value_t* in_out, sp_size_t count, value_t value); \
  void sp_##type_name##_set(value_t* in_out, sp_size_t count, value_t value); \
  void sp_##type_name##_subtract(value_t* in_out, sp_size_t count, value_t value); \
  status_t sp_##type_name##_new(sp_size_t count, value_t** out); \
  void sp_##type_name##_copy(value_t* in, sp_size_t count, value_t* out); \
  void sp_##type_name##_cusum(value_t* in, value_t count, value_t* out); \
  void sp_##type_name##_swap(sp_time_t* in_out, sp_ssize_t index_1, sp_ssize_t index_2); \
  void sp_##type_name##_shuffle(value_t* in, sp_size_t count); \
  void sp_##type_name##_array_free(value_t** in, sp_size_t count); \
  void sp_##type_name##_additions(value_t start, value_t summand, value_t count, value_t* out); \
  status_t sp_##type_name##_duplicate(value_t* a, sp_size_t count, value_t** out); \
  void sp_##type_name##_range(value_t* in, sp_size_t start, sp_size_t end, value_t* out); \
  void sp_##type_name##_and_##type_name(value_t* a, value_t* b, sp_size_t count, value_t limit, value_t* out); \
  void sp_##type_name##_or_##type_name(value_t* a, value_t* b, sp_size_t count, value_t limit, value_t* out); \
  void sp_##type_name##_xor_##type_name(value_t* a, value_t* b, sp_size_t count, value_t limit, value_t* out); \
  void sp_##type_name##_multiply_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_divide_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_add_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_subtract_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_set_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_set_##type_name##_left(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_harmonic(value_t base, sp_time_t count, value_t* out); \
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
