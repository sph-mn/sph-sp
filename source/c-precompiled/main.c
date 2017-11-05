
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
/** update status with the result of expression, check for failure and goto
 * error if so */
#define status_i_require_x(expression)                                         \
  status.id = expression;                                                      \
  if (status_failure_p) {                                                      \
    status_goto;                                                               \
  }
;
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
};
/* return status handling */ enum {
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
                                          : ((sp_status_id_file_incomplete ==
                                              a.id)
                                                 ? "incomplete write"
                                                 : ((sp_status_id_port_type ==
                                                     a.id)
                                                        ? "incompatible port "
                                                          "type"
                                                        : ""))))))))
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
#define sp_status_require_alloc(a)                                             \
  if (!a) {                                                                    \
    status_set_both_goto(sp_status_group_sp, sp_status_id_memory);             \
  }
#define sp_octets_to_samples(a) (a / sizeof(sp_sample_t))
#define sp_samples_to_octets(a) (a * sizeof(sp_sample_t))
sp_sample_t **sp_alloc_channel_array(b32 channel_count, b32 sample_count);
f32_s sp_sin_lq(f32_s a);
f32_s sp_sinc(f32_s a);
f32_s sp_window_blackman(f32_s a, size_t width);
b0 sp_spectral_inversion_ir(sp_sample_t *a, size_t a_len);
b0 sp_spectral_reversal_ir(sp_sample_t *a, size_t a_len);
status_t sp_fft(sp_sample_t *result, b32 result_len, sp_sample_t *source,
                b32 source_len);
status_t sp_ifft(sp_sample_t *result, b32 result_len, sp_sample_t *source,
                 b32 source_len);
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
typedef struct {
  b32 sample_rate;
  b32 channel_count;
  boolean closed_p;
  b8 flags;
  b8 type;
  size_t position;
  b16 position_offset;
  b0 *data;
  int data_int;
} sp_port_t;
#define sp_port_type_alsa 0
#define sp_port_type_file 1
#define sp_port_bit_input 1
#define sp_port_bit_output 2
#define sp_port_bit_position 4
status_t sp_port_read(sp_sample_t **result, sp_port_t *port, b32 sample_count);
status_t sp_port_write(sp_port_t *port, b32 sample_count,
                       sp_sample_t **channel_data);
status_t sp_port_sample_count(size_t *result, sp_port_t *port);
status_t sp_port_set_position(sp_port_t *port, b64_s sample_index);
status_t sp_file_open(sp_port_t *result, b8 *path, b32_s channel_count,
                      b32_s sample_rate);
status_t sp_alsa_open(sp_port_t *result, b8 *device_name, boolean input_p,
                      b32_s channel_count, b32_s sample_rate, b32_s latency);
status_t sp_port_close(sp_port_t *a);
#define kiss_fft_scalar sp_sample_t
#ifndef sc_included_stdio_h
#include <stdio.h>
#define sc_included_stdio_h
#endif
#ifndef sc_included_kiss_fft_h
#include <foreign/kissfft/kiss_fft.h>
#define sc_included_kiss_fft_h
#endif
#ifndef sc_included_kiss_fftr_h
#include <foreign/kissfft/tools/kiss_fftr.h>
#define sc_included_kiss_fftr_h
#endif
#ifndef sc_included_fcntl_h
#include <fcntl.h>
#define sc_included_fcntl_h
#endif
#ifndef sc_included_string_h
#include <string.h>
#define sc_included_string_h
#endif
#ifndef sc_included_stdlib_h
#include <stdlib.h>
#define sc_included_stdlib_h
#endif
#ifndef sc_included_unistd_h
#include <unistd.h>
#define sc_included_unistd_h
#endif
#ifndef sc_included_sys_stat_h
#include <sys/stat.h>
#define sc_included_sys_stat_h
#endif
#ifndef sc_included_libgen_h
#include <libgen.h>
#define sc_included_libgen_h
#endif
#ifndef sc_included_errno_h
#include <errno.h>
#define sc_included_errno_h
#endif
#ifndef sc_included_float_h
#include <float.h>
#define sc_included_float_h
#endif
#define file_exists_p(path) !(access(path, F_OK) == -1)
#define pointer_equal_p(a, b) (((b0 *)(a)) == ((b0 *)(b)))
/** set result to a new string with a trailing slash added, or the given string
  if it already has a trailing slash. returns 0 if result is the given string, 1
  if new memory could not be allocated, 2 if result is a new string */
