#include <byteswap.h>
#include <math.h>
#include <inttypes.h>
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
#ifndef sp_sample_format
#define sp_channel_count_t uint32_t
#define sp_default_alsa_enable_soft_resample 1
#define sp_default_alsa_latency 128
#define sp_default_channel_count 1
#define sp_default_sample_rate 16000
#define sp_file_format (SF_FORMAT_WAV | SF_FORMAT_FLOAT)
#define sp_float_t double
#define sp_sample_count_t size_t
#define sp_sample_format sp_sample_format_f32
#define sp_sample_rate_t uint32_t
#endif
#define f32 float
#define f64 double
#define boolean uint8_t
#define sp_port_type_alsa 0
#define sp_port_type_file 1
#define sp_port_bit_input 1
#define sp_port_bit_output 2
#define sp_port_bit_position 4
#define sp_port_bit_closed 8
#define sp_port_mode_read 1
#define sp_port_mode_write 2
#define sp_port_mode_read_write 3
#define sp_sample_format_f64 1
#define sp_sample_format_f32 2
#define sp_sample_format_int32 3
#define sp_sample_format_int16 4
#define sp_sample_format_int8 5
#define sp_status_group_libc "libc"
#define sp_status_group_sndfile "sndfile"
#define sp_status_group_sp "sp"
#define sp_status_group_sph "sph"
#define sp_status_group_alsa "alsa"
/** sample count to bit octets count */
#define sp_octets_to_samples(a) (a / sizeof(sp_sample_t))
#define sp_samples_to_octets(a) (a * sizeof(sp_sample_t))
#define duration_to_sample_count(seconds, sample_rate) (seconds * sample_rate)
#define sample_count_to_duration(sample_count, sample_rate) (sample_count / sample_rate)
/** sp-float-t integer -> sp-float-t
    radians-per-second samples-per-second -> cutoff-value */
#define sp_windowed_sinc_cutoff(freq, sample_rate) ((2 * M_PI * freq) / sample_rate)
#if (sp_sample_format_f64 == sp_sample_format)
#define sp_sample_t double
#define sp_sample_sum f64_sum
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_FLOAT64
#define sp_sf_write sf_writef_double
#define sp_sf_read sf_readf_double
#define kiss_fft_scalar sp_sample_t
#elif (sp_sample_format_f32 == sp_sample_format)
#define sp_sample_t float
#define sp_sample_sum f32_sum
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_FLOAT
#define sp_sf_write sf_writef_float
#define sp_sf_read sf_readf_float
#define kiss_fft_scalar sp_sample_t
#elif (sp_sample_format_int32 == sp_sample_format)
#define sp_sample_t int32_t
#define sp_sample_sum(a, b) (a + b)
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_S32
#define sp_sf_write sf_writef_int
#define sp_sf_read sf_readf_int
#define FIXED_POINT 32
#elif (sp_sample_format_int16 == sp_sample_format)
#define sp_sample_t int16_t
#define sp_sample_sum(a, b) (a + b)
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_S16
#define sp_sf_write sf_writef_short
#define sp_sf_read sf_readf_short
#define FIXED_POINT 16
#elif (sp_sample_format_int8 == sp_sample_format)
#define sp_sample_t int8_t
#define sp_sample_sum(a, b) (a + b)
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_S8
#define sp_sf_write sf_writef_short
#define sp_sf_read sf_readf_short
#define FIXED_POINT 8
#endif
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
  sp_status_id_port_closed,
  sp_status_id_port_position,
  sp_status_id_port_type,
  sp_status_id_undefined };
