
#define f128 long double
#define f64 double
#define sp_bool_t uint8_t
#define spline_path_value_t sp_sample_t
#define sp_memory_error status_set_goto(sp_s_group_sp, sp_s_id_memory)
#define sp_noise sp_samples_random
#define sp_random_state_t sph_random_state_t
#define sp_sample_to_time sp_cheap_round_positive
#define sp_s_group_libc "libc"
#define sp_s_group_sph "sph"
#define sp_s_group_sp "sp"
#define sp_s_id_eof 6
#define sp_s_id_file_eof 5
#define sp_s_id_file_not_implemented 4
#define sp_s_id_file_read 3
#define sp_s_id_file_write 2
#define sp_s_id_input_type 7
#define sp_s_id_invalid_argument 9
#define sp_s_id_memory 8
#define sp_s_id_not_implemented 10
#define sp_s_id_undefined 1
#define sp_status_declare status_declare_group(sp_s_group_sp)
#define srq status_require
#define sp_max_frq (sp_rate / 2)
#define sp_declare_block(id) sp_block_t id = { 0 }
#define sp_time_to_sample(x) ((sp_sample_t)(x))
#define sp_subtract(a, b) (a - b)
#define sp_block_declare(a) \
  sp_block_t a; \
  a.size = 0
#define sp_cheap_round_positive(a) ((sp_time_t)((0.5 + a)))

/** only works for non-negative values */
#define sp_cheap_floor_positive(a) ((sp_time_t)(a))
#define sp_cheap_ceiling_positive(a) (((sp_time_t)(a)) + (((sp_time_t)(a)) < a))
#define sp_inline_max(a, b) ((a > b) ? a : b)
#define sp_inline_min(a, b) ((a < b) ? a : b)
#define sp_inline_limit(x, min_value, max_value) sp_inline_max(min_value, (sp_inline_min(max_value, x)))
#define sp_inline_optional(a, b) (a ? a : b)
#define sp_inline_default(a, b) (!a ? (a = b) : 0)

/** subtract the smaller number from the greater number,
     regardless of if the smallest is the first or the second argument */
#define sp_inline_absolute_difference(a, b) ((a > b) ? (a - b) : (b - a))
#define sp_inline_abs(a) ((0 > a) ? (-1 * a) : a)

/** subtract b from a but return 0 for negative results */
#define sp_inline_no_underflow_subtract(a, b) ((a > b) ? (a - b) : 0)

/** divide a by b (a / b) but return 0 if b is zero */
#define sp_inline_no_zero_divide(a, b) ((0 == b) ? 0 : (a / b))
#define sp_status_set(_id) \
  status.group = sp_s_group_sp; \
  status.id = _id
#define sp_status_set_goto(id) \
  sp_status_set(id); \
  status_goto
