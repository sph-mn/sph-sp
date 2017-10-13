
#ifndef sc_included_alsa_asoundlib_h
#include <alsa/asoundlib.h>
#define sc_included_alsa_asoundlib_h
#endif
#ifndef sc_included_inttypes_h
#include <inttypes.h>
#define sc_included_inttypes_h
#endif
#ifndef sc_included_stdio_h
#include <stdio.h>
#define sc_included_stdio_h
#endif
#define boolean b8
#define pointer_t uintptr_t
#define b0 void
#define b8 uint8_t
#define b16 uint16_t
#define b32 uint32_t
#define b64 uint64_t
#define b8_s int8_t
#define b16_s int16_t
#define b32_s int32_t
#define b64_s int64_t
#define f32_s float
#define f64_s double
/** writes values with current routine name and line info to standard output.
  example: (debug-log "%d" 1)
  otherwise like printf */
#define debug_log(format, ...)                                                 \
  fprintf(stdout, "%s:%d " format "\n", __func__, __LINE__, __VA_ARGS__)
;
#define null ((b0)(0))
#define zero_p(a) (0 == a)
/* return status code and error handling. uses a local variable named "status" and a goto label named "exit".
   a status has an identifier and a group to discern between status identifiers of different libraries.
   status id 0 is success, everything else can be considered a failure or special case.
   status ids are 32 bit signed integers for compatibility with error return codes from many other existing libraries.
   bindings with a ! suffix update the status from an expression */
typedef b32_s status_i_t;
typedef struct {
  status_i_t id;
  b8 group;
} status_t;
#define status_id_success 0
#define status_group_undefined 0
#define status_init                                                            \
  status_t status = {status_id_success, status_group_undefined}
/** like status init but sets a default group */
#define status_init_group(group) status_t status = {status_id_success, group}
;
#define status_reset status_set_both(status_group_undefined, status_id_success)
#define status_success_p (status_id_success == status.id)
#define status_failure_p !status_success_p
#define status_goto goto exit
#define status_set_group(group_id) status.group = group_id
#define status_set_id(status_id) status.id = status_id
#define status_set_both(group_id, status_id)                                   \
  status_set_group(group_id);                                                  \
  status_set_id(status_id)
#define status_require                                                         \
  if (status_failure_p) {                                                      \
    status_goto;                                                               \
  }
/** update status with the result of expression, check for failure and goto
 * error if so */
#define status_require_x(expression)                                           \
  status = expression;                                                         \
  if (status_failure_p) {                                                      \
    status_goto;                                                               \
  }
;
/** set the status id and goto error */
#define status_set_id_goto(status_id)                                          \
  status_set_id(status_id);                                                    \
  status_goto
;
#define status_set_group_goto(group_id)                                        \
  status_set_group(group_id);                                                  \
  status_goto
#define status_set_both_goto(group_id, status_id)                              \
  status_set_both(group_id, status_id);                                        \
  status_goto
#define status_id_is_p(status_id) (status_id == status.id)
#define sp_sample_t f32_s
#define sp_default_sample_rate 16000
#define sp_default_channel_count 1
#define sp_default_alsa_enable_soft_resample 1
#define sp_default_alsa_latency 50
/** reverse the byte order of one sample */
sp_sample_t sample_reverse_endian(sp_sample_t a) {
  sp_sample_t result;
  b8 *b = ((b8 *)(&a));
  b8 *c = ((b8 *)(&result));
  (*c) = (*(b + 3));
  (*(c + 1)) = (*(b + 2));
  (*(c + 2)) = (*(b + 1));
  (*(c + 3)) = (*b);
  return (result);
}; /* return status handling */
enum {
  sp_status_id_undefined,
  sp_status_id_input_type,
  sp_status_id_not_implemented,
  sp_status_id_memory,
  sp_status_id_file_incompatible,
  sp_status_id_file_encoding,
  sp_status_id_file_header,
  sp_status_id_port_closed,
  sp_status_id_port_position,
  sp_status_id_file_channel_mismatch,
  sp_status_id_file_incomplete,
  sp_status_id_port_type,
  sp_status_group_sp,
  sp_status_group_libc,
  sp_status_group_alsa
};
b8 *sp_status_description(status_t a) {
  return (
      ((sp_status_group_sp == a.group)
           ? ((b8 *)((
                 (sp_status_id_input_type == a.id)
                     ? "input argument is of wrong type"
                     : ((sp_status_id_not_implemented == a.id)
                            ? "not implemented"
                            : ((sp_status_id_memory == a.id)
                                   ? "not enough memory or other memory "
                                     "allocation error"
                                   : ((sp_status_id_file_incompatible == a.id)
                                          ? "file exists but channel count or "
                                            "sample rate is different from "
                                            "what was requested"
                                          : ""))))))
           : ((sp_status_group_alsa == a.group) ? ((b8 *)(snd_strerror(a.id)))
                                                : ((b8 *)("")))));
};
b8 *sp_status_name(status_t a) {
  return (((sp_status_group_sp == a.group)
               ? ((b8 *)(((sp_status_id_input_type == a.id)
                              ? "input-type"
                              : ((sp_status_id_not_implemented == a.id)
                                     ? "not-implemented"
                                     : ((sp_status_id_memory == a.id)
                                            ? "memory"
                                            : "unknown")))))
               : ((sp_status_group_alsa == a.group) ? ((b8 *)("alsa"))
                                                    : ((b8 *)("unknown")))));
};
#define sp_status_init status_init_group(sp_status_group_sp)
#define sp_system_status_require_id(id)                                        \
  if ((id < 0)) {                                                              \
    status_set_both_goto(sp_status_group_libc, id);                            \
  }
