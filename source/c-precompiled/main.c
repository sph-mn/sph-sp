
#include <libguile.h>
#ifndef sc_included_stdio_h
#include <stdio.h>
#define sc_included_stdio_h
#endif
#ifndef sc_included_fcntl_h
#include <fcntl.h>
#define sc_included_fcntl_h
#endif
#ifndef sc_included_alsa_asoundlib_h
#include <alsa/asoundlib.h>
#define sc_included_alsa_asoundlib_h
#endif
#ifndef sc_included_sndfile_h
#include <sndfile.h>
#define sc_included_sndfile_h
#endif
#ifndef sc_included_byteswap_h
#include <byteswap.h>
#define sc_included_byteswap_h
#endif
#ifndef sc_included_math_h
#include <math.h>
#define sc_included_math_h
#endif
#define sp_sample_type_f64 1
#define sp_sample_type_f32 2
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
#ifndef sc_included_float_h
#include <float.h>
#define sc_included_float_h
#endif
#ifndef sc_included_math_h
#include <math.h>
#define sc_included_math_h
#endif
#define define_float_sum(prefix, type)                                         \
  type prefix##_sum(type *numbers, size_t len) {                               \
    type temp;                                                                 \
    type element;                                                              \
    type correction = 0;                                                       \
    len = (len - 1);                                                           \
    type result = (*(numbers + len));                                          \
    while (len) {                                                              \
      len = (len - 1);                                                         \
      element = (*(numbers + len));                                            \
      temp = (result + element);                                               \
      correction =                                                             \
          (correction + ((result >= element) ? ((result - temp) + element)     \
                                             : ((element - temp) + result)));  \
      result = temp;                                                           \
    };                                                                         \
    return ((correction + result));                                            \
  }
#define define_float_array_nearly_equal_p(prefix, type)                        \
  boolean prefix##_array_nearly_equal_p(type *a, size_t a_len, type *b,        \
                                        size_t b_len, type error_margin) {     \
    size_t index = 0;                                                          \
    if (!(a_len == b_len)) {                                                   \
      return (0);                                                              \
    };                                                                         \
    while ((index < a_len)) {                                                  \
      if (!prefix##_nearly_equal_p((*(a + index)), (*(b + index)),             \
                                   error_margin)) {                            \
        return (0);                                                            \
      };                                                                       \
      index = (1 + index);                                                     \
    };                                                                         \
    return (1);                                                                \
  }
/** approximate float comparison. margin is a factor and is low for low accepted
   differences. http://floating-point-gui.de/errors/comparison/ */
boolean f64_nearly_equal_p(f64_s a, f64_s b, f64_s margin) {
  if ((a == b)) {
    return (1);
  } else {
    f64_s diff = fabs((a - b));
    return (((((0 == a)) || ((0 == b)) || (diff < DBL_MIN))
                 ? (diff < (margin * DBL_MIN))
                 : ((diff / fmin((fabs(a) + fabs(b)), DBL_MAX)) < margin)));
  };
};
/** approximate float comparison. margin is a factor and is low for low accepted
   differences. http://floating-point-gui.de/errors/comparison/ */
boolean f32_nearly_equal_p(f32_s a, f32_s b, f32_s margin) {
  if ((a == b)) {
    return (1);
  } else {
    f32_s diff = fabs((a - b));
    return (((((0 == a)) || ((0 == b)) || (diff < FLT_MIN))
                 ? (diff < (margin * FLT_MIN))
                 : ((diff / fmin((fabs(a) + fabs(b)), FLT_MAX)) < margin)));
  };
};
define_float_array_nearly_equal_p(f32, f32_s);
define_float_array_nearly_equal_p(f64, f64_s);
define_float_sum(f32, f32_s);
define_float_sum(f64, f64_s);
/* return status handling */ enum {
  sp_status_group_alsa,
  sp_status_group_libc,
  sp_status_group_sndfile,
  sp_status_group_sp,
  sp_status_id_file_channel_mismatch,
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
  sp_status_id_undefined
};
b8 *sp_status_description(status_t a) {
  return ((
      (sp_status_group_sp == a.group)
          ? ((b8 *)((
                (sp_status_id_eof == a.id)
                    ? "end of file"
                    : ((sp_status_id_input_type == a.id)
                           ? "input argument is of wrong type"
                           : ((sp_status_id_not_implemented == a.id)
                                  ? "not implemented"
                                  : ((sp_status_id_memory == a.id)
                                         ? "memory allocation error"
                                         : ((sp_status_id_file_incompatible ==
                                             a.id)
                                                ? "file channel count or "
                                                  "sample rate is different "
                                                  "from what was requested"
                                                : ((sp_status_id_file_incomplete ==
                                                    a.id)
                                                       ? "incomplete write"
                                                       : ((sp_status_id_port_type ==
                                                           a.id)
                                                              ? "incompatible "
                                                                "port type"
                                                              : "")))))))))
          : ((sp_status_group_alsa == a.group)
                 ? ((b8 *)(sf_error_number(a.id)))
                 : ((sp_status_group_sndfile == a.group)
                        ? ((b8 *)(sf_error_number(a.id)))
                        : ((b8 *)(""))))));
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
               : ((sp_status_group_alsa == a.group)
                      ? ((b8 *)("alsa"))
                      : ((sp_status_group_sndfile == a.group)
                             ? ((b8 *)("sndfile"))
                             : ((b8 *)("unknown"))))));
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
#define sp_sample_type sp_sample_type_f64
#define sp_float_t f64_s
#define sp_default_sample_rate 16000
#define sp_default_channel_count 1
#define sp_default_alsa_enable_soft_resample 1
#define sp_default_alsa_latency 128
#if (sp_sample_type == sp_sample_type_f64)
#define sp_sample_t f64_s
#define sample_reverse_endian __bswap_64
#define sp_sample_sum f64_sum
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_FLOAT64_LE
#else
#if (sp_sample_type_f32 == sp_sample_type)
#define sp_sample_t f32_s
#define sample_reverse_endian __bswap_32
#define sp_sample_sum f32_sum
#define sp_alsa_snd_pcm_format SND_PCM_FORMAT_FLOAT_LE
#endif
#endif
#define sp_octets_to_samples(a) (a / sizeof(sp_sample_t))
#define sp_samples_to_octets(a) (a * sizeof(sp_sample_t))
#define duration_to_sample_count(seconds, sample_rate) (seconds * sample_rate)
#define sample_count_to_duration(sample_count, sample_rate)                    \
  (sample_count / sample_rate)