#define sp_malloc_type(count, type, pointer_address) sph_helper_malloc((count * sizeof(type)), pointer_address)
#define sp_calloc_type(count, type, pointer_address) sph_helper_calloc((count * sizeof(type)), pointer_address)
#define sp_realloc_type(count, type, pointer_address) sph_helper_realloc((count * sizeof(type)), pointer_address)
#define sp_malloc_type_srq(count, type, pointer_address) status_require((sp_malloc_type(count, type, pointer_address)))
#define sp_hz_to_samples(x) (sp_rate / x)
#define sp_samples_to_hz(x) ((sp_time_t)((sp_rate / x)))
#define sp_hz_to_factor(x) (((sp_sample_t)(x)) / ((sp_sample_t)(sp_rate)))
#define sp_hz_to_rad(a) (2 * M_PI * a)
#define sp_rad_to_hz(a) ((M_PI / 2) * a)
#define sp_factor_to_hz(x) ((sp_time_t)((x * sp_rate)))
#define sp_optional_array_get(array, fixed, index) (array ? array[index] : fixed)
#define sp_sample_to_unit(a) ((1 + a) / 2.0)
#define sp_time_random() sp_time_random_primitive((&sp_random_state))
#define sp_times_random(size, out) sp_times_random_primitive((&sp_random_state), size, out)
#define sp_samples_random(size, out) sp_samples_random_primitive((&sp_random_state), size, out)
#define sp_units_random(size, out) sp_units_random_primitive((&sp_random_state), size, out)
#define sp_times_random_bounded(range, size, out) sp_times_random_bounded_primitive((&sp_random_state), range, size, out)
#define sp_samples_random_bounded(range, size, out) sp_samples_random_bounded_primitive((&sp_random_state), range, size, out)
#define sp_units_random_bounded(range, size, out) sp_units_random_bounded_primitive((&sp_random_state), range, size, out)
#define sp_time_random_bounded(range) sp_time_random_bounded_primitive((&sp_random_state), range)
#define sp_sample_random() sp_sample_random_primitive((&sp_random_state))
#define sp_sample_random_bounded(range) sp_sample_random_bounded_primitive((&sp_random_state), range)
#define sp_unit_random() sp_unit_random_primitive((&sp_random_state))
#define error_memory_init(register_size) memreg2_init_named(error, register_size)
#define local_memory_init(register_size) memreg2_init_named(exit, register_size)
#define error_memory_free memreg2_free_named(error)
#define local_memory_free memreg2_free_named(exit)
#define error_memory_add2(address, handler) memreg2_add_named(error, address, handler)
#define local_memory_add2(address, handler) memreg2_add_named(exit, address, handler)
#define error_memory_add(address) error_memory_add2(address, free)
#define local_memory_add(address) local_memory_add2(address, free)
#define sp_time_odd_p(a) (a & 1)
#define sp_local_alloc_srq(allocator, size, pointer_address) \
  srq((allocator(size, pointer_address))); \
  local_memory_add((*pointer_address))
#define sp_local_units_srq(size, pointer_address) sp_local_alloc_srq(sp_units_new, size, pointer_address)
#define sp_local_times_srq(size, pointer_address) sp_local_alloc_srq(sp_times_new, size, pointer_address)
#define sp_local_samples_srq(size, pointer_address) sp_local_alloc_srq(sp_samples_new, size, pointer_address)

/** return a sample count relative to the current default sample rate sp_rate.
     (rate / d * n)
     example (rt 1 2) returns the count of samples for half a second of sound */
#define sp_rate_duration(n, d) ((sp_time_t)(((sp_rate / d) * n)))