b8 ensure_trailing_slash(b8 *a, b8 **result) {
  b32 a_len = strlen(a);
  if ((!a_len || (('/' == (*(a + (a_len - 1))))))) {
    (*result) = a;
    return (0);
  } else {
    char *new_a = malloc((2 + a_len));
    if (!new_a) {
      return (1);
    };
    memcpy(new_a, a, a_len);
    memcpy((new_a + a_len), "/", 1);
    (*(new_a + (1 + a_len))) = 0;
    (*result) = new_a;
    return (2);
  };
};
/** return a new string with the same contents as the given string. return 0 if
 * the memory allocation failed */
b8 *string_clone(b8 *a) {
  size_t a_size = (1 + strlen(a));
  b8 *result = malloc(a_size);
  if (result) {
    memcpy(result, a, a_size);
  };
  return (result);
};
/** like posix dirname, but never modifies its argument and always returns a new
 * string */
b8 *dirname_2(b8 *a) {
  b8 *path_copy = string_clone(a);
  return (dirname(path_copy));
};
/** return 1 if the path exists or has been successfully created */
boolean ensure_directory_structure(b8 *path, mode_t mkdir_mode) {
  if (file_exists_p(path)) {
    return (1);
  } else {
    b8 *path_dirname = dirname_2(path);
    boolean status = ensure_directory_structure(path_dirname, mkdir_mode);
    free(path_dirname);
    return ((status &&
             ((((EEXIST == errno)) || ((0 == mkdir(path, mkdir_mode)))))));
  };
};
/** always returns a new string */
b8 *string_append(b8 *a, b8 *b) {
  size_t a_length = strlen(a);
  size_t b_length = strlen(b);
  b8 *result = malloc((1 + a_length + b_length));
  if (result) {
    memcpy(result, a, a_length);
    memcpy((result + a_length), b, (1 + b_length));
  };
  return (result);
};
/** sum numbers with rounding error compensation using kahan summation with
 * neumaier modification */
f32_s float_sum(f32_s *numbers, b32 len) {
  f32_s temp;
  f32_s element;
  f32_s correction = 0;
  len = (len - 1);
  f32_s result = (*(numbers + len));
  while (len) {
    len = (len - 1);
    element = (*(numbers + len));
    temp = (result + element);
    correction =
        (correction + ((result >= element) ? ((result - temp) + element)
                                           : ((element - temp) + result)));
    result = temp;
  };
  return ((correction + result));
};
/** approximate float comparison. margin is a factor and is low for low accepted
   differences. http://floating-point-gui.de/errors/comparison/ */
boolean float_nearly_equal_p(f32_s a, f32_s b, f32_s margin) {
  if ((a == b)) {
    return (1);
  } else {
    f32_s diff = fabs((a - b));
    return (((((0 == a)) || ((0 == b)) || (diff < DBL_MIN))
                 ? (diff < (margin * DBL_MIN))
                 : ((diff / fmin((fabs(a) + fabs(b)), DBL_MAX)) < margin)));
  };
};
/** register memory in a local variable to free all memory allocated at point */
#define local_memory_init(register_size)                                       \
  b0 *sph_local_memory_register[register_size];                                \
  b8 sph_local_memory_index = 0
;
/** do not try to add more entries than specified by register-size or a buffer
 * overflow occurs */
#define local_memory_add(address)                                              \
  (*(sph_local_memory_register + sph_local_memory_index)) = address;           \
  sph_local_memory_index = (1 + sph_local_memory_index)
;
#define local_memory_free                                                      \
  while (sph_local_memory_index) {                                             \
    sph_local_memory_index = (sph_local_memory_index - 1);                     \
    free((*(sph_local_memory_register + sph_local_memory_index)));             \
  }
#define sp_define_malloc(id, type, size)                                       \
  type id = malloc(size);                                                      \
  if (!id) {                                                                   \
    status_set_both_goto(sp_status_group_sp, sp_status_id_memory);             \
  }
#define sp_set_malloc(id, size)                                                \
  id = malloc(size);                                                           \
  if (!id) {                                                                   \
    status_set_both_goto(sp_status_group_sp, sp_status_id_memory);             \
  }
#define sp_define_calloc(id, type, size)                                       \
  type id = calloc(size, 1);                                                   \
  if (!id) {                                                                   \
    status_set_both_goto(sp_status_group_sp, sp_status_id_memory);             \
  }
#define sp_set_calloc(id, size)                                                \
  id = calloc(size, 1);                                                        \
  if (!id) {                                                                   \
    status_set_both_goto(sp_status_group_sp, sp_status_id_memory);             \
  }
#define sp_define_malloc_samples(id, sample_count)                             \
  sp_define_malloc(id, sp_sample_t *, (sample_count * sizeof(sp_sample_t)))
#define sp_set_malloc_samples(id, sample_count)                                \
  sp_set_malloc(id, (sample_count * sizeof(sp_sample_t)))
