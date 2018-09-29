#include <byteswap.h>
#include <math.h>
#include <inttypes.h>
#include "../foreign/sph.c"
#include "../foreign/sph/status.c"
#include "../foreign/sph/types.c"
#include "../foreign/sph/float.c"
#include "./config.c"
#define sp_port_type_alsa 0
#define sp_port_type_file 1
#define sp_port_bit_input 1
#define sp_port_bit_output 2
#define sp_port_bit_position 4
#define sp_port_bit_closed 8
#define sp_sample_format_f64 1
#define sp_sample_format_f32 2
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
#define kiss_fft_scalar sp_sample_t
/** sp-float-t integer -> sp-float-t
    radians-per-second samples-per-second -> cutoff-value */
#define sp_windowed_sinc_cutoff(freq, sample_rate) ((2 * M_PI * freq) / sample_rate)
#if (sp_sample_format == sp_sample_format_f64)
#define sp_sample_t double
#define sample_reverse_endian __bswap_64
#define sp_sample_sum f64_sum
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_FLOAT64_LE
#else
#if (sp_sample_format_f32 == sp_sample_format)
#define sp_sample_t float
#define sample_reverse_endian __bswap_32
#define sp_sample_sum f32_sum
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_FLOAT_LE
#endif
#endif
enum { sp_status_id_file_channel_mismatch,
  sp_status_id_file_encoding,
  sp_status_id_file_header,
  sp_status_id_file_incompatible,
  sp_status_id_file_incomplete,
  sp_status_id_eof,
  sp_status_id_input_type,
  sp_status_id_memory,
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
status_t sp_port_read(sp_port_t* port, sp_sample_count_t sample_count, sp_sample_t** result_samples);
status_t sp_port_write(sp_port_t* port, sp_sample_count_t sample_count, sp_sample_t** channel_data);
status_t sp_port_position(sp_port_t* port, sp_sample_count_t* result_position);
status_t sp_port_set_position(sp_port_t* port, sp_sample_count_t sample_index);
status_t sp_file_open(uint8_t* path, sp_channel_count_t channel_count, sp_sample_count_t sample_rate, sp_port_t* result_port);
status_t sp_alsa_open(uint8_t* device_name, boolean is_input, sp_channel_count_t channel_count, sp_sample_rate_t sample_rate, int32_t latency, sp_port_t* result_port);
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
status_t sp_fft(sp_sample_count_t len, sp_sample_t* source, sp_sample_count_t source_len, sp_sample_t* result_samples);
status_t sp_ifft(sp_sample_count_t len, sp_sample_t* source, sp_sample_count_t source_len, sp_sample_t* result_samples);
status_t sp_moving_average(sp_sample_t* source, sp_sample_count_t source_len, sp_sample_t* prev, sp_sample_count_t prev_len, sp_sample_t* next, sp_sample_count_t next_len, sp_sample_count_t radius, sp_sample_count_t start, sp_sample_count_t end, sp_sample_t* result_samples);
void sp_convolve_one(sp_sample_t* a, sp_sample_count_t a_len, sp_sample_t* b, sp_sample_count_t b_len, sp_sample_t* result_samples);
void sp_convolve(sp_sample_t* a, sp_sample_count_t a_len, sp_sample_t* b, sp_sample_count_t b_len, sp_sample_count_t result_carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples);