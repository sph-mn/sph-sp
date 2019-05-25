#include <byteswap.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>
/* configuration */
#ifndef sp_channel_count_t
#define sp_channel_count_t uint8_t
#endif
#ifndef sp_file_format
#define sp_file_format (SF_FORMAT_WAV | SF_FORMAT_FLOAT)
#endif
#ifndef sp_float_t
#define sp_float_t double
#endif
#ifndef sp_count_t
#define sp_count_t uint32_t
#endif
#ifndef sp_count_max
#define sp_count_max UINT32_MAX
#endif
#ifndef sp_sample_rate_t
#define sp_sample_rate_t uint32_t
#endif
#ifndef sp_sample_sum
#define sp_sample_sum f64_sum
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
#ifndef sp_synth_count_t
#define sp_synth_count_t uint16_t
#endif
#ifndef sp_channel_limit
#define sp_channel_limit 2
#endif
#ifndef sp_synth_sine
#define sp_synth_sine sp_sine_96
#endif
#ifndef sp_synth_partial_limit
#define sp_synth_partial_limit 64
#endif
#ifndef spline_path_time_t
#define spline_path_time_t sp_count_t
#endif
#ifndef spline_path_value_t
#define spline_path_value_t sp_sample_t
#endif
/* return status code and error handling. uses a local variable named "status" and a goto label named "exit".
      a status has an identifier and a group to discern between status identifiers of different libraries.
      status id 0 is success, everything else can be considered a failure or special case.
      status ids are 32 bit signed integers for compatibility with error return codes from many other existing libraries.
      group ids are strings to make it easier to create new groups that dont conflict with others compared to using numbers */
#define sph_status 1
#define status_id_success 0
#define status_group_undefined ""
#define status_declare status_t status = { status_id_success, status_group_undefined }
#define status_reset status_set_both(status_group_undefined, status_id_success)
#define status_is_success (status_id_success == status.id)
#define status_is_failure !status_is_success
#define status_goto goto exit
/** like status declare but with a default group */
#define status_declare_group(group) status_t status = { status_id_success, group }
#define status_set_both(group_id, status_id) \
  status.group = group_id; \
  status.id = status_id
/** update status with the result of expression and goto error on failure */
#define status_require(expression) \
  status = expression; \
  if (status_is_failure) { \
    status_goto; \
  }
/** set the status id and goto error */
#define status_set_id_goto(status_id) \
  status.id = status_id; \
  status_goto
#define status_set_group_goto(group_id) \
  status.group = group_id; \
  status_goto
#define status_set_both_goto(group_id, status_id) \
  status_set_both(group_id, status_id); \
  status_goto
/** like status-require but expression returns only status.id */
#define status_id_require(expression) \
  status.id = expression; \
  if (status_is_failure) { \
    status_goto; \
  }
typedef int32_t status_id_t;
typedef struct {
  status_id_t id;
  uint8_t* group;
} status_t;
/* * spline-path creates discrete 2d paths interpolated between some given points
  * maps from one independent discrete value to one dependent continuous value
  * only the dependent value is returned
  * kept minimal (only 2d, only selected interpolators, limited segment count) to be extremely fast
  * multidimensional interpolation can be archieved with multiple configs and calls
  * a copy of segments is made internally and only the copy is used
  * uses points as structs because pre-defined size arrays can not be used in structs
  * segments-len must be greater than zero
  * segments must be a valid spline-path segment configuration
  * interpolators are called with path-relative start/end inside segment and with out positioned at the segment output
  * all segment types have a fixed number of points. line: 1, bezier: 3, move: 1, constant: 0, path: 0
  * negative x values not supported
  * internally all segments start at (0 0) and no gaps are between segments
  * assumes that bit 0 is spline-path-value-t zero
  * segments draw to the endpoint inclusive, start point exclusive */