typedef struct {
  b8 type;
  b8 flags;
  b32 sample_rate;
  b32 channel_count;
  b0 *data;
} sp_port_t;
#define sp_port_type_alsa 0
#define sp_port_type_file 1
#define sp_port_bit_input 1
#define sp_port_bit_output 2
#define sp_port_bit_position 4
#define sp_port_bit_closed 8
status_t sp_port_read(sp_sample_t **result, sp_port_t *port, b32 sample_count);
status_t sp_port_write(sp_port_t *port, size_t sample_count,
                       sp_sample_t **channel_data);
status_t sp_port_position(size_t *result, sp_port_t *port);
status_t sp_port_set_position(sp_port_t *port, size_t sample_index);
status_t sp_file_open(sp_port_t *result, b8 *path, b32 channel_count,
                      b32 sample_rate);
status_t sp_alsa_open(sp_port_t *result, b8 *device_name, boolean input_p,
                      b32 channel_count, b32 sample_rate, b32_s latency);
status_t sp_port_close(sp_port_t *a);
/** set sph/status object to group sp and id memory-error and goto "exit" label
 * if "a" is 0 */
#define sp_alloc_require(a)                                                    \
  if (!a) {                                                                    \
    status_set_both_goto(sp_status_group_sp, sp_status_id_memory);             \
  }
;
#define sp_alloc_set(a, octet_count)                                           \
  a = malloc(octet_count);                                                     \
  sp_alloc_require(a)
#define sp_alloc_set_zero(a, octet_count)                                      \
  a = calloc(octet_count, 1);                                                  \
  sp_alloc_require(a)
#define sp_alloc_define(id, type, octet_count)                                 \
  type id;                                                                     \
  sp_alloc_set(id, octet_count)
#define sp_alloc_define_zero(id, type, octet_count)                            \
  type id;                                                                     \
  sp_alloc_set_zero(id, octet_count)
#define sp_alloc_define_samples(id, sample_count)                              \
  sp_alloc_define(id, sp_sample_t *, (sample_count * sizeof(sp_sample_t)))
#define sp_alloc_set_samples(a, sample_count)                                  \
  sp_alloc_set(a, (sample_count * sizeof(sp_sample_t)))
#define sp_alloc_define_samples_zero(id, sample_count)                         \
  sp_alloc_define_zero(id, sp_sample_t *,                                      \
                       sp_samples_to_octets(sizeof(sp_sample_t)))
#define sp_alloc_set_samples_zero(a, sample_count)                             \
  sp_alloc_set_zero(a, sp_samples_to_octets(sample_count))
sp_sample_t **sp_alloc_channel_array(b32 channel_count, b32 sample_count);
/* depends on sph.sc */
#ifndef sc_included_string_h
#include <string.h>
#define sc_included_string_h
#endif
#ifndef sc_included_stdlib_h
#include <stdlib.h>
#define sc_included_stdlib_h
#endif
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
#define file_exists_p(path) !(access(path, F_OK) == -1)
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
#define inc(a) a = (1 + a)
#define dec(a) a = (a - 1)
/** return an array for channels with data arrays for each channel.
  returns zero if memory could not be allocated */
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
    if (!channel) {
      local_memory_free;
      return (0);
    };
    local_memory_add(channel);
    (*(result + channel_count)) = channel;
  };
  return (result);
};
#define set_optional_number(a, default)                                        \
  if ((a < 0)) {                                                               \
    a = default;                                                               \
  }
/** choose a default when number is negative */
#define optional_number(a, default) ((a < 0) ? default : a)
;
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
define_sp_interleave(sp_interleave, sp_sample_t,
                     { (*(b + b_size)) = (*((*(a + channel)) + a_size)); });
define_sp_interleave(sp_deinterleave, sp_sample_t,
                     { (*((*(a + channel)) + a_size)) = (*(b + b_size)); });