#define inc(a) a = (1 + a)
#define dec(a) a = (a - 1)
/** zero if memory could not be allocated */
sp_sample_t **sp_alloc_channel_array(b32 channel_count, b32 sample_count) {
  local_memory_init((channel_count + 1));
  sp_sample_t **result = malloc((channel_count * sizeof(sp_sample_t *)));
  if (!result) {
    return (0);
  };
  local_memory_add(result);
  sp_sample_t *channel;
  while (channel_count) {
    dec(channel_count);
    channel = calloc((sample_count * sizeof(sp_sample_t)), 1);
    local_memory_add(channel);
    if (!channel) {
      local_memory_free;
      return (0);
    };
    (*(result + channel_count)) = channel;
  };
  return (result);
};
/** lower precision version of sin() that is faster to compute */
f32_s sp_sin_lq(f32_s a) {
  f32_s b = (4 / M_PI);
  f32_s c = (-4 / (M_PI * M_PI));
  return ((((b * a) + (c * a * abs(a)))));
};
status_t sp_fft(sp_sample_t *result, b32 result_len, sp_sample_t *source,
                b32 source_len) {
  sp_status_init;
  local_memory_init(2);
  kiss_fftr_cfg fftr_state = kiss_fftr_alloc(result_len, 0, 0, 0);
  if (!fftr_state) {
    status_set_id_goto(sp_status_id_memory);
  };
  local_memory_add(fftr_state);
  sp_define_malloc(out, kiss_fft_cpx *, (result_len * sizeof(kiss_fft_cpx)));
  local_memory_add(out);
  kiss_fftr(fftr_state, source, out);
  while (result_len) {
    dec(result_len);
    (*(result + result_len)) = (*(out + result_len)).r;
  };
exit:
  local_memory_free;
  return (status);
};
status_t sp_ifft(sp_sample_t *result, b32 result_len, sp_sample_t *source,
                 b32 source_len) {
  sp_status_init;
  local_memory_init(2);
  kiss_fftr_cfg fftr_state = kiss_fftr_alloc(source_len, 1, 0, 0);
  if (!fftr_state) {
    status_set_id_goto(sp_status_id_memory);
  };
  local_memory_add(fftr_state);
  sp_define_malloc(in, kiss_fft_cpx *, (source_len * sizeof(kiss_fft_cpx)));
  local_memory_add(in);
  while (source_len) {
    dec(source_len);
    (*(in + source_len)).r = (*(source + (source_len * sizeof(sp_sample_t))));
  };
  kiss_fftri(fftr_state, in, result);
exit:
  local_memory_free;
  return (status);
};
/** apply a centered moving average filter to source at index start to end
  inclusively and write to result. removes higher frequencies with little
  distortion in the time domain.
   * only the result portion corresponding to the subvector from start to end is
  written to result
   * prev and next are unprocessed segments and can be null pointers,
     for example at the beginning and end of a stream
   * since the result value for a sample is calculated using samples left and
  right of it, a previous and following part of a stream is eventually needed to
  reference values outside the source segment to create a valid continuous
  result. zero is used for unavailable values outside the source segment
   * available values outside the start/end range are considered where needed to
  calculate averages
   * rounding errors are kept low by using modified kahan neumaier summation and
  not using a recursive implementation. both properties which make it much
  slower than many other implementations */
boolean sp_moving_average(sp_sample_t *result, sp_sample_t *source,
                          b32 source_len, sp_sample_t *prev, b32 prev_len,
                          sp_sample_t *next, b32 next_len, b32 start, b32 end,
                          b32 distance) {
  if (!source_len) {
    return (1);
  };
  b32 left;
  b32 right;
  b32 width = (1 + (2 * distance));
  sp_sample_t *window = 0;
  b32 window_index;
  if (!(((start >= distance)) && (((start + distance + 1) <= source_len)))) {
    window = malloc((width * sizeof(sp_sample_t)));
    if (!window) {
      return (1);
    };
  };
  while ((start <= end)) {
    if ((((start >= distance)) && (((start + distance + 1) <= source_len)))) {
      (*result) = (float_sum(((source + start) - distance), width) / width);
    } else {
      window_index = 0;
      if ((start < distance)) {
        right = (distance - start);
        if (prev) {
          left = ((right > prev_len) ? 0 : (prev_len - right));
          while ((left < prev_len)) {
            (*(window + window_index)) = (*(prev + left));
            inc(window_index);
            inc(left);
          };
        };
        while ((window_index < right)) {
          (*(window + window_index)) = 0;
          inc(window_index);
        };
        left = 0;
      } else {
        left = (start - distance);
      };
      right = (start + distance);
      if ((right >= source_len)) {
        right = (source_len - 1);
      };
      while ((left <= right)) {
        (*(window + window_index)) = (*(source + left));
        inc(window_index);
        inc(left);
      };
      right = (start + distance);
      if ((((right >= source_len)) && next)) {
        left = 0;
        right = (right - source_len);
        if ((right >= next_len)) {
          right = (next_len - 1);
        };
        while ((left <= right)) {
          (*(window + window_index)) = (*(next + left));
          inc(window_index);
          inc(left);
        };
      };
      while ((window_index < width)) {
        (*(window + window_index)) = 0;
        inc(window_index);
      };
      (*result) = (float_sum(window, width) / width);
    };
    inc(result);
    inc(start);
  };
  free(window);
  return (0);
};
/** the normalised sinc function */
f32_s sp_sinc(f32_s a) {
  return (((0 == a) ? 1 : (sin((M_PI * a)) / (M_PI * a))));
};
f32_s sp_window_blackman(f32_s a, size_t width) {
  return (((0.42 - (0.5 * cos(((2 * M_PI * a) / (width - 1))))) +
           (0.8 * cos(((4 * M_PI * a) / (width - 1))))));
};
/** modify an impulse response kernel for spectral inversion.
   a-len must be odd and "a" must have left-right symmetry.
  flips the frequency response top to bottom */
