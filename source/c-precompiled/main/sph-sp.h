#include <byteswap.h>
#include <math.h>
#include <inttypes.h>
#ifndef sp_config_is_set
#define sp_channel_count_t uint8_t
#define sp_file_format (SF_FORMAT_WAV | SF_FORMAT_FLOAT)
#define sp_float_t double
#define sp_count_t uint32_t
#define sp_sample_count_max UINT32_MAX
#define sp_sample_rate_t uint32_t
#define sp_sample_sum f64_sum
#define sp_sample_t double
#define sp_sf_read sf_readf_double
#define sp_sf_write sf_writef_double
#define sp_synth_count_t uint16_t
#define sp_synth_sine sp_sine_96
#define sp_synth_channel_limit 16
#define sp_synth_partial_limit 64
#define sp_config_is_set 1
#endif
#define boolean uint8_t
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
/** sample count to bit octets count */
#define sp_octets_to_samples(a) (a / sizeof(sp_sample_t))
#define sp_samples_to_octets(a) (a * sizeof(sp_sample_t))
/** t must be between 0 and 95999 */
#define sp_sine_96(t) sp_sine_96_table[t]
#define sp_cheap_round_positive(a) ((sp_count_t)((0.5 + a)))
#define sp_cheap_floor_positive(a) ((sp_count_t)(a))
#define sp_cheap_ceiling_positive(a) (((sp_count_t)(a)) + (((sp_count_t)(a)) < a))
#define sp_block_set_null(a) a.channels = 0
#include <stdio.h>
/** writes values with current routine name and line info to standard output.
    example: (debug-log "%d" 1)
    otherwise like printf */
#define debug_log(format, ...) fprintf(stdout, "%s:%d " format "\n", __func__, __LINE__, __VA_ARGS__)
/** display current function name and given number.
    example call: (debug-trace 1) */
#define debug_trace(n) fprintf(stdout, "%s %d\n", __func__, n)
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
/* "iteration array" - an array with variable length content that makes iteration easier to code.
  saves the size argument that usually has to be passed with arrays and saves the declaration of index counter variables.
  the data structure consists of only 4 pointers in a struct.
  most bindings are generic macros that will work on any i-array type. i-array-add and i-array-forward go from left to right.
  examples:
    i_array_declare_type(my_type, int);
    my_type a;
    if(i_array_allocate_my_type(4, &a)) {
      // memory allocation error
    }
    i_array_add(a, 1);
    i_array_add(a, 2);
    while(i_array_in_range(a)) {
      i_array_get(a);
      i_array_forward(a);
    }
    i_array_free(a); */
#include <stdlib.h>
/** .current: to avoid having to write for-loops. this would correspond to the index variable in loops
     .unused: to have variable length content in a fixed length array. points outside the memory area after the last element has been added
     .end: start + max-length. (last-index + 1) of the allocated array
     .start: the beginning of the allocated array and used for rewind and free */
#define i_array_declare_type(name, element_type) \
  typedef struct { \
    element_type* current; \
    element_type* unused; \
    element_type* end; \
    element_type* start; \
  } name; \
  uint8_t i_array_allocate_custom_##name(size_t length, void* (*alloc)(size_t), name* a) { \
    element_type* start; \
    start = alloc((length * sizeof(element_type))); \
    if (!start) { \
      return (1); \
    }; \
    a->start = start; \
    a->current = start; \
    a->unused = start; \
    a->end = (length + start); \
    return (0); \
  }; \
  uint8_t i_array_allocate_##name(size_t length, name* a) { return ((i_array_allocate_custom_##name(length, malloc, a))); }
/** define so that in-range is false, length is zero and free doesnt fail */
#define i_array_declare(a, type) type a = { 0, 0, 0, 0 }
#define i_array_add(a, value) \
  *(a.unused) = value; \
  a.unused = (1 + a.unused)
/** set so that in-range is false, length is zero and free doesnt fail */
#define i_array_set_null(a) \
  a.start = 0; \
  a.unused = 0