/* -- file */ b32 sp_file_sf_format = (SF_FORMAT_AU | SF_FORMAT_FLOAT);
status_t sp_file_close(sp_port_t *port) {
  status_init;
  status.id = sf_close(((SNDFILE *)((*port).data)));
  if (!status.id) {
    (*port).flags = (sp_port_bit_closed | (*port).flags);
  };
  return (status);
};
status_t sp_file_open(sp_port_t *result, b8 *path, b32 channel_count,
                      b32 sample_rate) {
  status_init;
  SF_INFO info;
  SNDFILE *file;
  int mode;
  info.format = sp_file_sf_format;
  info.channels = channel_count;
  info.samplerate = sample_rate;
  mode = SFM_RDWR;
  file = sf_open(path, mode, &info);
  if (!file) {
    status_set_both_goto(sp_status_group_sndfile, sf_error(file));
  };
  b8 bit_position = (info.seekable ? sp_port_bit_position : 0);
  (*result).channel_count = channel_count;
  (*result).sample_rate = sample_rate;
  (*result).type = sp_port_type_file;
  (*result).flags = (sp_port_bit_input | sp_port_bit_output | bit_position);
  (*result).data = file;
exit:
  return (status);
};
status_t sp_file_write(sp_port_t *port, size_t sample_count,
                       sp_sample_t **channel_data) {
  status_init;
  local_memory_init(1);
  b32 channel_count = (*port).channel_count;
  SNDFILE *file = (*port).data;
  size_t interleaved_size =
      (channel_count * sample_count * sizeof(sp_sample_t *));
  sp_alloc_define(interleaved, sp_sample_t *, interleaved_size);
  local_memory_add(interleaved);
  sp_interleave(channel_data, interleaved, sample_count, channel_count);
  sf_count_t result_count = sf_writef_double(file, interleaved, sample_count);
  if (!(sample_count == result_count)) {
    status_set_both_goto(sp_status_group_sp, sp_status_id_file_incomplete);
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
  sp_alloc_define(interleaved, sp_sample_t *, interleaved_size);
  local_memory_add(interleaved);
  sf_count_t result_count =
      sf_readf_double(((SNDFILE *)((*port).data)), interleaved, sample_count);
  if (!(interleaved_size == result_count)) {
    status_set_both(sp_status_group_sp, sp_status_id_eof);
  };
  sp_deinterleave(result, interleaved, result_count, channel_count);
exit:
  local_memory_free;
  return (status);
};
status_t sp_file_set_position(sp_port_t *port, size_t a) {
  status_init;
  SNDFILE *file = (*port).data;
  sf_count_t count = sf_seek(file, a, SEEK_SET);
  if ((count == -1)) {
    status_set_both_goto(sp_status_group_sndfile, sf_error(file));
  };
exit:
  return (status);
};
status_t sp_file_position(size_t *result, sp_port_t *port) {
  status_init;
  SNDFILE *file = (*port).data;
  sf_count_t count = sf_seek(file, 0, SEEK_CUR);
  if ((count == -1)) {
    status_set_both_goto(sp_status_group_sndfile, sf_error(file));
  };
  (*result) = count;
exit:
  return (status);
};
/* -- alsa */
/** open alsa sound output for capture or playback */
status_t sp_alsa_open(sp_port_t *result, b8 *device_name, boolean input_p,
                      b32 channel_count, b32 sample_rate, b32_s latency) {
  status_init;
  if (!device_name) {
    device_name = "default";
  };
  set_optional_number(latency, sp_default_alsa_latency);
  snd_pcm_t *alsa_port = 0;
  sp_alsa_status_require_x(snd_pcm_open(
      &alsa_port, device_name,
      (input_p ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK), 0));
  sp_alsa_status_require_x(snd_pcm_set_params(
      alsa_port, sp_alsa_snd_pcm_format, SND_PCM_ACCESS_RW_NONINTERLEAVED,
      channel_count, sample_rate, sp_default_alsa_enable_soft_resample,
      latency));
  (*result).type = sp_port_type_alsa;
  (*result).flags = (input_p ? sp_port_bit_input : sp_port_bit_output);
  (*result).data = ((b0 *)(alsa_port));
exit:
  if ((status_failure_p && alsa_port)) {
    snd_pcm_close(alsa_port);
  };
  return (status);
};
status_t sp_alsa_write(sp_port_t *port, size_t sample_count,
                       sp_sample_t **channel_data) {
  status_init;
  snd_pcm_t *alsa_port = (*port).data;
  snd_pcm_sframes_t frames_written = snd_pcm_writen(
      alsa_port, ((b0 **)(channel_data)), ((snd_pcm_uframes_t)(sample_count)));
  if (((frames_written < 0) &&
       (snd_pcm_recover(alsa_port, frames_written, 0) < 0))) {
    status_set_both(sp_status_group_alsa, frames_written);
  };
  return (status);
};
status_t sp_alsa_read(sp_sample_t **result, sp_port_t *port, b32 sample_count) {
  status_init;
  snd_pcm_t *alsa_port = (*port).data;
  snd_pcm_sframes_t frames_read =
      snd_pcm_readn(alsa_port, ((b0 **)(result)), sample_count);
  if (((frames_read < 0) && (snd_pcm_recover(alsa_port, frames_read, 0) < 0))) {
    status_set_both(sp_status_group_alsa, frames_read);
  };
  return (status);
};
/* -- sc-port */ boolean sp_port_position_p(sp_port_t *a) {
  return ((sp_port_bit_position & (*a).flags));
};
boolean sp_port_input_p(sp_port_t *a) {
  return ((sp_port_bit_input & (*a).flags));
};
boolean sp_port_output_p(sp_port_t *a) {
  return ((sp_port_bit_output & (*a).flags));
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
status_t sp_port_write(sp_port_t *port, size_t sample_count,
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
  if ((sp_port_type_alsa == (*a).type)) {
    sp_alsa_status_require_x(snd_pcm_close(((snd_pcm_t *)((*a).data))));
  } else {
    if ((sp_port_type_file == (*a).type)) {
      status = sp_file_close(a);
    };
  };
  if (status_success_p) {
    (*a).flags = (sp_port_bit_closed | (*a).flags);
  };
exit:
  return (status);
};
#define sp_port_not_implemented                                                \
  status_init;                                                                 \
  status_set_both(sp_status_group_sp, sp_status_id_not_implemented);           \
  return (status)
status_t sp_port_position(size_t *result, sp_port_t *port) {
  if ((sp_port_type_file == (*port).type)) {
    return (sp_file_position(result, port));
  } else {
    if ((sp_port_type_alsa == (*port).type)) {
      sp_port_not_implemented;
    };
  };
};
status_t sp_port_set_position(sp_port_t *port, size_t sample_index) {
  if ((sp_port_type_file == (*port).type)) {
    return (sp_file_set_position(port, sample_index));
  } else {
    if ((sp_port_type_alsa == (*port).type)) {
      sp_port_not_implemented;
    };
  };
};
/** lower precision version of sin() that is faster to compute */
sp_sample_t sp_sin_lq(sp_sample_t a) {
  sp_sample_t b = (4 / M_PI);
  sp_sample_t c = (-4 / (M_PI * M_PI));
  return ((((b * a) + (c * a * abs(a)))));
};
/** the normalised sinc function */
sp_sample_t sp_sinc(sp_sample_t a) {
  return (((0 == a) ? 1 : (sin((M_PI * a)) / (M_PI * a))));
};
/* write samples for a sine wave into dest.
   sample-duration: in seconds
   freq: radian frequency
   phase: phase offset
   amp: amplitude. 0..1
   defines sp-sine, sp-sine-lq */
#define define_sp_sine(id, sin)                                                \
  b0 id(sp_sample_t *dest, b32 len, sp_float_t sample_duration,                \
        sp_float_t freq, sp_float_t phase, sp_float_t amp) {                   \
    b32 index = 0;                                                             \
    while ((index <= len)) {                                                   \
      (*(dest + index)) = (amp * sin((freq * phase * sample_duration)));       \
      inc(phase);                                                              \
      inc(index);                                                              \
    };                                                                         \
  }
define_sp_sine(sp_sine, sin);
define_sp_sine(sp_sine_lq, sp_sin_lq);
/* routines that take sample arrays as input and process them.
  depends on base.sc */
#define kiss_fft_scalar sp_sample_t
/** sp-float-t integer -> sp-float-t
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
  sp_float_t freq;
  sp_float_t transition;
} sp_windowed_sinc_state_t;
size_t sp_windowed_sinc_ir_length(sp_float_t transition);
b0 sp_windowed_sinc_ir(sp_sample_t **result, size_t *result_len,
                       b32 sample_rate, sp_float_t freq, sp_float_t transition);
b0 sp_windowed_sinc_state_destroy(sp_windowed_sinc_state_t *state);
b8 sp_windowed_sinc_state_create(b32 sample_rate, sp_float_t freq,
                                 sp_float_t transition,
                                 sp_windowed_sinc_state_t **state);
status_i_t sp_windowed_sinc(sp_sample_t *result, sp_sample_t *source,
                            size_t source_len, b32 sample_rate, sp_float_t freq,
                            sp_float_t transition,
                            sp_windowed_sinc_state_t **state);
sp_float_t sp_window_blackman(sp_float_t a, size_t width);
b0 sp_spectral_inversion_ir(sp_sample_t *a, size_t a_len);
b0 sp_spectral_reversal_ir(sp_sample_t *a, size_t a_len);
status_t sp_fft(sp_sample_t *result, b32 result_len, sp_sample_t *source,
                b32 source_len);
status_t sp_ifft(sp_sample_t *result, b32 result_len, sp_sample_t *source,
                 b32 source_len);
status_i_t sp_moving_average(sp_sample_t *result, sp_sample_t *source,
                             b32 source_len, sp_sample_t *prev, b32 prev_len,
                             sp_sample_t *next, b32 next_len, b32 start,
                             b32 end, b32 distance);
b0 sp_convolve_one(sp_sample_t *result, sp_sample_t *a, size_t a_len,
                   sp_sample_t *b, size_t b_len);
b0 sp_convolve(sp_sample_t *result, sp_sample_t *a, size_t a_len,
               sp_sample_t *b, size_t b_len, sp_sample_t *carryover,
               size_t carryover_len);
#ifndef sc_included_kiss_fft
#include <kiss_fft.h>
#define sc_included_kiss_fft
#endif
#ifndef sc_included_kiss_fftr
#include <tools/kiss_fftr.h>
#define sc_included_kiss_fftr
#endif
status_t sp_fft(sp_sample_t *result, b32 result_len, sp_sample_t *source,
                b32 source_len) {
  sp_status_init;
  local_memory_init(2);
  kiss_fftr_cfg fftr_state = kiss_fftr_alloc(result_len, 0, 0, 0);
  if (!fftr_state) {
    status_set_id_goto(sp_status_id_memory);
  };
  local_memory_add(fftr_state);
  sp_alloc_define(out, kiss_fft_cpx *, (result_len * sizeof(kiss_fft_cpx)));
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
  sp_alloc_define(in, kiss_fft_cpx *, (source_len * sizeof(kiss_fft_cpx)));
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
status_i_t sp_moving_average(sp_sample_t *result, sp_sample_t *source,
                             b32 source_len, sp_sample_t *prev, b32 prev_len,
                             sp_sample_t *next, b32 next_len, b32 start,
                             b32 end, b32 radius) {
  if (!source_len) {
    return (1);
  };
  b32 left;
  b32 right;
  b32 width = (1 + (2 * radius));
  sp_sample_t *window = 0;
  b32 window_index;
  if (!(((start >= radius)) && (((start + radius + 1) <= source_len)))) {
    window = malloc((width * sizeof(sp_sample_t)));
    if (!window) {
      return (1);
    };
  };
  while ((start <= end)) {
    if ((((start >= radius)) && (((start + radius + 1) <= source_len)))) {
      (*result) = (sp_sample_sum(((source + start) - radius), width) / width);
    } else {
      window_index = 0;
      if ((start < radius)) {
        right = (radius - start);
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
        left = (start - radius);
      };
      right = (start + radius);
      if ((right >= source_len)) {
        right = (source_len - 1);
      };
      while ((left <= right)) {
        (*(window + window_index)) = (*(source + left));
        inc(window_index);
        inc(left);
      };
      right = (start + radius);
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
      (*result) = (sp_sample_sum(window, width) / width);
    };
    inc(result);
    inc(start);
  };
  free(window);
  return (0);
};
sp_float_t sp_window_blackman(sp_float_t a, size_t width) {
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
/* implementation of a hq windowed sinc filter with a blackman window (common
  truncated version) for continuous streams. variable sample-rate, cutoff radian
  frequency and transition band width per call. depends on some sp functions
  defined in main.sc and sph-sp.sc */
/** approximate impulse response length for a transition factor and
  ensure that the length is odd */
size_t sp_windowed_sinc_ir_length(sp_float_t transition) {
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
                       b32 sample_rate, sp_float_t freq,
                       sp_float_t transition) {
  size_t len = sp_windowed_sinc_ir_length(transition);
  (*result_len) = len;
  sp_float_t center_index = ((len - 1.0) / 2.0);
  sp_float_t cutoff = sp_windowed_sinc_cutoff(freq, sample_rate);
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
  sp_float_t result_sum = sp_sample_sum(result_temp, len);
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
b8 sp_windowed_sinc_state_create(b32 sample_rate, sp_float_t freq,
                                 sp_float_t transition,
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
                            size_t source_len, b32 sample_rate, sp_float_t freq,
                            sp_float_t transition,
                            sp_windowed_sinc_state_t **state) {
  if (sp_windowed_sinc_state_create(sample_rate, freq, transition, state)) {
    return (1);
  };
  sp_convolve(result, source, source_len, (*(*state)).ir, (*(*state)).ir_len,
              (*(*state)).data, (*(*state)).ir_len_prev);
  return (0);
};
#define scm_first SCM_CAR
#define scm_tail SCM_CDR
#define scm_c_define_procedure_c_init SCM scm_c_define_procedure_c_temp
#define scm_is_undefined(a) (SCM_UNDEFINED == a)
#define scm_c_define_procedure_c(name, required, optional, rest, c_function,   \
                                 documentation)                                \
  scm_c_define_procedure_c_temp =                                              \
      scm_c_define_gsubr(name, required, optional, rest, c_function);          \
  scm_set_procedure_property_x(scm_c_define_procedure_c_temp,                  \
                               scm_from_locale_symbol("documentation"),        \
                               scm_from_locale_string(documentation))
b0 scm_debug_log(SCM value) {
  scm_call_2(scm_variable_ref(scm_c_lookup("write")), value,
             scm_current_output_port());
  scm_newline(scm_current_output_port());
};
SCM scm_c_bytevector_take(size_t size_octets, b8 *a) {
  SCM r = scm_c_make_bytevector(size_octets);
  memcpy(SCM_BYTEVECTOR_CONTENTS(r), a, size_octets);
  return (r);
};
#define scm_c_list_each(list, e, body)                                         \
  while (!scm_is_null(list)) {                                                 \
    e = scm_first(list);                                                       \
    body;                                                                      \
    list = scm_tail(list);                                                     \
  }
#define inc(a) a = (1 + a)
#define dec(a) a = (a - 1)
/** raise an exception with error information set as an alist with given values
 */
#define scm_c_error(name, description)                                         \
  scm_call_1(scm_rnrs_raise,                                                   \
             scm_list_3(scm_from_latin1_symbol(name),                          \
                        scm_cons(scm_from_latin1_symbol("description"),        \
                                 scm_from_utf8_string(description)),           \
                        scm_cons(scm_from_latin1_symbol("c-routine"),          \
                                 scm_from_latin1_symbol(__FUNCTION__))))
;
#define status_to_scm_error(a)                                                 \
  scm_c_error(sp_status_name(a), sp_status_description(a))
#define status_to_scm(result)                                                  \
  (status_success_p ? result : status_to_scm_error(status))
/** call scm-c-error if status is not success or return result */
#define status_to_scm_return(result) return (status_to_scm(result))
;
#define scm_sp_object_type_init                                                \
  scm_type_sp_object = scm_make_smob_type("sp-object", 0)
#define scm_set_smob_print                                                     \
  scm_type_sp_object scm_sp_object_print scm_set_smob_free(scm_type_sp_object, \
                                                           scm_sp_object_free)
/** integer -> string */
#define sp_port_type_to_name(a)                                                \
  ((sp_port_type_file == a) ? "file"                                           \
                            : ((sp_port_type_alsa == a) ? "alsa" : "unknown"))
;
#define sp_object_type_port 0
#define sp_object_type_windowed_sinc 1
#define scm_sp_object_type SCM_SMOB_FLAGS
#define scm_sp_object_data SCM_SMOB_DATA
#define optional_sample_rate(a) (scm_is_undefined(a) ? -1 : scm_to_int32(a))
#define optional_channel_count(a) (scm_is_undefined(a) ? -1 : scm_to_int32(a))
scm_t_bits scm_type_sp_object;
SCM scm_rnrs_raise;
/** get a guile scheme object for channel data sample arrays. returns a list of
  f64vectors. eventually frees given data arrays */
SCM scm_take_channel_data(sp_sample_t **a, b32 channel_count,
                          b32 sample_count) {
  SCM result = SCM_EOL;
  while (channel_count) {
    dec(channel_count);
    result = scm_cons(scm_take_f64vector((*(a + channel_count)), sample_count),
                      result);
  };
  free(a);
  return (result);
};
/** only the result array is allocated, data is referenced to the scm vectors */
sp_sample_t **scm_to_channel_data(SCM a, b32 *channel_count,
                                  size_t *sample_count) {
  (*channel_count) = scm_to_uint32(scm_length(a));
  if (!(*channel_count)) {
    return (0);
  };
  sp_sample_t **result = malloc(((*channel_count) * sizeof(sp_sample_t *)));
  if (!result) {
    return (0);
  };
  (*sample_count) = sp_octets_to_samples(SCM_BYTEVECTOR_LENGTH(scm_first(a)));
  size_t index = 0;
  while (!scm_is_null(a)) {
    (*(result + index)) =
        ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(scm_first(a))));
    inc(index);
    a = scm_tail(a);
  };
  return (result);
};
/** sp-object type for storing arbitrary pointers */
SCM scm_sp_object_create(b0 *pointer, b8 sp_object_type) {
  SCM result = scm_new_smob(scm_type_sp_object, ((scm_t_bits)(pointer)));
  SCM_SET_SMOB_FLAGS(result, ((scm_t_bits)(sp_object_type)));
  return (result);
};
int scm_sp_object_print(SCM a, SCM output_port, scm_print_state *print_state) {
  char *result = malloc((70 + 10 + 7 + 10 + 10 + 2 + 2));
  if (!result) {
    return (0);
  };
  b8 type = scm_sp_object_type(a);
  if ((sp_object_type_port == type)) {
    sp_port_t *sp_port = ((sp_port_t *)(scm_sp_object_data(a)));
    sprintf(result,
            "#<sp-port %lx type:%s sample-rate:%d channel-count:%d closed?:%s "
            "input?:%s>",
            ((b0 *)(a)), sp_port_type_to_name((*sp_port).type),
            (*sp_port).sample_rate, (*sp_port).channel_count,
            ((sp_port_bit_closed & (*sp_port).flags) ? "#t" : "#f"),
            ((sp_port_bit_input & (*sp_port).flags) ? "#t" : "#f"));
  } else {
    if ((sp_object_type_windowed_sinc == type)) {
      sprintf(result, "#<sp-state %lx type:windowed-sinc>", ((b0 *)(a)));
    } else {
      sprintf(result, "#<sp %lx>", ((b0 *)(a)));
    };
  };
  scm_display(scm_take_locale_string(result), output_port);
  return (0);
};
size_t scm_sp_object_free(SCM a) {
  b8 type = SCM_SMOB_FLAGS(a);
  b0 *data = ((b0 *)(scm_sp_object_data(a)));
  if ((sp_object_type_windowed_sinc == type)) {
    sp_windowed_sinc_state_destroy(data);
  } else {
    if ((sp_object_type_port == type)) {
      sp_port_close(data);
    };
  };
  return (0);
};
SCM scm_float_nearly_equal_p(SCM a, SCM b, SCM margin) {
  return (scm_from_bool(f64_nearly_equal_p(scm_to_double(a), scm_to_double(b),
                                           scm_to_double(margin))));
};
SCM scm_f64vector_sum(SCM a, SCM start, SCM end) {
  return (scm_from_double(
      f64_sum(((scm_is_undefined(start) ? 0 : scm_to_size_t(start)) +
               ((f64_s *)(SCM_BYTEVECTOR_CONTENTS(a)))),
              ((scm_is_undefined(end) ? SCM_BYTEVECTOR_LENGTH(a)
                                      : (end - (1 + start))) *
               sizeof(f64_s)))));
};
SCM scm_f32vector_sum(SCM a, SCM start, SCM end) {
  return (scm_from_double(
      f32_sum(((scm_is_undefined(start) ? 0 : scm_to_size_t(start)) +
               ((f32_s *)(SCM_BYTEVECTOR_CONTENTS(a)))),
              ((scm_is_undefined(end) ? SCM_BYTEVECTOR_LENGTH(a)
                                      : (end - (1 + start))) *
               sizeof(f32_s)))));
};
#define scm_sp_port(a) ((sp_port_t *)(scm_sp_object_data(a)))
SCM scm_sp_port_p(SCM a) {
  return (scm_from_bool((SCM_SMOB_PREDICATE(scm_type_sp_object, a) &&
                         ((sp_object_type_port == scm_sp_object_type(a))))));
};
SCM scm_sp_port_channel_count(SCM port) {
  return (scm_from_uint32((*scm_sp_port(port)).channel_count));
};
SCM scm_sp_port_sample_rate(SCM port) {
  return (scm_from_uint32((*scm_sp_port(port)).sample_rate));
};
SCM scm_sp_port_position_p(SCM port) {
  return (scm_from_bool((sp_port_bit_position & (*scm_sp_port(port)).flags)));
};
SCM scm_sp_port_input_p(SCM port) {
  return (scm_from_bool((sp_port_bit_input & (*scm_sp_port(port)).flags)));
};
/** returns the current port position in number of octets */
SCM scm_sp_port_position(SCM port) {
  size_t position;
  sp_port_position(&position, scm_sp_port(port));
  return (scm_from_size_t(position));
};
SCM scm_sp_port_close(SCM a) {
  status_init;
  status = sp_port_close(scm_sp_port(a));
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_port_read(SCM scm_port, SCM scm_sample_count) {
  status_init;
  sp_port_t *port = scm_sp_port(scm_port);
  b32 sample_count = scm_to_uint32(scm_sample_count);
  b32 channel_count = (*port).channel_count;
  sp_sample_t **data = sp_alloc_channel_array(channel_count, sample_count);
  sp_status_require_alloc(data);
  status_require_x(sp_port_read(data, port, sample_count));
  SCM result = scm_take_channel_data(data, channel_count, sample_count);
exit:
  status_to_scm_return(result);
};
SCM scm_sp_port_write(SCM scm_port, SCM scm_channel_data,
                      SCM scm_sample_count) {
  status_init;
  sp_port_t *port = scm_sp_port(scm_port);
  local_memory_init(1);
  b32 channel_count;
  size_t sample_count;
  sp_sample_t **data =
      scm_to_channel_data(scm_channel_data, &channel_count, &sample_count);
  sp_status_require_alloc(data);
  local_memory_add(data);
  status_require_x(sp_port_write(port, sample_count, data));
exit:
  local_memory_free;
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_port_set_position(SCM scm_port, SCM scm_sample_offset) {
  status_init;
  status = sp_port_set_position(scm_sp_port(scm_port),
                                scm_to_uint64(scm_sample_offset));
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_file_open(SCM scm_path, SCM scm_channel_count, SCM scm_sample_rate) {
  status_init;
  b8 *path = scm_to_locale_string(scm_path);
  b32 channel_count = scm_to_uint32(scm_channel_count);
  b32 sample_rate = scm_to_uint32(scm_sample_rate);
  sp_alloc_define(sp_port, sp_port_t *, sizeof(sp_port_t));
  status_require_x(sp_file_open(sp_port, path, channel_count, sample_rate));
  SCM result = scm_sp_object_create(sp_port, sp_object_type_port);
exit:
  status_to_scm_return(result);
};
SCM scm_sp_alsa_open(SCM scm_device_name, SCM scm_input_p,
                     SCM scm_channel_count, SCM scm_sample_rate,
                     SCM scm_latency) {
  status_init;
  b8 *device_name = scm_to_locale_string(scm_device_name);
  boolean input_p = scm_to_bool(scm_input_p);
  b32 channel_count = scm_to_uint32(scm_channel_count);
  b32 sample_rate = scm_to_uint32(scm_sample_rate);
  b32 latency = scm_to_uint32(scm_latency);
  sp_alloc_define_zero(sp_port, sp_port_t *, sizeof(sp_port_t));
  status_require_x(sp_alsa_open(sp_port, device_name, input_p, channel_count,
                                sample_rate, latency));
  SCM result = scm_sp_object_create(sp_port, sp_object_type_port);
exit:
  status_to_scm_return(result);
};
b0 init_sp_io() {
  scm_c_define_procedure_c_init;
  scm_c_define_procedure_c("sp-port-close", 1, 0, 0, scm_sp_port_close,
                           "sp-port -> boolean");
  scm_c_define_procedure_c("sp-port-input?", 1, 0, 0, scm_sp_port_input_p,
                           "sp-port -> boolean");
  scm_c_define_procedure_c("sp-port-position?", 1, 0, 0, scm_sp_port_position_p,
                           "sp-port -> boolean");
  scm_c_define_procedure_c("sp-port-position", 1, 0, 0, scm_sp_port_position,
                           "sp-port -> integer/boolean");
  scm_c_define_procedure_c("sp-port-channel-count", 1, 0, 0,
                           scm_sp_port_channel_count, "sp-port -> integer");
  scm_c_define_procedure_c("sp-port-sample-rate", 1, 0, 0,
                           scm_sp_port_sample_rate,
                           "sp-port -> integer/boolean");
  scm_c_define_procedure_c("sp-port?", 1, 0, 0, scm_sp_port_p,
                           "sp-port -> boolean");
  scm_c_define_procedure_c(
      "sp-alsa-open", 5, 0, 0, scm_sp_alsa_open,
      "device-name input? channel-count sample-rate latency -> sp-port");
  scm_c_define_procedure_c("sp-file-open", 3, 0, 0, scm_sp_file_open,
                           "path channel-count sample-rate -> sp-port");
  scm_c_define_procedure_c("sp-port-write", 2, 1, 0, scm_sp_port_write,
                           "sp-port (f32vector ...):channel-data "
                           "[integer:sample-count] -> boolean\n    write "
                           "sample data to the channels of port");
  scm_c_define_procedure_c(
      "sp-port-read", 2, 0, 0, scm_sp_port_read,
      "sp-port integer:sample-count -> (f32vector ...):channel-data");
  scm_c_define_procedure_c(
      "sp-port-set-position", 2, 0, 0, scm_sp_port_set_position,
      "sp-port integer:sample-offset -> boolean\n    sample-offset can be "
      "negative, in which case it is from the end of the port");
};
/** defines scm-sp-sine!, scm-sp-sine-lq! */
#define define_sp_sine_x(scm_id, id)                                           \
  SCM scm_id(SCM data, SCM len, SCM sample_duration, SCM freq, SCM phase,      \
             SCM amp) {                                                        \
    id(((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(data))), scm_to_uint32(len),   \
       scm_to_double(sample_duration), scm_to_double(freq),                    \
       scm_to_double(phase), scm_to_double(amp));                              \
    return (SCM_UNSPECIFIED);                                                  \
  }
;
define_sp_sine_x(scm_sp_sine_x, sp_sine);
define_sp_sine_x(scm_sp_sine_lq_x, sp_sine);
b0 init_sp_generate() {
  scm_c_define_procedure_c_init;
  scm_c_define_procedure_c("sp-sine!", 6, 0, 0, scm_sp_sine_x,
                           "data len sample-duration freq phase amp -> "
                           "unspecified\n    f32vector integer integer "
                           "rational rational rational rational");
  scm_c_define_procedure_c("sp-sine-lq!", 6, 0, 0, scm_sp_sine_lq_x,
                           "data len sample-duration freq phase amp -> "
                           "unspecified\n    f32vector integer integer "
                           "rational rational rational rational\n    faster, "
                           "lower precision version of sp-sine!.\n    "
                           "currently faster by a factor of about 2.6");
};
SCM scm_sp_convolve_x(SCM result, SCM a, SCM b, SCM carryover,
                      SCM carryover_len) {
  b32 a_len = sp_octets_to_samples(SCM_BYTEVECTOR_LENGTH(a));
  b32 b_len = sp_octets_to_samples(SCM_BYTEVECTOR_LENGTH(b));
  sp_convolve(((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(result))),
              ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(a))), a_len,
              ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(b))), b_len,
              ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(carryover))),
              scm_to_size_t(carryover_len));
  return (SCM_UNSPECIFIED);
};
#define optional_samples(a, a_len, scm)                                        \
  if (scm_is_true(scm)) {                                                      \
    a = ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(scm)));                       \
    a_len = sp_octets_to_samples(SCM_BYTEVECTOR_LENGTH(scm));                  \
  } else {                                                                     \
    a = 0;                                                                     \
    a_len = 0;                                                                 \
  }