b0 sp_spectral_inversion_ir(sp_sample_t *a, size_t a_len) {
  while (a_len) {
    dec(a_len);
    (*(a + a_len)) = (-1 * (*(a + a_len)));
  };
  size_t center = ((a_len - 1) / 2);
  inc((*(a + center)));
};
/** inverts the sign for samples at odd indexes.
  a-len must be odd and "a" must have left-right symmetry.
  flips the frequency response left to right */
b0 sp_spectral_reversal_ir(sp_sample_t *a, size_t a_len) {
  while ((a_len > 1)) {
    a_len = (a_len - 2);
    (*(a + a_len)) = (-1 * (*(a + a_len)));
  };
};
/** discrete linear convolution.
  result length must be at least a-len + b-len - 1 */
b0 sp_convolve_one(sp_sample_t *result, sp_sample_t *a, size_t a_len,
                   sp_sample_t *b, size_t b_len) {
  size_t a_index = 0;
  size_t b_index = 0;
  while ((a_index < a_len)) {
    while ((b_index < b_len)) {
      (*(result + (a_index + b_index))) =
          ((*(result + (a_index + b_index))) +
           ((*(a + a_index)) * (*(b + b_index))));
      inc(b_index);
    };
    b_index = 0;
    inc(a_index);
  };
};
/** discrete linear convolution for segments of a continuous stream. maps
  segments (a, a-len) to result. result length is a-len, carryover length is
  b-len or previous b-len. b-len must be greater than zero */
b0 sp_convolve(sp_sample_t *result, sp_sample_t *a, size_t a_len,
               sp_sample_t *b, size_t b_len, sp_sample_t *carryover,
               size_t carryover_len) {
  memset(result, 0, (a_len * sizeof(sp_sample_t)));
  memcpy(result, carryover, (carryover_len * sizeof(sp_sample_t)));
  memset(carryover, 0, (b_len * sizeof(sp_sample_t)));
  size_t size = ((a_len < b_len) ? 0 : (a_len - (b_len - 1)));
  if (size) {
    sp_convolve_one(result, a, size, b, b_len);
  };
  size_t a_index = size;
  size_t b_index = 0;
  size_t c_index;
  while ((a_index < a_len)) {
    while ((b_index < b_len)) {
      c_index = (a_index + b_index);
      if ((c_index < a_len)) {
        (*(result + c_index)) =
            ((*(result + c_index)) + ((*(a + a_index)) * (*(b + b_index))));
      } else {
        c_index = (c_index - a_len);
        (*(carryover + c_index)) =
            ((*(carryover + c_index)) + ((*(a + a_index)) * (*(b + b_index))));
      };
      inc(b_index);
    };
    b_index = 0;
    inc(a_index);
  };
};
/* write samples for a sine wave into data between start at end.
   defines sp-sine, sp-sine-lq */
#define define_sp_sine(id, sin)                                                \
  b0 id(sp_sample_t *data, b32 start, b32 end, f32_s sample_duration,          \
        f32_s freq, f32_s phase, f32_s amp) {                                  \
    while ((start <= end)) {                                                   \
      (*(data + start)) = (amp * sin((freq * phase * sample_duration)));       \
      inc(phase);                                                              \
      inc(start);                                                              \
    };                                                                         \
  }
