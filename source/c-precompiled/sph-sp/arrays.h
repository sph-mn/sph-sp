
#include <sph-sp/arrays-template.h>

#define sp_samples_zero(a, size) memset(a, 0, (size * sizeof(sp_sample_t)))
#define sp_times_zero(a, size) memset(a, 0, (size * sizeof(sp_time_t)))
#define sp_time_interpolate_linear(a, b, t) sp_cheap_round_positive((((1 - ((sp_sample_t)(t))) * ((sp_sample_t)(a))) + (t * ((sp_sample_t)(b)))))
#define sp_sample_interpolate_linear(a, b, t) (((1 - t) * a) + (t * b))
#define sp_sequence_set_equal(a, b) ((a.size == b.size) && (((0 == a.size) && (0 == b.size)) || (0 == memcmp((a.data), (b.data), (a.size * sizeof(sp_time_t))))))

/* times */
arrays_template_h(sp_time_t, time, times);
sp_time_t sp_time_sum(sp_time_t* in, sp_time_t size);
sp_time_t sp_times_sum(sp_time_t* a, sp_time_t size);
void sp_times_display(sp_time_t* in, sp_size_t count);
void sp_times_random_discrete(sp_time_t* cudist, sp_time_t cudist_size, sp_time_t count, sp_time_t* out);
sp_time_t sp_time_random_discrete(sp_time_t* cudist, sp_time_t cudist_size);
sp_time_t sp_time_random_custom(sp_time_t* cudist, sp_time_t cudist_size, sp_time_t range);
status_t sp_times_permutations(sp_time_t size, sp_time_t* set, sp_time_t set_size, sp_time_t*** out, sp_time_t* out_size);
void sp_times_sequence_increment(sp_time_t* in, sp_size_t size, sp_size_t set_size);
status_t sp_times_compositions(sp_time_t sum, sp_time_t*** out, sp_time_t* out_size, sp_time_t** out_sizes);
void sp_times_select(sp_time_t* in, sp_time_t* indices, sp_time_t count, sp_time_t* out);
void sp_times_bits_to_times(sp_time_t* a, sp_time_t size, sp_time_t* out);
status_t sp_times_random_binary(sp_time_t size, sp_time_t* out);
void sp_times_gt_indices(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out, sp_time_t* out_size);
void sp_times_select_random(sp_time_t* a, sp_time_t size, sp_time_t* out, sp_time_t* out_size);
status_t sp_times_constant(sp_size_t count, sp_time_t value, sp_time_t** out);
status_t sp_times_scale(sp_time_t* in, sp_size_t count, sp_time_t factor, sp_time_t* out);
void sp_times_shuffle_swap(void* a, sp_size_t i1, sp_size_t i2);
void sp_times_scale_sum(sp_time_t* in, sp_size_t count, sp_time_t sum, sp_time_t* out);
void sp_times_multiplications(sp_time_t start, sp_time_t factor, sp_time_t count, sp_time_t* out);
uint8_t sp_times_contains(sp_time_t* in, sp_size_t count, sp_time_t value);
void sp_times_random_discrete_unique(sp_time_t* cudist, sp_time_t cudist_size, sp_time_t size, sp_time_t* out);
void sp_times_sequences(sp_time_t base, sp_time_t digits, sp_time_t size, sp_time_t* out);
status_t sp_times_deduplicate(sp_time_t* a, sp_time_t size, sp_time_t* out, sp_time_t* out_size);
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
/* samples */
arrays_template_h(sp_sample_t, sample, samples);
void sp_samples_display(sp_sample_t* in, sp_size_t count);
void sp_samples_to_times(sp_sample_t* in, sp_size_t count, sp_time_t* out);
void sp_samples_to_units(sp_sample_t* in_out, sp_size_t count);
void sp_samples_set_gain(sp_sample_t* in_out, sp_size_t count, sp_sample_t amp);
void sp_samples_set_gain(sp_sample_t* in_out, sp_size_t count, sp_sample_t amp);
void sp_samples_set_unity_gain(sp_sample_t* in_out, sp_sample_t* reference, sp_size_t count);
void sp_samples_divisions(sp_sample_t start, sp_sample_t divisor, sp_time_t count, sp_sample_t* out);
void sp_samples_scale_y(sp_sample_t* in, sp_time_t count, sp_sample_t target_y);
void sp_samples_scale_sum(sp_sample_t* in, sp_size_t count, sp_sample_t target_y, sp_sample_t* out);
void sp_samples_blend(sp_sample_t* a, sp_sample_t* b, sp_sample_t fraction, sp_time_t size, sp_sample_t* out);
void sp_samples_limit_abs(sp_sample_t* in, sp_time_t count, sp_sample_t limit, sp_sample_t* out);
/* extra */
typedef struct {
  sp_time_t size;
  uint8_t* data;
} sp_sequence_set_key_t;
uint64_t sp_u64_from_array(uint8_t* a, sp_time_t count);
uint64_t sp_sequence_set_hash(sp_sequence_set_key_t a, sp_time_t memory_size) { return ((sp_u64_from_array((a.data), (a.size)) % memory_size)); }
sph_hashtable_declare_type(sp_sequence_hashtable, sp_sequence_set_key_t, sp_time_t, sp_sequence_set_hash, sp_sequence_set_equal, 2);
sp_sequence_set_key_t sp_sequence_set_null = { 0, 0 };
sph_set_declare_type_nonull(sp_sequence_set, sp_sequence_set_key_t, sp_sequence_set_hash, sp_sequence_set_equal, sp_sequence_set_null, 2);
sph_set_declare_type(sp_time_set, sp_time_t, sph_set_hash_integer, sph_set_equal_integer, 0, 1, 2);
typedef struct {
  sp_time_t count;
  sp_time_t* sequence;
} sp_times_counted_sequences_t;
void sp_shuffle(void (*swap)(void*, sp_size_t, sp_size_t), void* in, sp_size_t count);
void sp_times_counted_sequences_sort_swap(void* a, sp_ssize_t b, sp_ssize_t c);
uint8_t sp_times_counted_sequences_sort_less(void* a, sp_ssize_t b, sp_ssize_t c);
uint8_t sp_times_counted_sequences_sort_greater(void* a, sp_ssize_t b, sp_ssize_t c);
void sp_times_counted_sequences_add(sp_time_t* a, sp_time_t size, sp_time_t width, sp_sequence_hashtable_t out);
sp_time_t sp_times_counted_sequences_count(sp_time_t* a, sp_time_t width, sp_sequence_hashtable_t b);
void sp_times_counted_sequences_values(sp_sequence_hashtable_t known, sp_time_t min, sp_times_counted_sequences_t* out, sp_time_t* out_size);