typedef struct {
  uint8_t type;
  uint8_t flags;
  sp_sample_rate_t sample_rate;
  sp_channel_count_t channel_count;
  void* data;
} sp_port_t;
typedef struct {
  sp_sample_t* carryover;
  sp_sample_count_t carryover_len;
  sp_sample_count_t carryover_alloc_len;
  sp_float_t freq;
  sp_sample_t* ir;
  sp_sample_count_t ir_len;
  sp_sample_rate_t sample_rate;
  sp_float_t transition;
} sp_windowed_sinc_state_t;
status_t sp_port_read(sp_port_t* port, sp_sample_count_t sample_count, sp_sample_t** result_channel_data, sp_sample_count_t* result_sample_count);
status_t sp_port_write(sp_port_t* port, sp_sample_t** channel_data, sp_sample_count_t sample_count, sp_sample_count_t* result_sample_count);
status_t sp_port_position(sp_port_t* port, sp_sample_count_t* result_position);
status_t sp_port_position_set(sp_port_t* port, size_t sample_offset);
status_t sp_file_open(uint8_t* path, int mode, sp_channel_count_t channel_count, sp_sample_rate_t sample_rate, sp_port_t* result_port);
status_t sp_alsa_open(uint8_t* device_name, int mode, sp_channel_count_t channel_count, sp_sample_rate_t sample_rate, int32_t latency, sp_port_t* result_port);
status_t sp_port_close(sp_port_t* a);
status_t sp_alloc_channel_array(sp_channel_count_t channel_count, sp_sample_count_t sample_count, sp_sample_t*** result_array);
uint8_t* sp_status_description(status_t a);
uint8_t* sp_status_name(status_t a);
void sp_sine(sp_sample_count_t len, sp_float_t sample_duration, sp_float_t freq, sp_float_t phase, sp_float_t amp, sp_sample_t* result_samples);
void sp_sine_lq(sp_sample_count_t len, sp_float_t sample_duration, sp_float_t freq, sp_float_t phase, sp_float_t amp, sp_sample_t* result_samples);
sp_float_t sp_sinc(sp_float_t a);
sp_sample_count_t sp_windowed_sinc_ir_length(sp_float_t transition);
status_t sp_windowed_sinc_ir(sp_sample_rate_t sample_rate, sp_float_t freq, sp_float_t transition, sp_sample_count_t* result_len, sp_sample_t** result_ir);
void sp_windowed_sinc_state_free(sp_windowed_sinc_state_t* state);
status_t sp_windowed_sinc_state_create(sp_sample_rate_t sample_rate, sp_float_t freq, sp_float_t transition, sp_windowed_sinc_state_t** result_state);
status_t sp_windowed_sinc(sp_sample_t* source, sp_sample_count_t source_len, sp_sample_rate_t sample_rate, sp_float_t freq, sp_float_t transition, sp_windowed_sinc_state_t** result_state, sp_sample_t* result_samples);
sp_float_t sp_window_blackman(sp_float_t a, sp_sample_count_t width);
void sp_spectral_inversion_ir(sp_sample_t* a, sp_sample_count_t a_len);
void sp_spectral_reversal_ir(sp_sample_t* a, sp_sample_count_t a_len);
status_t sp_fftr(sp_sample_count_t len, sp_sample_t* source, sp_sample_count_t source_len, sp_sample_t* result_samples);
status_t sp_fftri(sp_sample_count_t len, sp_sample_t* source, sp_sample_count_t source_len, sp_sample_t* result_samples);
status_t sp_moving_average(sp_sample_t* source, sp_sample_count_t source_len, sp_sample_t* prev, sp_sample_count_t prev_len, sp_sample_t* next, sp_sample_count_t next_len, sp_sample_count_t radius, sp_sample_count_t start, sp_sample_count_t end, sp_sample_t* result_samples);
void sp_convolve_one(sp_sample_t* a, sp_sample_count_t a_len, sp_sample_t* b, sp_sample_count_t b_len, sp_sample_t* result_samples);
void sp_convolve(sp_sample_t* a, sp_sample_count_t a_len, sp_sample_t* b, sp_sample_count_t b_len, sp_sample_count_t result_carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples);
void sp_channel_data_free(sp_sample_t** a, sp_channel_count_t channel_count);