define_sp_sine(sp_sine, sin);
define_sp_sine(sp_sine_lq, sp_sin_lq);
/* implementation of a hq windowed sinc filter with a blackman window (common
  truncated version) for continuous streams. variable sample-rate, cutoff radian
  frequency and transition band width per call. depends on some sp functions
  defined in main.sc and sph-sp.sc */
/** approximate impulse response length for a transition factor and
  ensure that the length is odd */
size_t sp_windowed_sinc_ir_length(f32_s transition) {
  b32 result = ceil((4 / transition));
  if (!(result % 2)) {
    inc(result);
  };
  return (result);
};
/** create an impulse response kernel for a windowed sinc filter. uses a
  blackman window (truncated version). allocates result, sets result-len. failed
  if result is null */
b0 sp_windowed_sinc_ir(sp_sample_t **result, size_t *result_len,
                       b32 sample_rate, f32_s freq, f32_s transition) {
  size_t len = sp_windowed_sinc_ir_length(transition);
  (*result_len) = len;
  f32_s center_index = ((len - 1.0) / 2.0);
  f32_s cutoff = sp_windowed_sinc_cutoff(freq, sample_rate);
  sp_sample_t *result_temp = malloc((len * sizeof(sp_sample_t)));
  result_temp = malloc((len * sizeof(sp_sample_t)));
  if (!result_temp) {
    (*result) = 0;
    return;
  };
  size_t index = 0;
  while ((index < len)) {
    (*(result_temp + index)) =
        sp_window_blackman(sp_sinc((2 * cutoff * (index - center_index))), len);
    inc(index);
  };
  f32_s result_sum = float_sum(result_temp, len);
  while (len) {
    dec(len);
    (*(result_temp + index)) = ((*(result_temp + index)) / result_sum);
  };
  (*result) = result_temp;
};
b0 sp_windowed_sinc_state_destroy(sp_windowed_sinc_state_t *state) {
  free((*state).ir);
  free((*state).data);
  free(state);
};
/** create or update a state object. impulse response array properties are
  calculated from sample-rate, freq and transition. eventually frees state.ir.
  ir-len-prev data elements have to be copied to the next result */
b8 sp_windowed_sinc_state_create(b32 sample_rate, f32_s freq, f32_s transition,
                                 sp_windowed_sinc_state_t **state) {
  sp_windowed_sinc_state_t *state_temp;
  if (!(*state)) {
    state_temp = malloc(sizeof(sp_windowed_sinc_state_t));
    if (!state_temp) {
      return (1);
    };
    (*state_temp).sample_rate = 0;
    (*state_temp).freq = 0;
    (*state_temp).transition = 0;
    (*state_temp).data = 0;
    (*state_temp).data_len = 0;
    (*state_temp).ir_len_prev = 0;
  } else {
    state_temp = (*state);
  };
  if (((((*state_temp).sample_rate == sample_rate)) &&
       (((*state_temp).freq == freq)) &&
       (((*state_temp).transition == transition)))) {
    return (0);
  };
  if (state) {
    free((*state_temp).ir);
  };
  size_t ir_len;
  sp_sample_t *ir;
  sp_windowed_sinc_ir(&ir, &ir_len, sample_rate, freq, transition);
  if (!ir) {
    if (!(*state)) {
      free(state_temp);
    };
    return (1);
  };
  (*state_temp).ir_len_prev = (state ? (*state_temp).ir_len : ir_len);
  if ((ir_len > (*state_temp).data_len)) {
    sp_sample_t *data = calloc(ir_len, sizeof(sp_sample_t));
    if (!data) {
      free(ir);
      if (!(*state)) {
        free(state_temp);
      };
      return (1);
    };
    if ((*state_temp).data) {
      memcpy(data, (*state_temp).data,
             ((*state_temp).ir_len_prev * sizeof(sp_sample_t)));
      free((*state_temp).data);
    };
    (*state_temp).data = data;
    (*state_temp).data_len = ir_len;
  };
  (*state_temp).ir = ir;
  (*state_temp).ir_len = ir_len;
  (*state_temp).sample_rate = sample_rate;
  (*state_temp).freq = freq;
  (*state_temp).transition = transition;
  (*state) = state_temp;
  return (0);
};
/** a windowed sinc filter for segments of continuous streams with
  sample-rate, frequency and transition variable per call.
  state can be zero and it will be allocated */
status_i_t sp_windowed_sinc(sp_sample_t *result, sp_sample_t *source,
                            size_t source_len, b32 sample_rate, f32_s freq,
                            f32_s transition,
                            sp_windowed_sinc_state_t **state) {
  if (sp_windowed_sinc_state_create(sample_rate, freq, transition, state)) {
    return (1);
  };
  sp_convolve(result, source, source_len, (*(*state)).ir, (*(*state)).ir_len,
              (*(*state)).data, (*(*state)).ir_len_prev);
  return (0);
};
/** -> boolean
  returns 1 if the header was successfully written, 0 otherwise.
  assumes file is positioned at offset 0 */
