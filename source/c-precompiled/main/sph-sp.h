
#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif
#include <byteswap.h>
#include <inttypes.h>
#include <string.h>

/* configuration */

#ifndef sp_channel_limit
#define sp_channel_limit 2
#endif
#ifndef sp_channel_count_t
#define sp_channel_count_t uint8_t
#endif
#ifndef sp_file_format
#define sp_file_format (SF_FORMAT_WAV | SF_FORMAT_FLOAT)
#endif
#ifndef sp_float_t
#define sp_float_t double
#endif
#ifndef spline_path_time_t
#define spline_path_time_t sp_time_t
#endif
#ifndef spline_path_value_t
#define spline_path_value_t sp_sample_t
#endif
#ifndef sp_sample_rate_t
#define sp_sample_rate_t uint32_t
#endif
#ifndef sp_samples_sum
#define sp_samples_sum f64_sum
#endif
#ifndef sp_sample_t
#define sp_sample_t double
#endif
#ifndef sp_sf_read
#define sp_sf_read sf_readf_double
#endif
#ifndef sp_sf_write
#define sp_sf_write sf_writef_double
#endif
#ifndef sp_time_t
#define sp_time_t uint32_t
#endif
#ifndef sp_time_half_t
#define sp_time_half_t uint16_t
#endif
#ifndef sp_times_random
#define sp_times_random sph_random_u32_array
#endif
#ifndef sp_times_random_bounded
#define sp_times_random_bounded sph_random_u32_bounded_array
#endif
#ifndef sp_samples_random
#define sp_samples_random sph_random_f64_array
#endif
#ifndef sp_samples_random_bounded
#define sp_samples_random_bounded sph_random_f64_bounded_array
#endif
#ifndef sp_time_random
#define sp_time_random sph_random_u32
#endif
#ifndef sp_time_random_bounded
#define sp_time_random_bounded sph_random_u32_bounded
#endif
#ifndef sp_sample_random
#define sp_sample_random sph_random_f64
#endif
#ifndef sp_sample_nearly_equal
#define sp_sample_nearly_equal f64_nearly_equal
#endif
#ifndef sp_sample_array_nearly_equal
#define sp_sample_array_nearly_equal f64_array_nearly_equal
#endif
#ifndef sp_default_random_seed
#define sp_default_random_seed 1557083953
#endif
#include <sph/status.c>
#include <sph/spline-path.h>
#include <sph/random.c>
#include <sph/array3.c>
#include <sph/array4.c>
#include <sph/float.c>
#include <sph/set.c>
#include <sph/hashtable.c>

/* main */

#define boolean uint8_t
#define f64 double
#define sp_s_group_libc "libc"
#define sp_s_group_sndfile "sndfile"
#define sp_s_group_sp "sp"
#define sp_s_group_sph "sph"
#define sp_s_id_undefined 1
#define sp_s_id_file_channel_mismatch 2
#define sp_s_id_file_encoding 3
#define sp_s_id_file_header 4
#define sp_s_id_file_incompatible 5
#define sp_s_id_file_incomplete 6
#define sp_s_id_eof 7
#define sp_s_id_input_type 8
#define sp_s_id_memory 9
#define sp_s_id_invalid_argument 10
#define sp_s_id_not_implemented 11
#define sp_s_id_file_closed 11
#define sp_s_id_file_position 12
#define sp_s_id_file_type 13
#define sp_file_bit_input 1
#define sp_file_bit_output 2
#define sp_file_bit_position 4
#define sp_file_mode_read 1
#define sp_file_mode_write 2
#define sp_file_mode_read_write 3
#define sp_random_state_t sph_random_state_t
#define sp_file_declare(a) \
  sp_file_t a; \
  a.flags = 0
#define sp_block_declare(a) \
  sp_block_t a; \
  a.size = 0
#define sp_cheap_round_positive(a) ((sp_time_t)((0.5 + a)))
#define sp_cheap_floor_positive(a) ((sp_time_t)(a))
#define sp_cheap_ceiling_positive(a) (((sp_time_t)(a)) + (((sp_time_t)(a)) < a))
#define sp_max(a, b) ((a > b) ? a : b)
#define sp_min(a, b) ((a < b) ? a : b)

/** subtract the smaller number from the greater number,
     regardless of if the smallest is the first or the second argument */
#define sp_absolute_difference(a, b) ((a > b) ? (a - b) : (b - a))
#define sp_abs(a) ((0 > a) ? (-1 * a) : a)