/** like rt but works in places where variables are not allowed (ex: array initializers) and before sp_initialize has been called */
#define sp_duration(n, d) ((sp_time_t)(((_sp_rate / d) * n)))
#define spd sp_duration
#define sprd sp_rate_duration
typedef struct {
  sp_channel_count_t channel_count;
  sp_time_t size;
  sp_sample_t* samples[sp_channel_count_limit];
} sp_block_t;
sp_channel_count_t sp_channel_count;
uint32_t sp_cpu_count;
typedef struct {
  FILE* file;
  sp_size_t data_size;
  sp_channel_count_t channel_count;
} sp_file_t;
sp_random_state_t sp_random_state;
sp_time_t sp_rate;
sp_time_t sp_sine_lfo_factor;
sp_sample_t* sp_sine_table_lfo;
sp_sample_t* sp_sine_table;
void sp_block_copy(sp_block_t a, sp_block_t b);
status_t sp_block_to_file(sp_block_t block, uint8_t* path, sp_time_t rate);
void sp_block_free(sp_block_t* a);
status_t sp_block_new(sp_channel_count_t channel_count, sp_time_t sample_count, sp_block_t* out_block);
sp_block_t sp_block_with_offset(sp_block_t a, sp_time_t offset);
void sp_block_zero(sp_block_t a);
void sp_convolve(sp_sample_t* a, sp_time_t a_len, sp_sample_t* b, sp_time_t b_len, sp_time_t result_carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples);
void sp_convolve_one(sp_sample_t* a, sp_time_t a_len, sp_sample_t* b, sp_time_t b_len, sp_sample_t* result_samples);
void sp_deinitialize();
int sp_ffti(sp_time_t input_len, double* input_or_output_real, double* input_or_output_imag);
int sp_fft(sp_time_t input_len, double* input_or_output_real, double* input_or_output_imag);
void sp_file_close_read(sp_file_t file);
void sp_file_close_write(sp_file_t* file);
status_t sp_file_open_read(uint8_t* path, sp_file_t* file);
status_t sp_file_open_write(uint8_t* path, sp_channel_count_t channel_count, sp_time_t sample_rate, sp_file_t* file);
status_t sp_file_read(sp_file_t file, sp_time_t sample_count, sp_sample_t** samples);
status_t sp_file_write(sp_file_t* file, sp_sample_t** samples, sp_time_t sample_count);
status_t sp_initialize(uint16_t cpu_count, sp_channel_count_t channel_count, sp_time_t rate);
status_t sp_null_ir(sp_sample_t** out_ir, sp_time_t* out_len);
sp_sample_t sp_pan_to_amp(sp_sample_t value, sp_channel_count_t channel);
status_t sp_passthrough_ir(sp_sample_t** out_ir, sp_time_t* out_len);
sp_time_t sp_phase(sp_time_t current, sp_time_t change, sp_time_t cycle);
sp_time_t sp_phase_float(sp_time_t current, double change, sp_time_t cycle);
sp_random_state_t sp_random_state_new(sp_time_t seed);
sp_sample_t sp_sample_max(sp_sample_t a, sp_sample_t b);
sp_sample_t sp_sample_min(sp_sample_t a, sp_sample_t b);
sp_sample_t sp_sinc(sp_sample_t a);
void sp_sine_lfo(sp_time_t size, sp_sample_t amp, sp_sample_t* amod, sp_time_t frq, sp_time_t* fmod, sp_time_t* phs_state, sp_sample_t* out);
void sp_sine_period(sp_time_t size, sp_sample_t* out);
void sp_sine(sp_time_t size, sp_sample_t amp, sp_sample_t* amod, sp_time_t frq, sp_time_t* fmod, sp_time_t* phs_state, sp_sample_t* out);
void sp_spectral_inversion_ir(sp_sample_t* a, sp_time_t a_len);
void sp_spectral_reversal_ir(sp_sample_t* a, sp_time_t a_len);
uint8_t* sp_status_description(status_t a);
uint8_t* sp_status_name(status_t a);
sp_time_t sp_time_factorial(sp_time_t a);
sp_time_t sp_time_max(sp_time_t a, sp_time_t b);
sp_time_t sp_time_min(sp_time_t a, sp_time_t b);
void sp_wave(sp_time_t size, sp_sample_t* wvf, sp_time_t wvf_size, sp_sample_t amp, sp_sample_t* amod, sp_time_t frq, sp_time_t* fmod, sp_time_t* phs_state, sp_sample_t* out);
sp_sample_t sp_window_blackman(sp_sample_t a, sp_time_t width);
/* extra */
sp_sample_t sp_triangle(sp_time_t t, sp_time_t a, sp_time_t b);
sp_sample_t sp_square(sp_time_t t, sp_time_t size);
sp_sample_t sp_pan_to_amp(sp_sample_t value, sp_channel_count_t channel);
sp_time_t sp_normal_random(sp_time_t min, sp_time_t max);
sp_time_t sp_time_harmonize(sp_time_t a, sp_time_t base, sp_sample_t amount);
sp_time_t sp_time_harmonize(sp_time_t a, sp_time_t base, sp_sample_t amount);
sp_time_t sp_time_deharmonize(sp_time_t a, sp_time_t base, sp_sample_t amount);
size_t sp_modulo_match(size_t index, size_t* divisors, size_t divisor_count);
sp_time_t sp_time_expt(sp_time_t base, sp_time_t exp);
sp_size_t sp_permutations_max(sp_size_t set_size, sp_size_t selection_size);
sp_size_t sp_compositions_max(sp_size_t sum);
sp_size_t sp_set_sequence_max(sp_size_t set_size, sp_size_t selection_size);