b8_s file_au_write_header(int file, b32 encoding, b32 sample_rate,
                          b32 channel_count) {
  b32 header[7];
  (*(header + 0)) = __builtin_bswap32(779316836);
  (*(header + 1)) = __builtin_bswap32(28);
  (*(header + 2)) = __builtin_bswap32(4294967295);
  (*(header + 3)) = __builtin_bswap32(encoding);
  (*(header + 4)) = __builtin_bswap32(sample_rate);
  (*(header + 5)) = __builtin_bswap32(channel_count);
  (*(header + 6)) = 0;
  ssize_t status = write(file, header, 28);
  return ((status == 28));
};
/** -> boolean
  when successful, the reader is positioned at the beginning of the sample data
*/
b8_s file_au_read_header(int file, b32 *encoding, b32 *sample_rate,
                         b32 *channel_count) {
  ssize_t status;
  b32 header[6];
  status = read(file, header, 24);
  if (!(((status == 24)) && (((*header) == __builtin_bswap32(779316836))))) {
    return (0);
  };
  if ((lseek(file, __builtin_bswap32((*(header + 1))), SEEK_SET) < 0)) {
    return (0);
  };
  (*encoding) = __builtin_bswap32((*(header + 3)));
  (*sample_rate) = __builtin_bswap32((*(header + 4)));
  (*channel_count) = __builtin_bswap32((*(header + 5)));
  return (1);
};
/** a: deinterleaved
   b: interleaved */
#define define_sp_interleave(name, type, body)                                 \
  b0 name(type **a, type *b, size_t a_size, b32 channel_count) {               \
    size_t b_size = (a_size * channel_count);                                  \
    b32 channel;                                                               \
    while (a_size) {                                                           \
      dec(a_size);                                                             \
      channel = channel_count;                                                 \
      while (channel) {                                                        \
        dec(channel);                                                          \
        dec(b_size);                                                           \
        body;                                                                  \
      };                                                                       \
    };                                                                         \
  }
;
define_sp_interleave(sp_interleave_and_reverse_endian, sp_sample_t, {
  (*(b + b_size)) = sample_reverse_endian((*((*(a + channel)) + a_size)));
});
define_sp_interleave(sp_deinterleave_and_reverse_endian, sp_sample_t, {
  (*((*(a + channel)) + a_size)) = sample_reverse_endian((*(b + b_size)));
});
/** choose a default when number is negative */
#define optional_number(a, default) ((a < 0) ? default : a)
;
#define set_optional_number(a, default)                                        \
  if ((a < 0)) {                                                               \
    a = default;                                                               \
  }
/* sp-port abstracts different output targets and formats */
/** integer integer integer integer integer pointer integer -> sp-port
   flags is a combination of sp-port-bits */
