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
#ifndef sp_channels_t
#define sp_channels_t uint8_t
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
#define sp_time_t uint64_t
#endif
#include <sph/status.c>
#include <sph/spline-path.h>
#include <sph/random.c>
#include <sph/array3.c>
#include <sph/array4.c>
#include <sph/memreg-heap.c>
#include <sph/float.c>
/* main */
#define boolean uint8_t
#define f64 double
#define sp_file_bit_input 1
#define sp_file_bit_output 2
#define sp_file_bit_position 4
#define sp_file_mode_read 1
#define sp_file_mode_write 2
#define sp_file_mode_read_write 3
#define sp_s_group_libc "libc"
#define sp_s_group_sndfile "sndfile"
#define sp_s_group_sp "sp"
#define sp_s_group_sph "sph"
#define sp_random_state_t sph_random_state_t
#define sp_random_state_new sph_random_state_new
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
#define sp_cheap_round_positive(a) ((sp_time_t)((0.5 + a)))
#define sp_cheap_floor_positive(a) ((sp_time_t)(a))
#define sp_cheap_ceiling_positive(a) (((sp_time_t)(a)) + (((sp_time_t)(a)) < a))
/** t must be between 0 and 95999 */
#define sp_sine_96(t) sp_sine_96_table[t]
#define sp_sine_96_state_1(spd, amp, phs) sp_wave_state_1(sp_sine_96_table, 96000, amp, phs)
#define sp_sine_96_state_2(spd, amp1, amp2, phs1, phs2) sp_wave_state_2(sp_sine_96_table, 96000, amp1, amp2, phs1, phs2)
typedef struct {
  sp_channels_t channels;
  sp_time_t size;
  sp_sample_t* samples[sp_channel_limit];
} sp_block_t;
typedef struct {
  uint8_t flags;
  sp_sample_rate_t sample_rate;
  sp_channels_t channel_count;
  void* data;
} sp_file_t;
uint32_t sp_cpu_count;
sp_random_state_t sp_default_random_state;
void sp_block_zero(sp_block_t a);
status_t sp_file_read(sp_file_t* file, sp_time_t sample_count, sp_sample_t** result_block, sp_time_t* result_sample_count);
status_t sp_file_write(sp_file_t* file, sp_sample_t** block, sp_time_t sample_count, sp_time_t* result_sample_count);
status_t sp_file_position(sp_file_t* file, sp_time_t* result_position);
status_t sp_file_position_set(sp_file_t* file, sp_time_t sample_offset);
status_t sp_file_open(uint8_t* path, int mode, sp_channels_t channel_count, sp_sample_rate_t sample_rate, sp_file_t* result_file);
status_t sp_file_close(sp_file_t* a);
status_t sp_block_to_file(sp_block_t block, uint8_t* path, sp_time_t rate);
status_t sp_block_new(sp_channels_t channel_count, sp_time_t sample_count, sp_block_t* out_block);
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
status_t sp_initialise(uint16_t cpu_count);
typedef struct {
  sp_sample_t* amp[sp_channel_limit];
  sp_time_t phs[sp_channel_limit];
  sp_time_t* spd;
  sp_time_t wvf_size;
  sp_sample_t* wvf;
} sp_wave_state_t;
sp_sample_t* sp_sine_96_table;
status_t sp_sine_table_new(sp_sample_t** out, sp_time_t size);
sp_time_t sp_phase(sp_time_t current, sp_time_t change, sp_time_t cycle);
sp_time_t sp_phase_float(sp_time_t current, double change, sp_time_t cycle);
sp_sample_t sp_square_96(sp_time_t t);
sp_sample_t sp_triangle(sp_time_t t, sp_time_t a, sp_time_t b);
sp_sample_t sp_triangle_96(sp_time_t t);
void sp_wave(sp_time_t start, sp_time_t duration, sp_wave_state_t* state, sp_block_t out);
sp_wave_state_t sp_wave_state_1(sp_sample_t* wvf, sp_time_t wvf_size, sp_time_t* spd, sp_sample_t* amp, sp_time_t phs);
sp_wave_state_t sp_wave_state_2(sp_sample_t* wvf, sp_time_t wvf_size, sp_time_t* spd, sp_sample_t* amp1, sp_sample_t* amp2, sp_time_t phs1, sp_time_t phs2);
/* arrays */
#define sp_samples_zero(a, size) memset(a, 0, (size * sizeof(sp_sample_t)))
#define sp_times_zero(a, size) memset(a, 0, (size * sizeof(sp_time_t)))
void sp_samples_set_unity_gain(sp_sample_t* in, sp_time_t in_size, sp_sample_t* out);
void sp_samples_random(sp_random_state_t* state, sp_time_t size, sp_sample_t* out);
void sp_times_random(sp_random_state_t* state, sp_time_t size, sp_time_t* out);
status_t sp_times_new(sp_time_t size, sp_time_t** out);
void sp_samples_to_times(sp_sample_t* in, sp_time_t in_size, sp_time_t* out);
void sp_samples_display(sp_sample_t* a, sp_time_t size);
status_t sp_samples_new(sp_time_t size, sp_sample_t** out);
/* filter */
#define sp_filter_state_t sp_convolution_filter_state_t
#define sp_filter_state_free sp_convolution_filter_state_free
#define sp_cheap_filter_passes_limit 8
#define sp_cheap_filter_lp(...) sp_cheap_filter(sp_state_variable_filter_lp, __VA_ARGS__)
#define sp_cheap_filter_hp(...) sp_cheap_filter(sp_state_variable_filter_hp, __VA_ARGS__)
#define sp_cheap_filter_bp(...) sp_cheap_filter(sp_state_variable_filter_bp, __VA_ARGS__)
#define sp_cheap_filter_br(...) sp_cheap_filter(sp_state_variable_filter_br, __VA_ARGS__)
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
status_t sp_moving_average(sp_sample_t* in, sp_sample_t* in_end, sp_sample_t* in_window, sp_sample_t* in_window_end, sp_sample_t* prev, sp_sample_t* prev_end, sp_sample_t* next, sp_sample_t* next_end, sp_time_t radius, sp_sample_t* out);
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
void sp_cheap_filter(sp_state_variable_filter_t type, sp_sample_t* in, sp_time_t in_size, sp_float_t cutoff, sp_time_t passes, sp_float_t q_factor, uint8_t unity_gain, sp_cheap_filter_state_t* state, sp_sample_t* out);
void sp_cheap_filter_state_free(sp_cheap_filter_state_t* a);
status_t sp_cheap_filter_state_new(sp_time_t max_size, sp_time_t max_passes, sp_cheap_filter_state_t* out_state);
status_t sp_filter(sp_sample_t* in, sp_time_t in_size, sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_filter_state_t** out_state, sp_sample_t* out_samples);
/* plot */
void sp_plot_samples(sp_sample_t* a, sp_time_t a_size);
void sp_plot_times(sp_time_t* a, sp_time_t a_size);
void sp_plot_samples_to_file(sp_sample_t* a, sp_time_t a_size, uint8_t* path);
void sp_plot_times_to_file(sp_time_t* a, sp_time_t a_size, uint8_t* path);
void sp_plot_samples_file(uint8_t* path, uint8_t use_steps);
void sp_plot_spectrum_to_file(sp_sample_t* a, sp_time_t a_size, uint8_t* path);
void sp_plot_spectrum_file(uint8_t* path);
void sp_plot_spectrum(sp_sample_t* a, sp_time_t a_size);
/* sequencer */
#define sp_event_duration(a) (a.end - a.start)
#define sp_event_duration_set(a, duration) a.end = (a.start + duration)
#define sp_event_move(a, start) \
  a.end = (start + (a.end - a.start)); \
  a.start = start