#define i_array_in_range(a) (a.current < a.unused)
#define i_array_get_at(a, index) (a.start)[index]
#define i_array_get(a) *(a.current)
#define i_array_forward(a) a.current = (1 + a.current)
#define i_array_rewind(a) a.current = a.start
#define i_array_clear(a) a.unused = a.start
#define i_array_remove(a) a.unused = (a.unused - 1)
#define i_array_length(a) (a.unused - a.start)
#define i_array_max_length(a) (a.end - a.start)
#define i_array_free(a) free((a.start))
/** create an i-array from a standard array.
     sets source as data array to use, with the first count number of slots used.
     source will not be copied but used as is, and will i-array-free frees is.
     # example
     int other_array[4] = {1, 2, 0, 0};
     my_type a;
     i_array_take(a, other_array, 4 2); */
#define i_array_take(a, source, size, count) \
  a->start = source; \
  a->current = source; \
  a->unused = (count + source); \
  a->end = (size + source)
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
#define sp_channel_limit 2
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
  sp_count_t start;
  sp_count_t end;
  sp_synth_count_t modifies;
  sp_sample_t* amp[sp_synth_channel_limit];
  sp_count_t* wvl[sp_synth_channel_limit];
  sp_count_t phs[sp_synth_channel_limit];
} sp_synth_partial_t;
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
sp_float_t sp_window_blackman(sp_float_t a, sp_count_t width);
void sp_spectral_inversion_ir(sp_sample_t* a, sp_count_t a_len);
void sp_spectral_reversal_ir(sp_sample_t* a, sp_count_t a_len);
status_t sp_fft(sp_count_t input_len, double* input_or_output_real, double* input_or_output_imag);
status_t sp_ffti(sp_count_t input_len, double* input_or_output_real, double* input_or_output_imag);
status_t sp_moving_average(sp_sample_t* in, sp_sample_t* in_end, sp_sample_t* in_window, sp_sample_t* in_window_end, sp_sample_t* prev, sp_sample_t* prev_end, sp_sample_t* next, sp_sample_t* next_end, sp_count_t radius, sp_sample_t* out);
void sp_convolve_one(sp_sample_t* a, sp_count_t a_len, sp_sample_t* b, sp_count_t b_len, sp_sample_t* result_samples);
void sp_convolve(sp_sample_t* a, sp_count_t a_len, sp_sample_t* b, sp_count_t b_len, sp_count_t result_carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples);
void sp_block_free(sp_block_t a);
status_t sp_windowed_sinc_lp_hp_ir(sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_sample_t** out_ir, sp_count_t* out_len);
status_t sp_null_ir(sp_sample_t** out_ir, sp_count_t* out_len);
status_t sp_passthrough_ir(sp_sample_t** out_ir, sp_count_t* out_len);
status_t sp_sine_table_new(sp_sample_t** out, sp_count_t size);
sp_sample_t* sp_sine_96_table;
status_t sp_initialise();
sp_count_t sp_cheap_phase_96(sp_count_t current, sp_count_t change);
sp_count_t sp_cheap_phase_96_float(sp_count_t current, double change);
status_t sp_synth(sp_block_t out, sp_count_t start, sp_count_t duration, sp_synth_count_t config_len, sp_synth_partial_t* config, sp_count_t* phases);
status_t sp_synth_state_new(sp_count_t channel_count, sp_synth_count_t config_len, sp_synth_partial_t* config, sp_count_t** out_state);
void sp_state_variable_filter_lp(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_hp(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_bp(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_br(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_peak(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
void sp_state_variable_filter_all(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state);
#define sp_events_new i_array_allocate_sp_events_t
struct sp_event_t;
typedef struct sp_event_t {
  void* state;
  sp_count_t start;
  sp_count_t end;
  void (*f)(sp_count_t, sp_count_t, sp_block_t, struct sp_event_t*);
} sp_event_t;
typedef void (*sp_event_f_t)(sp_count_t, sp_count_t, sp_block_t, sp_event_t*);
typedef struct {
  sp_synth_count_t config_len;
  sp_synth_partial_t config[sp_synth_partial_limit];
  sp_count_t* state;
} sp_synth_event_state_t;
i_array_declare_type(sp_events_t, sp_event_t);
void sp_seq_events_prepare(sp_events_t a);
void sp_seq(sp_count_t time, sp_count_t offset, sp_count_t size, sp_block_t output, sp_events_t events);
status_t sp_synth_event(sp_count_t start, sp_count_t end, sp_count_t channel_count, sp_count_t config_len, sp_synth_partial_t* config, sp_event_t* out_event);
void sp_plot_samples(sp_sample_t* a);