status_i_t sp_port_init(sp_port_t *result, b8 type, b8 flags, b32 sample_rate,
                        b32 channel_count, b16 position_offset, b0 *data,
                        int data_int) {
  (*result).channel_count = channel_count;
  (*result).sample_rate = sample_rate;
  (*result).type = type;
  (*result).flags = flags;
  (*result).data = data;
  (*result).data_int = data_int;
  (*result).position = 0;
  (*result).position_offset = position_offset;
  return (status_id_success);
};
boolean sp_port_position_p(sp_port_t *a) {
  return ((sp_port_bit_position & (*a).flags));
};
boolean sp_port_input_p(sp_port_t *a) {
  return ((sp_port_bit_input & (*a).flags));
};
boolean sp_port_output_p(sp_port_t *a) {
  return ((sp_port_bit_output & (*a).flags));
};
/* -- au file */ status_t sp_file_open(sp_port_t *result, b8 *path,
                                       b32_s channel_count, b32_s sample_rate) {
  sp_status_init;
  int file;
  b32 channel_count_file;
  b32 sample_rate_file;
  b8 sp_port_flags =
      (sp_port_bit_input | sp_port_bit_output | sp_port_bit_position);
  if (file_exists_p(path)) {
    file = open(path, O_RDWR);
    sp_system_status_require_id(file);
    b32 encoding;
    if (!file_au_read_header(file, &encoding, &sample_rate_file,
                             &channel_count_file)) {
      status_set_id_goto(sp_status_id_file_header);
    };
    if (!(encoding == 6)) {
      status_set_id_goto(sp_status_id_file_encoding);
    };
    if (!(channel_count_file == channel_count)) {
      status_set_id_goto(sp_status_id_file_incompatible);
    };
    if (!(sample_rate_file == sample_rate)) {
      status_set_id_goto(sp_status_id_file_incompatible);
    };
    off_t offset = lseek(file, 0, SEEK_CUR);
    sp_system_status_require_id(offset);
    status_i_require_x(sp_port_init(result, sp_port_type_file, sp_port_flags,
                                    sample_rate_file, channel_count_file,
                                    offset, 0, file));
  } else {
    file = open(path, (O_RDWR | O_CREAT), 384);
    sp_system_status_require_id(file);
    sample_rate_file = optional_number(sample_rate, sp_default_sample_rate);
    channel_count_file =
        optional_number(channel_count, sp_default_channel_count);
    if (!file_au_write_header(file, 6, sample_rate_file, channel_count_file)) {
      status_set_id_goto(sp_status_id_file_header);
    };
    off_t offset = lseek(file, 0, SEEK_CUR);
    sp_system_status_require_id(offset);
    status_i_require_x(sp_port_init(result, sp_port_type_file, sp_port_flags,
                                    sample_rate_file, channel_count_file,
                                    offset, 0, file));
  };
exit:
  if ((status_failure_p && file)) {
    close(file);
  };
  return (status);
};
status_t sp_file_write(sp_port_t *port, b32 sample_count,
                       sp_sample_t **channel_data) {
  status_init;
  local_memory_init(1);
  if (!(sp_port_bit_input & (*port).flags)) {
    status_set_both_goto(sp_status_group_sp, sp_status_id_port_type);
  };
  b32 channel_count = (*port).channel_count;
  size_t interleaved_size =
      (channel_count * sample_count * sizeof(sp_sample_t *));
  sp_define_malloc(interleaved, sp_sample_t *, interleaved_size);
  local_memory_add(interleaved);
  sp_interleave_and_reverse_endian(channel_data, interleaved, sample_count,
                                   channel_count);
  int count = write((*port).data_int, interleaved, interleaved_size);
  if (!(interleaved_size == count)) {
    if ((count < 0)) {
      status_set_both_goto(sp_status_group_libc, count);
    } else {
      status_set_both_goto(sp_status_group_sp, sp_status_id_file_incomplete);
    };
  };
exit:
  local_memory_free;
  return (status);
};
status_t sp_file_read(sp_sample_t **result, sp_port_t *port,
                      size_t sample_count) {
  status_init;
  local_memory_init(1);
  b32 channel_count = (*port).channel_count;
  size_t interleaved_size =
      (channel_count * sample_count * sizeof(sp_sample_t));
  sp_define_malloc(interleaved, sp_sample_t *, interleaved_size);
  local_memory_add(interleaved);
  int count = read((*port).data_int, interleaved, interleaved_size);
  if (!count) {
    result = 0;
    goto exit;
  } else {
    if ((count < 0)) {
      status_set_both_goto(sp_status_group_libc, count);
    } else {
      if (!(interleaved_size == count)) {
        interleaved_size = count;
        sample_count = (interleaved_size / channel_count / sizeof(sp_sample_t));
      };
    };
  };
  sp_deinterleave_and_reverse_endian(result, interleaved, sample_count,
                                     channel_count);
exit:
  local_memory_free;
  return (status);
};
#define sp_file_index_to_position(index, position_offset, channel_count)       \
  ((index - position_offset) / channel_count / sizeof(sp_sample_t))