#include <inttypes.h>
#include <strings.h>
#include <stdlib.h>
#ifndef spline_path_time_t
#define spline_path_time_t uint32_t
#endif
#ifndef spline_path_value_t
#define spline_path_value_t double
#endif
#ifndef spline_path_segment_count_t
#define spline_path_segment_count_t uint16_t
#endif
#ifndef spline_path_time_max
#define spline_path_time_max UINT32_MAX
#endif
#define spline_path_point_limit 3
#define spline_path_interpolator_points_len(a) ((spline_path_i_bezier == a) ? 3 : 1)
typedef struct {
  spline_path_time_t x;
  spline_path_value_t y;
} spline_path_point_t;
typedef void (*spline_path_interpolator_t)(spline_path_time_t, spline_path_time_t, spline_path_point_t, spline_path_point_t*, void*, spline_path_value_t*);
typedef struct {
  spline_path_point_t _start;
  uint8_t _points_len;
  spline_path_point_t points[spline_path_point_limit];
  spline_path_interpolator_t interpolator;
  void* options;
} spline_path_segment_t;
typedef struct {
  spline_path_segment_count_t segments_len;
  spline_path_segment_t* segments;
} spline_path_t;
void spline_path_i_move(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_i_constant(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_i_line(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_i_bezier(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_get(spline_path_t path, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out);
void spline_path_i_path(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
spline_path_point_t spline_path_start(spline_path_t path);
spline_path_point_t spline_path_end(spline_path_t path);
uint8_t spline_path_new(spline_path_segment_count_t segments_len, spline_path_segment_t* segments, spline_path_t* out_path);
void spline_path_free(spline_path_t a);
uint8_t spline_path_new_get(spline_path_segment_count_t segments_len, spline_path_segment_t* segments, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out);
spline_path_segment_t spline_path_move(spline_path_time_t x, spline_path_value_t y);
spline_path_segment_t spline_path_line(spline_path_time_t x, spline_path_value_t y);
spline_path_segment_t spline_path_bezier(spline_path_time_t x1, spline_path_value_t y1, spline_path_time_t x2, spline_path_value_t y2, spline_path_time_t x3, spline_path_value_t y3);
spline_path_segment_t spline_path_constant();
spline_path_segment_t spline_path_path(spline_path_t* path);
/* shorter type names derived from inttypes.h */
#include <inttypes.h>
#define boolean uint8_t
#define i8 int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t
#define i8_least int_least8_t
#define i16_least int_least16_t
#define i32_least int_least32_t
#define i64_least int_least64_t
#define i8_fast int_fast8_t
#define i16_fast int_fast16_t
#define i32_fast int_fast32_t
#define i64_fast int_fast64_t
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define u8_least uint_least8_t
#define u16_least uint_least16_t
#define u32_least uint_least32_t
#define u64_least uint_least64_t
#define u8_fast uint_fast8_t
#define u16_fast uint_fast16_t
#define u32_fast uint_fast32_t
#define u64_fast uint_fast64_t
#define f32 float
#define f64 double
/* depends on types.c. have to use sph- prefix because standard lib uses random */
/** guarantees that all dyadic rationals of the form (k / 2**−53) will be equally likely. this conversion prefers the high bits of x.
     from http://xoshiro.di.unimi.it/ */
#define f64_from_u64(a) ((a >> 11) * (1.0 / (UINT64_C(1) << 53)))
#define define_sph_random(name, size_type, data_type, transfer) \
  /** return uniformly distributed random real numbers in the range -1 to 1. \
       implements xoshiro256plus from http://xoshiro.di.unimi.it/ \
       referenced by https://nullprogram.com/blog/2017/09/21/ */ \
  void name(sph_random_state_t* state, size_type size, data_type* out) { \
    u64 result_plus; \
    size_type i; \
    u64 t; \
    sph_random_state_t s; \
    s = *state; \
    for (i = 0; (i < size); i = (1 + i)) { \
      result_plus = ((s.data)[0] + (s.data)[3]); \
      t = ((s.data)[1] << 17); \
      (s.data)[2] = ((s.data)[2] ^ (s.data)[0]); \
      (s.data)[3] = ((s.data)[3] ^ (s.data)[1]); \
      (s.data)[1] = ((s.data)[1] ^ (s.data)[2]); \
      (s.data)[0] = ((s.data)[0] ^ (s.data)[3]); \
      (s.data)[2] = ((s.data)[2] ^ t); \
      (s.data)[3] = rotl(((s.data)[3]), 45); \
      out[i] = transfer; \
    }; \
    *state = s; \
  }
typedef struct {
  u64 data[4];
} sph_random_state_t;
sph_random_state_t sph_random_state_new(u64 seed);
void sph_random(sph_random_state_t* state, u32 size, f64* out);
/* main */
#define boolean uint8_t
#define f64 double
#define sp_file_bit_input 1
#define sp_file_bit_output 2
#define sp_file_bit_position 4
#define sp_file_bit_closed 8
#define sp_file_mode_read 1
#define sp_file_mode_write 2
#define sp_file_mode_read_write 3
#define sp_status_group_libc "libc"
#define sp_status_group_sndfile "sndfile"
#define sp_status_group_sp "sp"
#define sp_status_group_sph "sph"
#define sp_random sph_random
#define sp_random_state_t sph_random_state_t
#define sp_random_state_new sph_random_state_new
/** sample count to bit octets count */
#define sp_octets_to_samples(a) (a / sizeof(sp_sample_t))
#define sp_samples_to_octets(a) (a * sizeof(sp_sample_t))
#define sp_cheap_round_positive(a) ((sp_count_t)((0.5 + a)))
#define sp_cheap_floor_positive(a) ((sp_count_t)(a))
#define sp_cheap_ceiling_positive(a) (((sp_count_t)(a)) + (((sp_count_t)(a)) < a))
#define sp_block_set_null(a) a.channels = 0
enum { sp_status_id_file_channel_mismatch,
  sp_status_id_file_encoding,
  sp_status_id_file_header,
  sp_status_id_file_incompatible,
  sp_status_id_file_incomplete,
  sp_status_id_eof,
  sp_status_id_input_type,
  sp_status_id_memory,
  sp_status_id_invalid_argument,
  sp_status_id_not_implemented,
  sp_status_id_file_closed,
  sp_status_id_file_position,
  sp_status_id_file_type,
  sp_status_id_undefined };
typedef struct {
  sp_channel_count_t channels;
  sp_count_t size;
  sp_sample_t* samples[sp_channel_limit];
} sp_block_t;
typedef struct {
  uint8_t flags;
  sp_sample_rate_t sample_rate;
  sp_channel_count_t channel_count;
  void* data;
} sp_file_t;
sp_random_state_t sp_default_random_state;
status_t sp_file_read(sp_file_t* file, sp_count_t sample_count, sp_sample_t** result_block, sp_count_t* result_sample_count);
status_t sp_file_write(sp_file_t* file, sp_sample_t** block, sp_count_t sample_count, sp_count_t* result_sample_count);
status_t sp_file_position(sp_file_t* file, sp_count_t* result_position);
status_t sp_file_position_set(sp_file_t* file, sp_count_t sample_offset);
status_t sp_file_open(uint8_t* path, int mode, sp_channel_count_t channel_count, sp_sample_rate_t sample_rate, sp_file_t* result_file);
status_t sp_file_close(sp_file_t* a);
status_t sp_block_new(sp_channel_count_t channel_count, sp_count_t sample_count, sp_block_t* out_block);
uint8_t* sp_status_description(status_t a);
uint8_t* sp_status_name(status_t a);
sp_sample_t sp_sin_lq(sp_float_t a);
sp_float_t sp_sinc(sp_float_t a);
sp_float_t sp_window_blackman(sp_float_t a, sp_count_t width);
void sp_spectral_inversion_ir(sp_sample_t* a, sp_count_t a_len);
void sp_spectral_reversal_ir(sp_sample_t* a, sp_count_t a_len);
status_id_t sp_fft(sp_count_t input_len, double* input_or_output_real, double* input_or_output_imag);
status_id_t sp_ffti(sp_count_t input_len, double* input_or_output_real, double* input_or_output_imag);
void sp_convolve_one(sp_sample_t* a, sp_count_t a_len, sp_sample_t* b, sp_count_t b_len, sp_sample_t* result_samples);
void sp_convolve(sp_sample_t* a, sp_count_t a_len, sp_sample_t* b, sp_count_t b_len, sp_count_t result_carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples);
void sp_block_free(sp_block_t a);
sp_block_t sp_block_with_offset(sp_block_t a, sp_count_t offset);
status_t sp_null_ir(sp_sample_t** out_ir, sp_count_t* out_len);
status_t sp_passthrough_ir(sp_sample_t** out_ir, sp_count_t* out_len);
status_t sp_initialise(uint16_t cpu_count);
void sp_random_samples(sp_random_state_t* state, sp_count_t size, sp_sample_t* out);
status_t sp_samples_new(sp_count_t size, sp_sample_t** out);
status_t sp_counts_new(sp_count_t size, sp_count_t** out);
/* filter */
#define sp_filter_state_t sp_convolution_filter_state_t
#define sp_filter_state_free sp_convolution_filter_state_free
#define sp_cheap_filter_passes_limit 8
#define sp_cheap_filter_lp(...) sp_cheap_filter(sp_state_variable_filter_lp, __VA_ARGS__)
#define sp_cheap_filter_hp(...) sp_cheap_filter(sp_state_variable_filter_hp, __VA_ARGS__)
#define sp_cheap_filter_bp(...) sp_cheap_filter(sp_state_variable_filter_bp, __VA_ARGS__)
#define sp_cheap_filter_br(...) sp_cheap_filter(sp_state_variable_filter_br, __VA_ARGS__)
typedef status_t (*sp_convolution_filter_ir_f_t)(void*, sp_sample_t**, sp_count_t*);
typedef struct {
  sp_sample_t* carryover;
  sp_count_t carryover_len;
  sp_count_t carryover_alloc_len;
  sp_sample_t* ir;
  sp_convolution_filter_ir_f_t ir_f;
  void* ir_f_arguments;
  uint8_t ir_f_arguments_len;
  sp_count_t ir_len;
} sp_convolution_filter_state_t;
typedef struct {
  sp_sample_t* in_temp;
  sp_sample_t* out_temp;
  sp_sample_t svf_state[(2 * sp_cheap_filter_passes_limit)];
} sp_cheap_filter_state_t;
typedef void (*sp_state_variable_filter_t)(sp_sample_t*, sp_sample_t*, sp_float_t, sp_float_t, sp_count_t, sp_sample_t*);
status_t sp_moving_average(sp_sample_t* in, sp_sample_t* in_end, sp_sample_t* in_window, sp_sample_t* in_window_end, sp_sample_t* prev, sp_sample_t* prev_end, sp_sample_t* next, sp_sample_t* next_end, sp_count_t radius, sp_sample_t* out);
sp_count_t sp_windowed_sinc_lp_hp_ir_length(sp_float_t transition);
status_t sp_windowed_sinc_ir(sp_float_t cutoff, sp_float_t transition, sp_count_t* result_len, sp_sample_t** result_ir);
void sp_convolution_filter_state_free(sp_convolution_filter_state_t* state);
status_t sp_convolution_filter_state_set(sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state);
status_t sp_convolution_filter(sp_sample_t* in, sp_count_t in_len, sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples);
status_t sp_windowed_sinc_lp_hp_ir(sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_sample_t** out_ir, sp_count_t* out_len);
status_t sp_windowed_sinc_bp_br_ir(sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_sample_t** out_ir, sp_count_t* out_len);
status_t sp_windowed_sinc_lp_hp_ir_f(void* arguments, sp_sample_t** out_ir, sp_count_t* out_len);
status_t sp_windowed_sinc_bp_br_ir_f(void* arguments, sp_sample_t** out_ir, sp_count_t* out_len);
status_t sp_windowed_sinc_lp_hp(sp_sample_t* in, sp_count_t in_len, sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples);
status_t sp_windowed_sinc_bp_br(sp_sample_t* in, sp_count_t in_len, sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples);
status_t sp_windowed_sinc_lp_hp_ir(sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_sample_t** out_ir, sp_count_t* out_len);
void sp_state_variable_filter_lp(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_hp(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_bp(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_br(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_peak(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_all(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_cheap_filter(sp_state_variable_filter_t type, sp_sample_t* in, sp_count_t in_size, sp_float_t cutoff, sp_count_t passes, sp_float_t q_factor, uint8_t unity_gain, sp_cheap_filter_state_t* state, sp_sample_t* out);
void sp_cheap_filter_state_free(sp_cheap_filter_state_t* a);
status_t sp_cheap_filter_state_new(sp_count_t max_size, sp_count_t max_passes, sp_cheap_filter_state_t* out_state);
status_t sp_filter(sp_sample_t* in, sp_count_t in_size, sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_filter_state_t** out_state, sp_sample_t* out_samples);
/* plot */
void sp_plot_samples(sp_sample_t* a, sp_count_t a_size);
void sp_plot_counts(sp_count_t* a, sp_count_t a_size);
void sp_plot_samples_to_file(sp_sample_t* a, sp_count_t a_size, uint8_t* path);
void sp_plot_counts_to_file(sp_count_t* a, sp_count_t a_size, uint8_t* path);
void sp_plot_samples_file(uint8_t* path, uint8_t use_steps);
void sp_plot_spectrum_to_file(sp_sample_t* a, sp_count_t a_size, uint8_t* path);
void sp_plot_spectrum_file(uint8_t* path);
void sp_plot_spectrum(sp_sample_t* a, sp_count_t a_size);
/* synthesiser */
/** t must be between 0 and 95999 */
#define sp_sine_96(t) sp_sine_96_table[t]
typedef struct {
  sp_count_t start;
  sp_count_t end;
  sp_synth_count_t modifies;
  sp_sample_t* amp[sp_channel_limit];
  sp_count_t* wvl[sp_channel_limit];
  sp_count_t phs[sp_channel_limit];
} sp_synth_partial_t;
sp_sample_t* sp_sine_96_table;
status_t sp_sine_table_new(sp_sample_t** out, sp_count_t size);
sp_count_t sp_phase_96(sp_count_t current, sp_count_t change);
sp_count_t sp_phase_96_float(sp_count_t current, double change);
status_t sp_synth(sp_block_t out, sp_count_t start, sp_count_t duration, sp_synth_count_t config_len, sp_synth_partial_t* config, sp_count_t* phases);
status_t sp_synth_state_new(sp_count_t channel_count, sp_synth_count_t config_len, sp_synth_partial_t* config, sp_count_t** out_state);
sp_synth_partial_t sp_synth_partial_1(sp_count_t start, sp_count_t end, sp_synth_count_t modifies, sp_sample_t* amp, sp_count_t* wvl, sp_count_t phs);
sp_synth_partial_t sp_synth_partial_2(sp_count_t start, sp_count_t end, sp_synth_count_t modifies, sp_sample_t* amp1, sp_sample_t* amp2, sp_count_t* wvl1, sp_count_t* wvl2, sp_count_t phs1, sp_count_t phs2);
sp_sample_t sp_square_96(sp_count_t t);
sp_sample_t sp_triangle(sp_count_t t, sp_count_t a, sp_count_t b);
sp_sample_t sp_triangle_96(sp_count_t t);
/* sequencer */
#define sp_cheap_noise_event_lp(start, end, amp, ...) sp_cheap_noise_event(start, end, amp, sp_state_variable_filter_lp, __VA_ARGS__)
#define sp_cheap_noise_event_hp(start, end, amp, ...) sp_cheap_noise_event(start, end, amp, sp_state_variable_filter_hp, __VA_ARGS__)
#define sp_cheap_noise_event_bp(start, end, amp, ...) sp_cheap_noise_event(start, end, amp, sp_state_variable_filter_bp, __VA_ARGS__)
#define sp_cheap_noise_event_br(start, end, amp, ...) sp_cheap_noise_event(start, end, amp, sp_state_variable_filter_br, __VA_ARGS__)
struct sp_event_t;
typedef struct sp_event_t {
  void* state;
  sp_count_t start;
  sp_count_t end;
  void (*f)(sp_count_t, sp_count_t, sp_block_t, struct sp_event_t*);
  void (*free)(struct sp_event_t*);
} sp_event_t;
typedef void (*sp_event_f_t)(sp_count_t, sp_count_t, sp_block_t, sp_event_t*);
typedef struct {
  sp_count_t size;
  sp_event_t* data;
} sp_events_t;
typedef struct {
  sp_synth_count_t config_len;
  sp_synth_partial_t config[sp_synth_partial_limit];
  sp_count_t* state;
} sp_synth_event_state_t;
void sp_seq_events_prepare(sp_events_t a);
void sp_seq(sp_count_t start, sp_count_t end, sp_block_t out, sp_events_t events);
status_t sp_seq_parallel(sp_count_t start, sp_count_t end, sp_block_t out, sp_events_t events);
status_t sp_synth_event(sp_count_t start, sp_count_t end, sp_count_t channel_count, sp_count_t config_len, sp_synth_partial_t* config, sp_event_t* out_event);
status_t sp_noise_event(sp_count_t start, sp_count_t end, sp_sample_t** amp, sp_sample_t* cut_l, sp_sample_t* cut_h, sp_sample_t* trn_l, sp_sample_t* trn_h, uint8_t is_reject, sp_count_t resolution, sp_random_state_t random_state, sp_event_t* out_event);
void sp_events_free(sp_events_t events);
status_t sp_cheap_noise_event(sp_count_t start, sp_count_t end, sp_sample_t** amp, sp_state_variable_filter_t type, sp_sample_t* cut, sp_count_t passes, sp_sample_t q_factor, sp_count_t resolution, sp_random_state_t random_state, sp_event_t* out_event);
void sp_counts_from_samples(sp_sample_t* in, sp_count_t in_size, sp_count_t* out) {
  sp_count_t i;
  for (i = 0; (i < in_size); i = (1 + i)) {
    out[i] = sp_cheap_round_positive((in[i]));
  };
};
status_id_t spline_path_new_get_4(sp_sample_t* out, sp_count_t duration, spline_path_segment_t s1, spline_path_segment_t s2, spline_path_segment_t s3, spline_path_segment_t s4) {
  spline_path_segment_t segments[4];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  segments[3] = s4;
  return ((spline_path_new_get(4, segments, 0, duration, out)));
};
status_id_t spline_path_new_get_2(sp_sample_t* out, sp_count_t duration, spline_path_segment_t s1, spline_path_segment_t s2) {
  spline_path_segment_t segments[2];
  segments[0] = s1;
  segments[1] = s2;
  return ((spline_path_new_get(2, segments, 0, duration, out)));
};
status_t sp_block_to_file(sp_block_t block, uint8_t* path, sp_count_t rate) {
  status_declare;
  sp_file_t file;
  sp_count_t written;
  status_require((sp_file_open(path, sp_file_mode_write, (block.channels), rate, (&file))));
  status_require((sp_file_write((&file), (block.samples), (block.size), (&written))));
  sp_file_close((&file));
exit:
  return (status);
};
#define sp_samples_zero(a, size) memset(a, 0, (size * sizeof(sp_sample_t)))