#define optional_index(a, default)                                             \
  ((!scm_is_undefined(start) && scm_is_true(start)) ? scm_to_uint32(a)         \
                                                    : default)
SCM scm_sp_moving_average_x(SCM result, SCM source, SCM scm_prev, SCM scm_next,
                            SCM distance, SCM start, SCM end) {
  b32 source_len = sp_octets_to_samples(SCM_BYTEVECTOR_LENGTH(source));
  sp_sample_t *prev;
  b32 prev_len;
  sp_sample_t *next;
  b32 next_len;
  optional_samples(prev, prev_len, scm_prev);
  optional_samples(next, next_len, scm_next);
  sp_moving_average(
      ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(result))),
      ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(source))), source_len, prev,
      prev_len, next, next_len, optional_index(start, 0),
      optional_index(end, (source_len - 1)), scm_to_uint32(distance));
  return (SCM_UNSPECIFIED);
};
SCM scm_sp_windowed_sinc_state_create(SCM sample_rate, SCM freq, SCM transition,
                                      SCM old_state) {
  sp_windowed_sinc_state_t *state;
  if (scm_is_true(old_state)) {
    state = ((sp_windowed_sinc_state_t *)(scm_sp_object_data(old_state)));
  } else {
    state = 0;
  };
  sp_windowed_sinc_state_create(scm_to_uint32(sample_rate), scm_to_double(freq),
                                scm_to_double(transition), &state);
  return ((scm_is_true(old_state)
               ? old_state
               : scm_sp_object_create(state, sp_object_type_windowed_sinc)));
};
SCM scm_sp_windowed_sinc_x(SCM result, SCM source, SCM state) {
  return (SCM_UNSPECIFIED);
};
SCM scm_sp_fft(SCM source) {
  status_init;
  b32 result_len = ((3 * SCM_BYTEVECTOR_LENGTH(source)) / 2);
  SCM result =
      scm_make_f32vector(scm_from_uint32(result_len), scm_from_uint8(0));
  status_require_x(sp_fft(((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(result))),
                          result_len,
                          ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(source))),
                          SCM_BYTEVECTOR_LENGTH(source)));