#define sp_system_status_require_x(expression)                                 \
  status_set_id(expression);                                                   \
  if ((status.id < 0)) {                                                       \
    status_set_group_goto(sp_status_group_libc);                               \
  } else {                                                                     \
    status_reset;                                                              \
  }
#define sp_alsa_status_require_x(expression)                                   \
  status_set_id(expression);                                                   \
  if (status_failure_p) {                                                      \
    status_set_group_goto(sp_status_group_alsa);                               \
  }
#define sp_octets_to_samples(a) (a / sizeof(sp_sample_t))
#define sp_samples_to_octets(a) (a * sizeof(sp_sample_t))
f32_s sp_sin_lq(f32_s a);
f32_s sp_sinc(f32_s a);
f32_s sp_blackman(f32_s a, size_t width);
b0 sp_spectral_inversion_ir(sp_sample_t *a, size_t a_len);
b0 sp_spectral_reversal_ir(sp_sample_t *a, size_t a_len);
status_t sp_fft(sp_sample_t *result, b32 result_len, sp_sample_t *source,
                b32 source_len);
status_t sp_fft_inverse(sp_sample_t *result, b32 result_len,
                        sp_sample_t *source, b32 source_len);
boolean sp_moving_average(sp_sample_t *result, sp_sample_t *source,
                          b32 source_len, sp_sample_t *prev, b32 prev_len,
                          sp_sample_t *next, b32 next_len, b32 start, b32 end,
                          b32 distance);
b0 sp_convolve_one(sp_sample_t *result, sp_sample_t *a, size_t a_len,
                   sp_sample_t *b, size_t b_len);
b0 sp_convolve(sp_sample_t *result, sp_sample_t *a, size_t a_len,
               sp_sample_t *b, size_t b_len, sp_sample_t *carryover,
               size_t carryover_len);
b0 sp_sine(sp_sample_t *data, b32 start, b32 end, f32_s sample_duration,
           f32_s freq, f32_s phase, f32_s amp);
b0 sp_sine_lq(sp_sample_t *data, b32 start, b32 end, f32_s sample_duration,
              f32_s freq, f32_s phase, f32_s amp);
/** f32-s integer -> f32-s
  radians-per-second samples-per-second -> cutoff-value */
#define sp_windowed_sinc_cutoff(freq, sample_rate)                             \
  ((2 * M_PI * freq) / sample_rate)
;
typedef struct {
  sp_sample_t *data;
  size_t data_len;
  size_t ir_len_prev;
  sp_sample_t *ir;
  size_t ir_len;
  b32 sample_rate;
  f32_s freq;
  f32_s transition;
} sp_windowed_sinc_state_t;
size_t sp_windowed_sinc_ir_length(f32_s transition);
b0 sp_windowed_sinc_ir(sp_sample_t **result, size_t *result_len,
                       b32 sample_rate, f32_s freq, f32_s transition);
b0 sp_windowed_sinc_state_destroy(sp_windowed_sinc_state_t *state);
b8 sp_windowed_sinc_state_create(b32 sample_rate, f32_s freq, f32_s transition,
                                 sp_windowed_sinc_state_t **state);
status_i_t sp_windowed_sinc(sp_sample_t *result, sp_sample_t *source,
                            size_t source_len, b32 sample_rate, f32_s freq,
                            f32_s transition, sp_windowed_sinc_state_t **state);