#define sp_cheap_noise_event_lp(start, end, amp, ...) sp_cheap_noise_event(start, end, amp, sp_state_variable_filter_lp, __VA_ARGS__)
#define sp_cheap_noise_event_hp(start, end, amp, ...) sp_cheap_noise_event(start, end, amp, sp_state_variable_filter_hp, __VA_ARGS__)
#define sp_cheap_noise_event_bp(start, end, amp, ...) sp_cheap_noise_event(start, end, amp, sp_state_variable_filter_bp, __VA_ARGS__)
#define sp_cheap_noise_event_br(start, end, amp, ...) sp_cheap_noise_event(start, end, amp, sp_state_variable_filter_br, __VA_ARGS__)
#define sp_group_size_t uint32_t
#define sp_group_events(a) ((sp_group_event_state_t*)(a.state))->events
#define sp_group_memory(a) ((sp_group_event_state_t*)(a.state))->memory
#define sp_group_memory_add(a, pointer) \
  if (array4_not_full((sp_group_memory(a)))) { \
    array4_add((sp_group_memory(a)), pointer); \
  }
#define sp_group_add(a, event) \
  if (array4_not_full((sp_group_events(a)))) { \
    array4_add((sp_group_events(a)), event); \
    if (a.end < event.end) { \
      a.end = event.end; \
    }; \
  }