/** set port to offset in sample data */
status_t sp_file_set_position(sp_port_t *port, b64_s sample_index) {
  status_init;
  b64_s index = (sample_index * (*port).channel_count * sizeof(sp_sample_t));
  b16 header_size = (*port).position_offset;
  int file = (*port).data_int;
  if ((0 > index)) {
    off_t end_position = lseek(file, 0, SEEK_END);
    sp_system_status_require_id(end_position);
    index = (end_position + index);
  } else {
    index = (header_size + index);
  };
  if ((header_size > index)) {
    status_set_both_goto(sp_status_group_sp, sp_status_id_port_position);
  };
  sp_system_status_require_x(lseek(file, index, SEEK_SET));
  (*port).position =
      sp_file_index_to_position(index, header_size, (*port).channel_count);
exit:
  return (status);
};
status_t sp_file_sample_count(size_t *result, sp_port_t *port) {
  status_init;
  int file = (*port).data_int;
  off_t offset = lseek(file, 0, SEEK_END);
  sp_system_status_require_id(offset);
  size_t index = lseek(file, 0, SEEK_CUR);
  b16 header_size = (*port).position_offset;
  if ((header_size > index)) {
    status_set_both_goto(sp_status_group_sp, sp_status_id_port_position);
  };
  (*result) =
      sp_file_index_to_position(index, header_size, (*port).channel_count);
exit:
  return (status);
};
/* -- alsa */
/** open alsa sound output for capture or playback */
status_t sp_alsa_open(sp_port_t *result, b8 *device_name, boolean input_p,
                      b32_s channel_count, b32_s sample_rate, b32_s latency) {
  status_init;
  if (!device_name) {
    device_name = "default";
  };
  set_optional_number(latency, sp_default_alsa_latency);
  set_optional_number(sample_rate, sp_default_channel_count);
  snd_pcm_t *alsa_port = 0;
  sp_alsa_status_require_x(snd_pcm_open(
      &alsa_port, device_name,
      (input_p ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK), 0));
  sp_alsa_status_require_x(snd_pcm_set_params(
      alsa_port, SND_PCM_FORMAT_FLOAT_LE, SND_PCM_ACCESS_RW_NONINTERLEAVED,
      channel_count, sample_rate, sp_default_alsa_enable_soft_resample,
      latency));
  b8 sp_port_flags = (input_p ? sp_port_bit_input : sp_port_bit_output);
  status_i_require_x(sp_port_init(result, sp_port_type_alsa, sp_port_flags,
                                  sample_rate, channel_count, 0,
                                  ((b0 *)(alsa_port)), 0));
exit:
  if ((status_failure_p && alsa_port)) {
    snd_pcm_close(alsa_port);
  };
  return (status);
};
status_t sp_alsa_write(sp_port_t *port, b32 sample_count,
                       sp_sample_t **channel_data) {
  status_init;
  size_t deinterleaved_size = sp_samples_to_octets(sample_count);
  snd_pcm_t *alsa_port = (*port).data;
  snd_pcm_sframes_t frames_written =
      snd_pcm_writen(alsa_port, ((b0 **)(channel_data)),
                     ((snd_pcm_uframes_t)(deinterleaved_size)));
  if (((frames_written < 0) &&
       (snd_pcm_recover(alsa_port, frames_written, 0) < 0))) {
    status_set_both_goto(sp_status_group_alsa, frames_written);
  };
exit:
  return (status);
};
status_t sp_alsa_read(sp_sample_t **result, sp_port_t *port, b32 sample_count) {
  status_init;
  snd_pcm_t *alsa_port = (*port).data;
  snd_pcm_sframes_t frames_read =
      snd_pcm_readn(alsa_port, ((b0 **)(result)), sample_count);
  if (((frames_read < 0) && (snd_pcm_recover(alsa_port, frames_read, 0) < 0))) {
    status_set_both_goto(sp_status_group_alsa, frames_read);
  };
exit:
  return (status);
};
status_t sp_port_read(sp_sample_t **result, sp_port_t *port, b32 sample_count) {
  if ((sp_port_type_file == (*port).type)) {
    return (sp_file_read(result, port, sample_count));
  } else {
    if ((sp_port_type_alsa == (*port).type)) {
      return (sp_alsa_read(result, port, sample_count));
    };
  };
};
status_t sp_port_write(sp_port_t *port, b32 sample_count,
                       sp_sample_t **channel_data) {
  if ((sp_port_type_file == (*port).type)) {
    return (sp_file_write(port, sample_count, channel_data));
  } else {
    if ((sp_port_type_alsa == (*port).type)) {
      return (sp_alsa_write(port, sample_count, channel_data));
    };
  };
};
status_t sp_port_close(sp_port_t *a) {
  status_init;
  if ((*a).closed_p) {
    goto exit;
  };
  if ((sp_port_type_alsa == (*a).type)) {
    sp_alsa_status_require_x(snd_pcm_close(((snd_pcm_t *)((*a).data))));
  } else {
    if ((sp_port_type_file == (*a).type)) {
      sp_system_status_require_x(close((*a).data_int));
    };
  };
  (*a).closed_p = 1;
exit:
  return (status);
};
#define sp_port_not_implemented                                                \
  status_init;                                                                 \
  status_set_both(sp_status_group_sp, sp_status_id_not_implemented);           \
  return (status)
status_t sp_port_sample_count(size_t *result, sp_port_t *port) {
  if ((sp_port_type_file == (*port).type)) {
    return (sp_file_sample_count(result, port));
  } else {
    if ((sp_port_type_alsa == (*port).type)) {
      sp_port_not_implemented;
    };
  };
};
status_t sp_port_set_position(sp_port_t *port, b64_s sample_index) {
  if ((sp_port_type_file == (*port).type)) {
    return (sp_file_set_position(port, sample_index));
  } else {
    if ((sp_port_type_alsa == (*port).type)) {
      sp_port_not_implemented;
    };
  };
};