exit:
  status_to_scm_return(result);
};
SCM scm_sp_ifft(SCM source) {
  status_init;
  b32 result_len = ((SCM_BYTEVECTOR_LENGTH(source) - 1) * 2);
  SCM result =
      scm_make_f32vector(scm_from_uint32(result_len), scm_from_uint8(0));
  status_require_x(sp_ifft(((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(result))),
                           result_len,
                           ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(source))),
                           SCM_BYTEVECTOR_LENGTH(source)));
exit:
  status_to_scm_return(result);
};
b0 init_sp_transform() {
  scm_c_define_procedure_c_init;
  scm_c_define_procedure_c("sp-fft", 1, 0, 0, scm_sp_fft,
                           "f32vector:value-per-time -> "
                           "f32vector:frequencies-per-time\n    discrete "
                           "fourier transform on the input data");
  scm_c_define_procedure_c("sp-ifft", 1, 0, 0, scm_sp_ifft,
                           "f32vector:frequencies-per-time -> "
                           "f32vector:value-per-time\n    inverse discrete "
                           "fourier transform on the input data");
  scm_c_define_procedure_c("sp-moving-average!", 5, 2, 0,
                           scm_sp_moving_average_x,
                           "result source previous next distance [start end] "
                           "-> unspecified\n  f32vector f32vector f32vector "
                           "f32vector integer integer integer [integer]");
  scm_c_define_procedure_c("sp-windowed-sinc!", 7, 2, 0, scm_sp_windowed_sinc_x,
                           "result source previous next sample-rate freq "
                           "transition [start end] -> unspecified\n    "
                           "f32vector f32vector f32vector f32vector number "
                           "number integer integer -> boolean");
  scm_c_define_procedure_c("sp-convolve!", 3, 0, 0, scm_sp_convolve_x,
                           "a b state:(integer . f32vector) -> state");
};
b0 init_sp_guile() {
  init_sp_io();
  init_sp_generate();
  init_sp_transform();
  scm_c_define_procedure_c_init;
  scm_sp_object_type_init;
  scm_rnrs_raise = scm_c_public_ref("rnrs exceptions", "raise");
  scm_c_define_procedure_c("f32vector-sum", 1, 2, 0, scm_f32vector_sum,
                           "f32vector [start end] -> number");
  scm_c_define_procedure_c("f64vector-sum", 1, 2, 0, scm_f64vector_sum,
                           "f64vector [start end] -> number");
  scm_c_define_procedure_c(
      "float-nearly-equal?", 3, 0, 0, scm_float_nearly_equal_p,
      "a b margin -> boolean\n    number number number -> boolean");
};