#define sp_group_prepare(a) sp_seq_events_prepare(((sp_group_events(a)).data), (array4_size((sp_group_events(a)))))
#define sp_event_declare(variable) \
  sp_event_t variable; \
  variable.state = 0
#define sp_group_declare sp_event_declare
#define sp_group_free(a) \
  if (a.state) { \
    (a.free)((&a)); \
  }
struct sp_event_t;
typedef struct sp_event_t {
  void* state;
  sp_time_t start;
  sp_time_t end;
  void (*f)(sp_time_t, sp_time_t, sp_block_t, struct sp_event_t*);
  void (*free)(struct sp_event_t*);
} sp_event_t;
typedef void (*sp_event_f_t)(sp_time_t, sp_time_t, sp_block_t, sp_event_t*);
void sp_seq_events_prepare(sp_event_t* data, sp_time_t size);
void sp_seq(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* events, sp_time_t size);
status_t sp_seq_parallel(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* events, sp_time_t size);
status_t sp_noise_event(sp_time_t start, sp_time_t end, sp_sample_t** amp, sp_sample_t* cut_l, sp_sample_t* cut_h, sp_sample_t* trn_l, sp_sample_t* trn_h, uint8_t is_reject, sp_time_t resolution, sp_random_state_t random_state, sp_event_t* out_event);
void sp_events_array_free(sp_event_t* events, sp_time_t size);
void sp_wave_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event);
status_t sp_wave_event(sp_time_t start, sp_time_t end, sp_wave_state_t state, sp_event_t* out);
status_t sp_cheap_noise_event(sp_time_t start, sp_time_t end, sp_sample_t** amp, sp_state_variable_filter_t type, sp_sample_t* cut, sp_time_t passes, sp_sample_t q_factor, sp_time_t resolution, sp_random_state_t random_state, sp_event_t* out_event);
array4_declare_type(sp_events, sp_event_t);
typedef struct {
  sp_events_t events;
  memreg_register_t memory;
} sp_group_event_state_t;
status_t sp_group_new(sp_time_t start, sp_group_size_t event_size, sp_group_size_t memory_size, sp_event_t* out);
void sp_group_append(sp_event_t* a, sp_event_t event);
void sp_group_event_f(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event);
void sp_group_event_free(sp_event_t* a);
/* path */
#define path_move spline_path_move
#define sp_path_t spline_path_t
#define sp_path_segment_t spline_path_segment_t
#define sp_path_segment_count_t spline_path_segment_count_t
#define sp_path_line spline_path_line
#define sp_path_move spline_path_move
#define sp_path_bezier spline_path_bezier
#define sp_path_constant spline_path_constant
#define sp_path_path spline_path_path
#define sp_path_i_line spline_path_i_line
#define sp_path_i_move spline_path_i_move
#define sp_path_i_bezier spline_path_i_bezier
#define sp_path_i_constant spline_path_i_constant
#define sp_path_i_path spline_path_i_path
array3_declare_type(sp_path_segments, spline_path_segment_t);
status_t sp_path_samples(sp_path_segments_t segments, sp_time_t size, sp_sample_t** out);
status_t sp_path_samples_1(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1);
status_t sp_path_samples_2(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2);
status_t sp_path_samples_3(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3);
status_t sp_path_samples_4(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4);
status_t sp_path_times(sp_path_segments_t segments, sp_time_t size, sp_time_t** out);
status_t sp_path_times_1(sp_time_t** out, sp_time_t size, sp_path_segment_t s1);
status_t sp_path_times_2(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2);
status_t sp_path_times_3(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3);
status_t sp_path_times_4(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4);
/* main 2 */
#define declare_render_config(name) sp_render_config_t name = { .channels = 2, .rate = 96000, .block_size = 96000 }
/** returns n/d fractions of the sample rate. for example, 1/2 is half the sample rate.
     this macro references a local variable named rate which must exist and contain the current sample rate */
#define rt(n, d) ((sp_time_t)(((rate / d) * n)))
typedef struct {
  sp_time_t rate;
  sp_time_t block_size;
  sp_channels_t channels;
} sp_render_config_t;
status_t sp_render_file(sp_event_t event, sp_time_t start, sp_time_t duration, sp_render_config_t config, uint8_t* path);