/** subtract b from a but return 0 for negative results */
#define sp_no_underflow_subtract(a, b) ((a > b) ? (a - b) : 0)

/** divide a by b (a / b) but return 0 if b is zero */
#define sp_no_zero_divide(a, b) ((0 == b) ? 0 : (a / b))
#define sp_status_set(id) \
  status.id = sp_s_id_memory; \
  status.group = sp_s_group_sp
#define sp_malloc_type(count, type, pointer_address) sph_helper_malloc((count * sizeof(type)), pointer_address)
#define sp_calloc_type(count, type, pointer_address) sph_helper_calloc((count * sizeof(type)), pointer_address)
#define sp_realloc_type(count, type, pointer_address) sph_helper_realloc((count * sizeof(type)), pointer_address)
typedef struct {
  sp_channel_count_t channels;
  sp_time_t size;
  sp_sample_t* samples[sp_channel_limit];
} sp_block_t;
typedef struct {
  uint8_t flags;
  sp_sample_rate_t sample_rate;
  sp_channel_count_t channel_count;
  void* data;
} sp_file_t;
uint32_t sp_cpu_count;
sp_random_state_t sp_default_random_state;
sp_time_t sp_rate;
sp_channel_count_t sp_channels;
sp_sample_t* sp_sine_table;
sp_sample_t* sp_sine_table_lfo;
sp_time_t sp_sine_lfo_factor;
sp_random_state_t sp_random_state_new(sp_time_t seed);
void sp_block_zero(sp_block_t a);
void sp_block_copy(sp_block_t a, sp_block_t b);
status_t sp_file_read(sp_file_t* file, sp_time_t sample_count, sp_sample_t** result_block, sp_time_t* result_sample_count);
status_t sp_file_write(sp_file_t* file, sp_sample_t** block, sp_time_t sample_count, sp_time_t* result_sample_count);
status_t sp_file_position(sp_file_t* file, sp_time_t* result_position);
status_t sp_file_position_set(sp_file_t* file, sp_time_t sample_offset);
status_t sp_file_open(uint8_t* path, int mode, sp_channel_count_t channel_count, sp_sample_rate_t sample_rate, sp_file_t* result_file);
status_t sp_file_close(sp_file_t a);
status_t sp_block_to_file(sp_block_t block, uint8_t* path, sp_time_t rate);
status_t sp_block_new(sp_channel_count_t channel_count, sp_time_t sample_count, sp_block_t* out_block);
uint8_t* sp_status_description(status_t a);
uint8_t* sp_status_name(status_t a);
sp_sample_t sp_sin_lq(sp_float_t a);
sp_float_t sp_sinc(sp_float_t a);
sp_float_t sp_window_blackman(sp_float_t a, sp_time_t width);
void sp_spectral_inversion_ir(sp_sample_t* a, sp_time_t a_len);
void sp_spectral_reversal_ir(sp_sample_t* a, sp_time_t a_len);
int sp_fft(sp_time_t input_len, double* input_or_output_real, double* input_or_output_imag);
int sp_ffti(sp_time_t input_len, double* input_or_output_real, double* input_or_output_imag);
void sp_convolve_one(sp_sample_t* a, sp_time_t a_len, sp_sample_t* b, sp_time_t b_len, sp_sample_t* result_samples);
void sp_convolve(sp_sample_t* a, sp_time_t a_len, sp_sample_t* b, sp_time_t b_len, sp_time_t result_carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples);
void sp_block_free(sp_block_t a);
sp_block_t sp_block_with_offset(sp_block_t a, sp_time_t offset);
status_t sp_null_ir(sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_passthrough_ir(sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_initialise(uint16_t cpu_count, sp_channel_count_t channels, sp_time_t rate);
void sp_sine_period(sp_time_t size, sp_sample_t* out);
sp_time_t sp_phase(sp_time_t current, sp_time_t change, sp_time_t cycle);
sp_time_t sp_phase_float(sp_time_t current, double change, sp_time_t cycle);
sp_sample_t sp_square(sp_time_t t, sp_time_t size);
sp_sample_t sp_triangle(sp_time_t t, sp_time_t a, sp_time_t b);
sp_time_t sp_time_expt(sp_time_t base, sp_time_t exp);
sp_time_t sp_time_factorial(sp_time_t a);
/* arrays */

#define sp_samples_zero(a, size) memset(a, 0, (size * sizeof(sp_sample_t)))
#define sp_times_zero(a, size) memset(a, 0, (size * sizeof(sp_time_t)))
#define sp_time_interpolate_linear(a, b, t) sp_cheap_round_positive((((1 - ((sp_sample_t)(t))) * ((sp_sample_t)(a))) + (t * ((sp_sample_t)(b)))))
#define sp_sample_interpolate_linear(a, b, t) (((1 - t) * a) + (t * b))
#define sp_sequence_set_equal(a, b) ((a.size == b.size) && (((0 == a.size) && (0 == b.size)) || (0 == memcmp((a.data), (b.data), (a.size * sizeof(sp_time_t))))))
typedef struct {
  sp_time_t size;
  uint8_t* data;
} sp_sequence_set_key_t;
uint64_t sp_u64_from_array(uint8_t* a, sp_time_t size);
sp_sequence_set_key_t sp_sequence_set_null = { 0, 0 };
uint64_t sp_sequence_set_hash(sp_sequence_set_key_t a, sp_time_t memory_size) { return ((sp_u64_from_array((a.data), (a.size)) % memory_size)); }
sph_set_declare_type_nonull(sp_sequence_set, sp_sequence_set_key_t, sp_sequence_set_hash, sp_sequence_set_equal, sp_sequence_set_null, 2);
sph_set_declare_type(sp_time_set, sp_time_t, sph_set_hash_integer, sph_set_equal_integer, 0, 1, 2);
hashtable_declare_type(sp_sequence_hashtable, sp_sequence_set_key_t, sp_time_t, sp_sequence_set_hash, sp_sequence_set_equal, 2);
sp_sample_t sp_samples_absolute_max(sp_sample_t* in, sp_time_t in_size);
void sp_samples_add_1(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out);
void sp_samples_add(sp_sample_t* a, sp_time_t size, sp_sample_t* b, sp_sample_t* out);
void sp_samples_and(sp_sample_t* a, sp_sample_t* b, sp_time_t size, sp_sample_t limit, sp_sample_t* out);
void sp_samples_display(sp_sample_t* a, sp_time_t size);
void sp_samples_divide_1(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out);
void sp_samples_divide(sp_sample_t* a, sp_time_t size, sp_sample_t* b, sp_sample_t* out);
uint8_t sp_samples_equal_1(sp_sample_t* a, sp_time_t size, sp_sample_t n);
uint8_t sp_samples_every_equal(sp_sample_t* a, sp_time_t size, sp_sample_t n);
void sp_samples_multiply_1(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out);
void sp_samples_multiply(sp_sample_t* a, sp_time_t size, sp_sample_t* b, sp_sample_t* out);
status_t sp_samples_new(sp_time_t size, sp_sample_t** out);
void sp_samples_or(sp_sample_t* a, sp_sample_t* b, sp_time_t size, sp_sample_t limit, sp_sample_t* out);
uint8_t sp_samples_sort_less(void* a, ssize_t b, ssize_t c);
void sp_samples_sort_swap(void* a, ssize_t b, ssize_t c);
void sp_samples_reverse(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
void sp_samples_set_unity_gain(sp_sample_t* in, sp_time_t in_size, sp_sample_t* out);
void sp_samples_square(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
void sp_samples_subtract_1(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out);
void sp_samples_subtract(sp_sample_t* a, sp_time_t size, sp_sample_t* b, sp_sample_t* out);
void sp_samples_to_times(sp_sample_t* in, sp_time_t in_size, sp_time_t* out);
void sp_samples_xor(sp_sample_t* a, sp_sample_t* b, sp_time_t size, sp_sample_t limit, sp_sample_t* out);
status_t sp_samples_duplicate(sp_sample_t* a, sp_time_t size, sp_sample_t** out);
void sp_samples_differences(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
void sp_samples_additions(sp_sample_t start, sp_sample_t summand, sp_time_t count, sp_sample_t* out);
void sp_samples_divisions(sp_sample_t start, sp_sample_t n, sp_time_t count, sp_sample_t* out);
void sp_samples_scale_y(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out);
void sp_samples_scale_sum(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out);
void sp_times_add_1(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out);
void sp_times_add(sp_time_t* a, sp_time_t size, sp_time_t* b, sp_time_t* out);
void sp_times_and(sp_time_t* a, sp_time_t* b, sp_time_t size, sp_time_t limit, sp_time_t* out);
void sp_times_display(sp_time_t* a, sp_time_t size);
void sp_times_divide_1(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out);
void sp_times_divide(sp_time_t* a, sp_time_t size, sp_time_t* b, sp_time_t* out);
uint8_t sp_times_equal_1(sp_time_t* a, sp_time_t size, sp_time_t n);
void sp_times_multiply_1(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out);
void sp_times_multiply(sp_time_t* a, sp_time_t size, sp_time_t* b, sp_time_t* out);
status_t sp_times_new(sp_time_t size, sp_time_t** out);
void sp_times_or(sp_time_t* a, sp_time_t* b, sp_time_t size, sp_time_t limit, sp_time_t* out);
void sp_times_set_1(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out);
void sp_samples_set_1(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out);
uint8_t sp_times_sort_less(void* a, ssize_t b, ssize_t c);
void sp_times_sort_swap(void* a, ssize_t b, ssize_t c);
void sp_times_reverse(sp_time_t* a, sp_time_t size, sp_time_t* out);
void sp_times_square(sp_time_t* a, sp_time_t size, sp_time_t* out);
void sp_times_subtract_1(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out);
void sp_times_subtract(sp_time_t* a, sp_time_t size, sp_time_t* b, sp_time_t* out);
void sp_times_xor(sp_time_t* a, sp_time_t* b, sp_time_t size, sp_time_t limit, sp_time_t* out);
status_t sp_times_duplicate(sp_time_t a, sp_time_t size, sp_time_t** out);
void sp_times_differences(sp_time_t* a, sp_time_t size, sp_time_t* out);
void sp_times_cusum(sp_time_t* a, sp_time_t size, sp_time_t* out);
sp_time_t sp_time_random_custom(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size, sp_time_t range);
sp_time_t sp_time_random_discrete(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size);
void sp_times_random_discrete(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size, sp_time_t count, sp_time_t* out);
sp_sample_t sp_sample_random_custom(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size, sp_sample_t range);
void sp_times_swap(sp_time_t* a, ssize_t i1, ssize_t i2);
void sp_times_sequence_increment(sp_time_t* a, sp_time_t size, sp_time_t set_size);
status_t sp_times_compositions(sp_time_t sum, sp_time_t*** out, sp_time_t* out_size, sp_time_t** out_sizes);
status_t sp_times_permutations(sp_time_t size, sp_time_t* set, sp_time_t set_size, sp_time_t*** out, sp_time_t* out_size);
void sp_times_multiplications(sp_time_t start, sp_time_t factor, sp_time_t count, sp_time_t* out);
void sp_times_additions(sp_time_t start, sp_time_t summand, sp_time_t count, sp_time_t* out);
void sp_times_select(sp_time_t* a, sp_time_t* indices, sp_time_t size, sp_time_t* out);
void sp_times_bits_to_times(sp_time_t* a, sp_time_t size, sp_time_t* out);
void sp_times_shuffle(sp_random_state_t* state, sp_time_t* a, sp_time_t size);
status_t sp_times_random_binary(sp_random_state_t* state, sp_time_t size, sp_time_t* out);
void sp_times_gt_indices(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out, sp_time_t* out_size);
void sp_times_select_random(sp_random_state_t* state, sp_time_t* a, sp_time_t size, sp_time_t* out, sp_time_t* out_size);
status_t sp_times_constant(sp_time_t a, sp_time_t size, sp_time_t value, sp_time_t** out);
void sp_shuffle(sp_random_state_t* state, void (*swap)(void*, size_t, size_t), void* a, size_t size);
status_t sp_times_scale(sp_time_t* a, sp_time_t a_size, sp_time_t factor, sp_time_t* out);
void sp_times_shuffle_swap(void* a, size_t i1, size_t i2);
status_t sp_samples_smooth(sp_sample_t* a, sp_time_t size, sp_time_t radius, sp_sample_t* out);
void sp_times_array_free(sp_time_t** a, sp_time_t size);
void sp_samples_array_free(sp_sample_t** a, sp_time_t size);
uint8_t sp_times_contains(sp_time_t* a, sp_time_t size, sp_time_t b);
void sp_times_random_discrete_unique(sp_random_state_t* state, sp_time_t* cudist, sp_time_t cudist_size, sp_time_t size, sp_time_t* out);
void sp_times_sequences(sp_time_t base, sp_time_t digits, sp_time_t size, sp_time_t* out);
void sp_times_range(sp_time_t start, sp_time_t end, sp_time_t* out);
sp_time_t sp_time_round_to_multiple(sp_time_t a, sp_time_t base);
void sp_times_limit(sp_time_t* a, sp_time_t size, sp_time_t n, sp_time_t* out);
void sp_samples_limit_abs(sp_sample_t* a, sp_time_t size, sp_sample_t n, sp_sample_t* out);
typedef struct {
  sp_time_t count;
  sp_time_t* sequence;
} sp_times_counted_sequences_t;
void sp_times_counted_sequences_sort_swap(void* a, ssize_t b, ssize_t c);
uint8_t sp_times_counted_sequences_sort_less(void* a, ssize_t b, ssize_t c);
uint8_t sp_times_counted_sequences_sort_greater(void* a, ssize_t b, ssize_t c);
status_t sp_times_deduplicate(sp_time_t* a, sp_time_t size, sp_time_t* out, sp_time_t* out_size);
void sp_times_counted_sequences_hash(sp_time_t* a, sp_time_t size, sp_time_t width, sp_sequence_hashtable_t out);
void sp_times_counted_sequences(sp_sequence_hashtable_t known, sp_time_t limit, sp_times_counted_sequences_t* out, sp_time_t* out_size);
void sp_times_remove(sp_time_t* in, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out);
void sp_times_insert_space(sp_time_t* in, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out);
void sp_times_subdivide(sp_time_t* a, sp_time_t size, sp_time_t index, sp_time_t count, sp_time_t* out);
void sp_times_blend(sp_time_t* a, sp_time_t* b, sp_sample_t fraction, sp_time_t size, sp_time_t* out);
void sp_times_mask(sp_time_t* a, sp_time_t* b, sp_sample_t* coefficients, sp_time_t size, sp_time_t* out);
void sp_samples_blend(sp_sample_t* a, sp_sample_t* b, sp_sample_t fraction, sp_time_t size, sp_sample_t* out);
/* filter */

#define sp_filter_state_t sp_convolution_filter_state_t
#define sp_filter_state_free sp_convolution_filter_state_free
#define sp_cheap_filter_passes_limit 8
#define sp_cheap_filter_lp(...) sp_cheap_filter(sp_state_variable_filter_lp, __VA_ARGS__)
#define sp_cheap_filter_hp(...) sp_cheap_filter(sp_state_variable_filter_hp, __VA_ARGS__)
#define sp_cheap_filter_bp(...) sp_cheap_filter(sp_state_variable_filter_bp, __VA_ARGS__)
#define sp_cheap_filter_br(...) sp_cheap_filter(sp_state_variable_filter_br, __VA_ARGS__)
#define sp_declare_cheap_filter_state(name) sp_cheap_filter_state_t name = { 0 }
typedef status_t (*sp_convolution_filter_ir_f_t)(void*, sp_sample_t**, sp_time_t*);
typedef struct {
  sp_sample_t* carryover;
  sp_time_t carryover_len;
  sp_time_t carryover_alloc_len;
  sp_sample_t* ir;
  sp_convolution_filter_ir_f_t ir_f;
  void* ir_f_arguments;
  uint8_t ir_f_arguments_len;
  sp_time_t ir_len;
} sp_convolution_filter_state_t;
typedef struct {
  sp_sample_t* in_temp;
  sp_sample_t* out_temp;
  sp_sample_t svf_state[(2 * sp_cheap_filter_passes_limit)];
} sp_cheap_filter_state_t;
typedef void (*sp_state_variable_filter_t)(sp_sample_t*, sp_sample_t*, sp_float_t, sp_float_t, sp_time_t, sp_sample_t*);
void sp_moving_average(sp_sample_t* in, sp_time_t in_size, sp_sample_t* prev, sp_sample_t* next, sp_time_t radius, sp_sample_t* out);
sp_time_t sp_windowed_sinc_lp_hp_ir_length(sp_float_t transition);
status_t sp_windowed_sinc_ir(sp_float_t cutoff, sp_float_t transition, sp_time_t* result_len, sp_sample_t** result_ir);
void sp_convolution_filter_state_free(sp_convolution_filter_state_t* state);
status_t sp_convolution_filter_state_set(sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state);
status_t sp_convolution_filter(sp_sample_t* in, sp_time_t in_len, sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples);
status_t sp_windowed_sinc_lp_hp_ir(sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_windowed_sinc_bp_br_ir(sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_windowed_sinc_lp_hp_ir_f(void* arguments, sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_windowed_sinc_bp_br_ir_f(void* arguments, sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_windowed_sinc_lp_hp(sp_sample_t* in, sp_time_t in_len, sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples);
status_t sp_windowed_sinc_bp_br(sp_sample_t* in, sp_time_t in_len, sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples);
status_t sp_windowed_sinc_lp_hp_ir(sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_sample_t** out_ir, sp_time_t* out_len);
void sp_state_variable_filter_lp(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_time_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_hp(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_time_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_bp(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_time_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_br(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_time_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_peak(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_time_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_all(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_time_t q_factor, sp_sample_t* state);
void sp_cheap_filter(sp_state_variable_filter_t type, sp_sample_t* in, sp_time_t in_size, sp_float_t cutoff, sp_time_t passes, sp_float_t q_factor, sp_cheap_filter_state_t* state, sp_sample_t* out);
void sp_cheap_filter_state_free(sp_cheap_filter_state_t* a);
status_t sp_cheap_filter_state_new(sp_time_t max_size, sp_time_t max_passes, sp_cheap_filter_state_t* out_state);
status_t sp_filter(sp_sample_t* in, sp_time_t in_size, sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_filter_state_t** out_state, sp_sample_t* out_samples);
/* plot */
#define sp_block_plot_1(a) sp_plot_samples(((a.samples)[0]), (a.size))
void sp_plot_samples(sp_sample_t* a, sp_time_t a_size);
void sp_plot_times(sp_time_t* a, sp_time_t a_size);
void sp_plot_samples_to_file(sp_sample_t* a, sp_time_t a_size, uint8_t* path);
void sp_plot_times_to_file(sp_time_t* a, sp_time_t a_size, uint8_t* path);
void sp_plot_samples_file(uint8_t* path, uint8_t use_steps);
void sp_plot_spectrum_to_file(sp_sample_t* a, sp_time_t a_size, uint8_t* path);
void sp_plot_spectrum_file(uint8_t* path);
void sp_plot_spectrum(sp_sample_t* a, sp_time_t a_size);
/* sequencer */

#define sp_declare_wave_event_config(name) sp_wave_event_config_t name = { 0 }
#define sp_declare_event(id) sp_event_t id = { 0 }
#define sp_declare_event_2(id1, id2) \
  sp_declare_event(id1); \
  sp_declare_event(id2)
#define sp_declare_event_3(id1, id2, id3) \
  sp_declare_event(id1); \
  sp_declare_event(id2); \
  sp_declare_event(id3)
#define sp_declare_event_4(id1, id2, id3, id4) \
  sp_declare_event_2(id1, id2); \
  sp_declare_event_2(id3, id4)
#define sp_declare_events(id, size) \
  array4_declare(id, sp_events_t); \
  sp_event_t id##_data[size]; \
  array4_take(id, id##_data, size, 0)

/** optional helper that sets defaults. the mod arrays must be zero if not used */
#define sp_declare_noise_event_config(name) sp_noise_event_config_t name = { 0 }
#define sp_declare_cheap_noise_event_config(name) sp_cheap_noise_event_config_t name = { 0 }
#define sp_event_duration(a) (a.end - a.start)
#define sp_event_duration_set(a, duration) a.end = (a.start + duration)
#define sp_event_move(a, start) \
  a.end = (start + (a.end - a.start)); \
  a.start = start
#define sp_group_size_t uint16_t
#define sp_group_events(a) *((sp_events_t*)(a.state))
#define sp_group_add(a, event) \
  array4_add((sp_group_events(a)), event); \
  if (a.end < event.end) { \
    a.end = event.end; \
  }
#define sp_event_memory_add(a, data) sp_event_memory_add_handler(a, data, free)

/** sp_event_t void* function:{void* -> void} */
#define sp_event_memory_add_handler(a, data_, free) \
  ((a.memory)[a.memory_used]).data = data_; \
  ((a.memory)[a.memory_used]).free = free; \
  a.memory_used += 1
#define sp_event_memory_add_2(a, data1, data2) \
  sp_event_memory_add(a, data1); \
  sp_event_memory_add(a, data2)
#define sp_event_memory_add_3(a, data1, data2, data3) \
  sp_event_memory_add_2(a, data1, data2); \
  sp_event_memory_add(a, data3)
#define sp_event_memory_init(a, size) sph_helper_malloc((size * sizeof(sp_memory_t)), (&(a.memory)))
typedef void (*sp_memory_free_t)(void*);
typedef struct {
  sp_memory_free_t free;
  void* data;
} sp_memory_t;
struct sp_event_t;
typedef struct sp_event_t {
  sp_time_t start;
  sp_time_t end;
  void* state;
  status_t (*generate)(sp_time_t, sp_time_t, sp_block_t, void*);
  status_t (*prepare)(struct sp_event_t*);
  void (*free)(struct sp_event_t*);
  sp_memory_t* memory;
  sp_time_half_t memory_used;
} sp_event_t;
typedef status_t (*sp_event_generate_t)(sp_time_t, sp_time_t, sp_block_t, void*);
typedef struct {
  boolean use;
  boolean mute;
  sp_time_t delay;
  sp_time_t phs;
  sp_sample_t amp;
  sp_sample_t* amod;
} sp_channel_config_t;
typedef struct {
  sp_sample_t* wvf;
  sp_time_t wvf_size;
  sp_time_t phs;
  sp_time_t frq;
  sp_time_t* fmod;
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_channel_count_t channels;
  sp_channel_config_t channel_config[sp_channel_limit];
} sp_wave_event_config_t;
typedef struct {
  sp_sample_t* wvf;
  sp_time_t wvf_size;
  sp_time_t phs;
  sp_time_t frq;
  sp_time_t* fmod;
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_channel_count_t channel;
} sp_wave_event_state_t;
typedef struct {
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_sample_t cutl;
  sp_sample_t cuth;
  sp_sample_t trnl;
  sp_sample_t trnh;
  sp_sample_t* cutl_mod;
  sp_sample_t* cuth_mod;
  sp_sample_t* trnl_mod;
  sp_sample_t* trnh_mod;
  sp_time_t resolution;
  uint8_t is_reject;
  sp_channel_count_t channels;
  sp_channel_config_t channel_config[sp_channel_limit];
} sp_noise_event_config_t;
typedef struct {
  sp_sample_t amp;
  sp_sample_t* amod;
  sp_sample_t cut;
  sp_sample_t* cut_mod;
  sp_sample_t q_factor;
  sp_time_t passes;
  sp_state_variable_filter_t type;
  sp_random_state_t* random_state;
  sp_time_t resolution;
  sp_channel_count_t channels;
  sp_channel_config_t channel_config[sp_channel_limit];
} sp_cheap_noise_event_config_t;
typedef status_t (*sp_map_event_generate_t)(sp_time_t, sp_time_t, sp_block_t, sp_block_t, void*);
typedef struct {
  sp_event_t event;
  sp_map_event_generate_t generate;
  void* state;
} sp_map_event_state_t;
array4_declare_type(sp_events, sp_event_t);
void sp_seq_events_free(sp_events_t* events);
void sp_event_memory_free(sp_event_t* event);
void sp_seq_events_prepare(sp_events_t* events);
status_t sp_seq(sp_time_t start, sp_time_t end, sp_block_t out, sp_events_t* events);
status_t sp_seq_parallel(sp_time_t start, sp_time_t end, sp_block_t out, sp_events_t* events);
status_t sp_wave_event(sp_time_t start, sp_time_t end, sp_wave_event_config_t config, sp_event_t* out);
status_t sp_noise_event(sp_time_t start, sp_time_t end, sp_noise_event_config_t config, sp_event_t* out_event);
void sp_events_array_free(sp_event_t* events, sp_time_t size);
status_t sp_wave_event(sp_time_t start, sp_time_t end, sp_wave_event_config_t config, sp_event_t* out);
status_t sp_cheap_noise_event(sp_time_t start, sp_time_t end, sp_cheap_noise_event_config_t config, sp_event_t* out_event);
status_t sp_group_new(sp_time_t start, sp_group_size_t event_size, sp_event_t* out);
void sp_group_append(sp_event_t* a, sp_event_t event);
void sp_group_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event);
void sp_group_event_parallel_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event);
void sp_group_event_free(sp_event_t* a);
status_t sp_map_event(sp_event_t event, sp_map_event_generate_t f, void* state, uint8_t isolate, sp_event_t* out);
sp_channel_config_t sp_channel_config(boolean mute, sp_time_t delay, sp_time_t phs, sp_sample_t amp, sp_sample_t* amod);
/* path */

#define sp_path_t spline_path_t
#define sp_path_time_t spline_path_time_t
#define sp_path_value_t spline_path_value_t
#define sp_path_point_t spline_path_point_t
#define sp_path_segment_t spline_path_segment_t
#define sp_path_segment_count_t spline_path_segment_count_t
#define sp_path_line spline_path_line
#define sp_path_move spline_path_move
#define sp_path_bezier spline_path_bezier
#define sp_path_constant spline_path_constant
#define sp_path_path spline_path_path
#define sp_path_prepare_segments spline_path_prepare_segments
#define sp_path_i_line spline_path_i_line
#define sp_path_i_move spline_path_i_move
#define sp_path_i_bezier spline_path_i_bezier
#define sp_path_i_constant spline_path_i_constant
#define sp_path_i_path spline_path_i_path
#define sp_path_end spline_path_end
#define sp_path_size spline_path_size
#define sp_path_free spline_path_free
#define sp_path_get spline_path_get
#define sp_path_set spline_path_set
#define sp_path_times_constant(out, size, value) sp_path_times_2(out, size, (sp_path_move(0, value)), (sp_path_constant()))
#define sp_path_samples_constant(out, size, value) sp_path_samples_2(out, size, (sp_path_move(0, value)), (sp_path_constant()))
status_t sp_path_samples_new(sp_path_t path, sp_path_time_t size, sp_sample_t** out);
status_t sp_path_samples_1(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1);
status_t sp_path_samples_2(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2);
status_t sp_path_samples_3(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3);
status_t sp_path_samples_4(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4);
status_t sp_path_times_new(sp_path_t path, sp_path_time_t size, sp_time_t** out);
status_t sp_path_times_1(sp_time_t** out, sp_time_t size, sp_path_segment_t s1);
status_t sp_path_times_2(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2);
status_t sp_path_times_3(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3);
status_t sp_path_times_4(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4);
status_t sp_path_derivation(sp_path_t path, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_time_t index, sp_path_t* out);
status_t sp_path_samples_derivation(sp_path_t path, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_time_t index, sp_sample_t** out, sp_path_time_t* out_size);
status_t sp_path_times_derivation(sp_path_t path, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_time_t index, sp_time_t** out, sp_path_time_t* out_size);
void sp_path_multiply(sp_path_t path, sp_sample_t x_factor, sp_sample_t y_factor);
status_t sp_path_derivations_normalized(sp_path_t base, sp_time_t count, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_path_t** out);
status_t sp_path_samples_derivations_normalized(sp_path_t path, sp_time_t count, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_sample_t*** out, sp_time_t** out_sizes);
/* statistics */
sp_time_t sp_sequence_max(sp_time_t size, sp_time_t min_size);
sp_time_t sp_set_sequence_max(sp_time_t set_size, sp_time_t selection_size);
sp_time_t sp_permutations_max(sp_time_t set_size, sp_time_t selection_size);
sp_time_t sp_compositions_max(sp_time_t sum);
typedef uint8_t (*sp_stat_times_f_t)(sp_time_t*, sp_time_t, sp_sample_t*);
typedef uint8_t (*sp_stat_samples_f_t)(sp_sample_t*, sp_time_t, sp_sample_t*);
uint8_t sp_stat_times_range(sp_time_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_times_repetition(sp_time_t* a, sp_time_t size, sp_time_t width, sp_sample_t* out);
uint8_t sp_stat_times_repetition_all(sp_time_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_times_mean(sp_time_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_times_deviation(sp_time_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_times_median(sp_time_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_times_center(sp_time_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_times_inharmonicity(sp_time_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_times_kurtosis(sp_time_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_times_skewness(sp_time_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_samples_repetition_all(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_samples_mean(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_samples_deviation(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_samples_inharmonicity(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_samples_median(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_samples_center(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_samples_range(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_samples_kurtosis(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
uint8_t sp_stat_samples_skewness(sp_sample_t* a, sp_time_t size, sp_sample_t* out);
void sp_samples_scale_to_times(sp_sample_t* a, sp_time_t size, sp_time_t max, sp_time_t* out);
sp_time_t sp_stat_unique_max(sp_time_t size, sp_time_t width);
sp_time_t sp_stat_unique_all_max(sp_time_t size);
sp_time_t sp_stat_repetition_all_max(sp_time_t size);
uint8_t sp_stat_times_repetition(sp_time_t* a, sp_time_t size, sp_time_t width, sp_sample_t* out);
sp_time_t sp_stat_repetition_max(sp_time_t size, sp_time_t width);
/* main 2 */

/** return a sample count relative to the current default sample rate sp_rate.
     (rate / d * n)
     example (rt 1 2) returns half of sp_rate */
#define rt(n, d) ((sp_time_t)(((sp_rate / d) * n)))
typedef struct {
  sp_channel_count_t channels;
  sp_time_t rate;
  sp_time_t block_size;
} sp_render_config_t;
sp_render_config_t sp_render_config(sp_channel_count_t channels, sp_time_t rate, sp_time_t block_size);
status_t sp_render_file(sp_events_t events, sp_time_t start, sp_time_t end, sp_render_config_t config, uint8_t* path);
status_t sp_render_block(sp_events_t events, sp_time_t start, sp_time_t end, sp_render_config_t config, sp_block_t* out);
status_t sp_render_quick(sp_events_t events, uint8_t file_or_plot);