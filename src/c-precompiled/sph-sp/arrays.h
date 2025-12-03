
#define arrays_template_h(value_t, value_type_name, type_name) \
  void sp_##value_type_name##_sort_swap(void* a, ssize_t b, ssize_t c); \
  uint8_t sp_##value_type_name##_sort_less(void* a, ssize_t b, ssize_t c); \
  value_t sp_##value_type_name##_round_to_multiple(value_t a, value_t base); \
  value_t sp_##type_name##_min(value_t* in, sp_size_t count); \
  value_t sp_##type_name##_max(value_t* in, sp_size_t count); \
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
  status_t sp_##type_name##_duplicate(value_t* a, sp_size_t count, value_t** out); \
  void sp_##type_name##_and_##type_name(value_t* a, value_t* b, sp_size_t count, value_t limit, value_t* out); \
  void sp_##type_name##_or_##type_name(value_t* a, value_t* b, sp_size_t count, value_t limit, value_t* out); \
  void sp_##type_name##_xor_##type_name(value_t* a, value_t* b, sp_size_t count, value_t limit, value_t* out); \
  void sp_##type_name##_multiply_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_divide_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_add_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_subtract_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_set_##type_name(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_set_##type_name##_left(value_t* in_out, sp_size_t count, value_t* in); \
  void sp_##type_name##_sort_ascending(value_t* a, sp_size_t count)

#define sp_samples_zero(a, size) memset(a, 0, (size * sizeof(sp_sample_t)))
#define sp_times_zero(a, size) memset(a, 0, (size * sizeof(sp_time_t)))
#define sp_time_interpolate_linear(a, b, t) sp_cheap_round_positive((((1 - ((sp_sample_t)(t))) * ((sp_sample_t)(a))) + (t * ((sp_sample_t)(b)))))
#define sp_sample_interpolate_linear(a, b, t) (((1 - t) * a) + (t * b))
#define sp_define_samples(id, value) sp_sample_t* id = value
#define sp_define_times(id, value) sp_time_t* id = value
#define sp_define_samples_new_srq(id, count) \
  sp_sample_t* id; \
  status_require((sp_samples_new(count, (&id))))
#define sp_define_times_srq(id, count) \
  sp_time_t* id; \
  status_require((sp_times_new(count, (&id))))
void sp_shuffle(void (*swap)(void*, sp_size_t, sp_size_t), void* in, sp_size_t count);
uint64_t sp_u64_from_array(uint8_t* a, sp_time_t count);
/* times */
#include <sph-sp/primes.h>
arrays_template_h(sp_time_t, time, times);
sp_time_t sp_time_sum(sp_time_t* in, sp_time_t size);
sp_time_t sp_times_sum(sp_time_t* a, sp_time_t size);
void sp_times_display(sp_time_t* in, sp_size_t count);
status_t sp_times_permutations(sp_time_t size, sp_time_t* set, sp_time_t set_size, sp_time_t*** out, sp_time_t* out_size);
void sp_times_sequence_increment(sp_time_t* in, sp_size_t size, sp_size_t set_size);
status_t sp_times_compositions(sp_time_t sum, sp_time_t*** out, sp_time_t* out_size, sp_time_t** out_sizes);
void sp_times_select(sp_time_t* in, sp_time_t* indices, sp_time_t count, sp_time_t* out);
void sp_times_bits_to_times(sp_time_t* a, sp_time_t size, sp_time_t* out);
void sp_times_gt_indices(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out, sp_time_t* out_size);
void sp_times_select_random(sp_time_t* a, sp_time_t size, sp_time_t* out, sp_time_t* out_size);
status_t sp_times_constant(sp_size_t count, sp_time_t value, sp_time_t** out);
status_t sp_times_scale(sp_time_t* in, sp_size_t count, sp_time_t factor, sp_time_t* out);
void sp_times_shuffle_swap(void* a, sp_size_t i1, sp_size_t i2);
void sp_times_scale_sum(sp_time_t* in, sp_size_t count, sp_time_t sum, sp_time_t* out);
uint8_t sp_times_contains(sp_time_t* in, sp_size_t count, sp_time_t value);
void sp_times_sequences(sp_time_t base, sp_time_t digits, sp_time_t size, sp_time_t* out);
void sp_times_blend(sp_time_t* a, sp_time_t* b, sp_sample_t fraction, sp_time_t size, sp_time_t* out);
void sp_times_mask(sp_time_t* a, sp_time_t* b, sp_sample_t* coefficients, sp_time_t size, sp_time_t* out);
void sp_times_extract_in_range(sp_time_t* a, sp_time_t size, sp_time_t min, sp_time_t max, sp_time_t* out, sp_time_t* out_size);
void sp_times_make_seamless_right(sp_time_t* a, sp_time_t a_count, sp_time_t* b, sp_time_t b_count, sp_time_t* out);
void sp_times_make_seamless_left(sp_time_t* a, sp_time_t a_count, sp_time_t* b, sp_time_t b_count, sp_time_t* out);
void sp_times_limit(sp_time_t* a, sp_time_t count, sp_time_t n, sp_time_t* out);
void sp_times_scale_y(sp_time_t* in, sp_size_t count, sp_time_t target_y, sp_time_t* out);
void sp_times_remove(sp_time_t* in, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out);
void sp_times_insert_space(sp_time_t* in, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out);
void sp_times_subdivide_difference(sp_time_t* a, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out);
void sp_times_to_samples(sp_time_t* in, sp_size_t count, sp_sample_t* out);
status_t sp_times_to_samples_replace(sp_time_t* in, sp_size_t count, sp_sample_t** out);
sp_time_t sp_times_absolute_max(sp_time_t* in, sp_size_t count);
/* samples */
arrays_template_h(sp_sample_t, sample, samples);
void sp_samples_display(sp_sample_t* in, sp_size_t count);
void sp_samples_to_times(sp_sample_t* in, sp_size_t count, sp_time_t* out);
status_t sp_samples_to_times_replace(sp_sample_t* in, sp_size_t count, sp_time_t** out);
void sp_samples_to_units(sp_sample_t* in_out, sp_size_t count);
void sp_samples_set_gain(sp_sample_t* in_out, sp_size_t count, sp_sample_t amp);
void sp_samples_set_gain(sp_sample_t* in_out, sp_size_t count, sp_sample_t amp);
void sp_samples_set_unity_gain(sp_sample_t* in_out, sp_sample_t* reference, sp_size_t count);
void sp_samples_divisions(sp_sample_t start, sp_sample_t divisor, sp_time_t count, sp_sample_t* out);
void sp_samples_scale_y(sp_sample_t* in, sp_time_t count, sp_sample_t target_y);
void sp_samples_scale_sum(sp_sample_t* in, sp_size_t count, sp_sample_t target_y, sp_sample_t* out);
void sp_samples_blend(sp_sample_t* a, sp_sample_t* b, sp_sample_t fraction, sp_time_t size, sp_sample_t* out);
void sp_samples_limit_abs(sp_sample_t* in, sp_time_t count, sp_sample_t limit, sp_sample_t* out);
sp_sample_t sp_samples_absolute_max(sp_sample_t* in, sp_size_t count);