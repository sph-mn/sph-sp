
#define debug_log_p 1
#include <alsa/asoundlib.h>
#include <libguile.h>
#include <stdio.h>
#ifndef sc_included_sph
#define sc_included_sph
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
#if debug_log_p
#define debug_log(format, ...)                                                 \
  fprintf(stderr, "%s:%d " format "\n", __func__, __LINE__, __VA_ARGS__)
#else
#define debug_log(format, ...) null
#endif
#define null ((b0)(0))
#define zero_p(a) (0 == a)
#endif
#ifndef sc_included_one
#define sc_included_one
#ifndef sc_included_sph
#define sc_included_sph
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
#if debug_log_p
#define debug_log(format, ...)                                                 \
  fprintf(stderr, "%s:%d " format "\n", __func__, __LINE__, __VA_ARGS__)
#else
#define debug_log(format, ...) null
#endif
#define null ((b0)(0))
#define zero_p(a) (0 == a)
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
#define file_exists_p(path) !(access(path, F_OK) == -1)
#define pointer_equal_p(a, b) (((b0 *)(a)) == ((b0 *)(b)))
#define free_and_set_zero(a)                                                   \
  free(a);                                                                     \
  a = 0
#define increment(a) a = (1 + a)
#define decrement(a) a = (a - 1)
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
#endif
#ifndef sc_included_guile
#define sc_included_guile
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
#endif
#ifndef sc_included_io
#define sc_included_io
#include <fcntl.h>
#define optional_samples_per_second(a)                                         \
  ((SCM_UNDEFINED == a) ? 96000 : scm_to_uint32(a))
#define optional_channel_count(a) ((SCM_UNDEFINED == a) ? 1 : scm_to_uint32(a))
#define scm_c_sp_port_p(a) SCM_SMOB_PREDICATE(sp_port_scm_type, a)
#define default_alsa_enable_soft_resample 1
#define default_alsa_latency 50
#define default_samples_per_second 16000
#define default_channel_count 1
#define sp_port_type_alsa 0
#define sp_port_type_file 1
scm_t_bits sp_port_scm_type;
SCM scm_sp_port_type_alsa;
SCM scm_sp_port_type_file;
#define define_sp_interleave(name, get_source_element)                         \
  b0 name(f32_s *interleaved, f32_s **non_interleaved, b32 channel_count,      \
          b32 non_interleaved_size) {                                          \
    b32 interleaved_size = (non_interleaved_size * channel_count);             \
    b32 current_channel;                                                       \
    while (non_interleaved_size) {                                             \
      decrement_one(non_interleaved_size);                                     \
      current_channel = channel_count;                                         \
      while (current_channel) {                                                \
        decrement_one(current_channel);                                        \
        decrement_one(interleaved_size);                                       \
        (*(interleaved + interleaved_size)) = get_source_element;              \
      };                                                                       \
    };                                                                         \
  }
#define define_sp_deinterleave(name, get_source_element)                       \
  b0 name(f32_s **non_interleaved, f32_s *interleaved, b32 channel_count,      \
          b32 non_interleaved_size) {                                          \
    b32 interleaved_size = (non_interleaved_size * channel_count);             \
    b32 current_channel;                                                       \
    while (non_interleaved_size) {                                             \
      decrement_one(non_interleaved_size);                                     \
      current_channel = channel_count;                                         \
      while (current_channel) {                                                \
        decrement_one(current_channel);                                        \
        decrement_one(interleaved_size);                                       \
        (*((*(non_interleaved + current_channel)) + non_interleaved_size)) =   \
            get_source_element;                                                \
      };                                                                       \
    };                                                                         \
  }
f32_s word_octets_reverse_f32(f32_s a) {
  f32_s r;
  b8 *b = ((b8 *)(&a));
  b8 *c = ((b8 *)(&r));
  (*c) = (*(b + 3));
  (*(c + 1)) = (*(b + 2));
  (*(c + 2)) = (*(b + 1));
  (*(c + 3)) = (*b);
  return (r);
};
define_sp_deinterleave(sp_deinterleave_n_and_swap_endian,
                       word_octets_reverse_f32((*(interleaved +
                                                  interleaved_size))));
define_sp_deinterleave(sp_deinterleave_n, (*(interleaved + interleaved_size)));
define_sp_interleave(sp_interleave_n,
                     (*((*(non_interleaved + current_channel)) +
                        non_interleaved_size)));
define_sp_interleave(
    sp_interleave_n_and_swap_endian,
    word_octets_reverse_f32((*((*(non_interleaved + current_channel)) +
                               non_interleaved_size))));
typedef struct {
  b32 samples_per_second;
  b32 channel_count;
  b8 input_p;
  b8 type;
  b8 closed_p;
  b8 position_p;
  b64 position;
  b16 position_offset;
  pointer data;
} port_data_t;
#define sp_port_type_to_name(a)                                                \
  ((sp_port_type_file == a) ? "file"                                           \
                            : ((sp_port_type_alsa == a) ? "alsa" : "unknown"))
#define sp_port_to_port_data(a) ((port_data_t *)(SCM_SMOB_DATA(a)))
int sp_port_print(SCM a, SCM output_port, scm_print_state *print_state) {
  port_data_t *port_data = ((port_data_t *)(SCM_SMOB_DATA(a)));
  char *r = malloc(114);
  sprintf(r,
          "#<sp-port %lx type:%s samples-per-second:%d channel-count:%d "
          "closed?:%s input?:%s>",
          ((pointer)(a)), sp_port_type_to_name((*port_data).type),
          (*port_data).samples_per_second, (*port_data).channel_count,
          ((*port_data).closed_p ? "#t" : "#f"),
          ((*port_data).input_p ? "#t" : "#f"));
  scm_display(scm_take_locale_string(r), output_port);
  return (0);
};
#define sp_port_scm_type_init                                                  \
  sp_port_scm_type = scm_make_smob_type("sp-port", 0);                         \
  scm_set_smob_print(sp_port_scm_type, sp_port_print)
SCM sp_port_create(b8 type, b8 input_p, b32 samples_per_second,
                   b32 channel_count, b8 position_p, b16 position_offset,
                   pointer data) {
  port_data_t *port_data = scm_gc_malloc(sizeof(port_data_t), "sp-port-data");
  (*port_data).channel_count = channel_count;
  (*port_data).samples_per_second = samples_per_second;
  (*port_data).type = type;
  (*port_data).input_p = input_p;
  (*port_data).data = data;
  (*port_data).position_p = position_p;
  (*port_data).position = 0;
  (*port_data).position_offset = position_offset;
  return (scm_new_smob(sp_port_scm_type, ((scm_t_bits)(port_data))));
};
#define scm_c_require_success_alsa(a)                                          \
  s = a;                                                                       \
  if (s) {                                                                     \
    scm_c_local_error("alsa", scm_from_locale_string(snd_strerror(s)));        \
  }
SCM scm_sp_port_close(SCM a) {
  scm_c_local_error_init;
  init_status;
  port_data_t *port_data;
  if (scm_c_sp_port_p(a)) {
    port_data = sp_port_to_port_data(a);
    if ((*port_data).closed_p) {
      scm_c_local_error("is-closed", 0);
    } else {
      let_macro(type((*port_data).type),
                if ((sp_port_type_alsa == type)) {
                  scm_c_require_success_alsa(
                      snd_pcm_close(((snd_pcm_t *)((*port_data).data))));
                } else {
                  if ((sp_port_type_file == type)) {
                    scm_c_require_success_system(
                        close(((int)((*port_data).data))));
                  };
                },
                (*port_data).closed_p = 1;);
    };
  } else {
    scm_c_local_error("type-check", 0);
  };
  return (SCM_BOOL_T);
error:
  scm_c_local_error_return;
};
SCM sp_io_alsa_open(b8 input_p, SCM device_name, SCM channel_count,
                    SCM samples_per_second, SCM latency) {
  scm_c_local_error_init;
  local_memory_init(1);
  init_status;
  snd_pcm_t *alsa_port = 0;
  char *device_name_c;
  scm_if_undefined(device_name, device_name_c = "default",
                   device_name_c = scm_to_locale_string(device_name);
                   local_memory_add(device_name_c););
  scm_c_require_success_alsa(snd_pcm_open(
      &alsa_port, device_name_c,
      (input_p ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK), 0));
  b32 latency_c = scm_if_undefined_expr(latency, default_alsa_latency,
                                        scm_to_uint32(latency));
  b32 channel_count_c =
      scm_if_undefined_expr(channel_count, 1, scm_to_uint32(channel_count));
  b32 samples_per_second_c =
      scm_if_undefined_expr(samples_per_second, default_samples_per_second,
                            scm_to_uint32(samples_per_second));
  scm_c_require_success_alsa(snd_pcm_set_params(
      alsa_port, SND_PCM_FORMAT_FLOAT_LE, SND_PCM_ACCESS_RW_NONINTERLEAVED,
      channel_count_c, samples_per_second_c, default_alsa_enable_soft_resample,
      latency_c));
  local_memory_free;
  return (sp_port_create(sp_port_type_alsa, input_p, samples_per_second_c,
                         channel_count_c, 0, 0, ((pointer)(alsa_port))));
error:
  if (alsa_port) {
    snd_pcm_close(alsa_port);
  };
  local_memory_free;
  scm_c_local_error_return;
};
SCM scm_sp_io_alsa_open_input(SCM device_name, SCM channel_count,
                              SCM samples_per_second, SCM latency) {
  return (sp_io_alsa_open(1, device_name, channel_count, samples_per_second,
                          latency));
};
SCM scm_sp_io_alsa_open_output(SCM device_name, SCM channel_count,
                               SCM samples_per_second, SCM latency) {
  return (sp_io_alsa_open(0, device_name, channel_count, samples_per_second,
                          latency));
};
b8_s file_au_write_header(int file, b32 encoding, b32 samples_per_second,
                          b32 channel_count) {
  ssize_t s;
  b32 header[7];
  (*header) = __builtin_bswap32(779316836);
  (*(header + 1)) = __builtin_bswap32(28);
  (*(header + 2)) = __builtin_bswap32(4294967295);
  (*(header + 3)) = __builtin_bswap32(encoding);
  (*(header + 4)) = __builtin_bswap32(samples_per_second);
  (*(header + 5)) = __builtin_bswap32(channel_count);
  (*(header + 6)) = 0;
  s = write(file, header, 28);
  if (!(s == 28)) {
    return (-1);
  };
  return (0);
};
b8_s file_au_read_header(int file, b32 *encoding, b32 *samples_per_second,
                         b32 *channel_count) {
  ssize_t s;
  b32 header[6];
  s = read(file, header, 24);
  if (!(((s == 24)) && (((*header) == __builtin_bswap32(779316836))))) {
    return (-1);
  };
  if ((lseek(file, __builtin_bswap32((*(header + 1))), SEEK_SET) < 0)) {
    return (-1);
  };
  (*encoding) = __builtin_bswap32((*(header + 3)));
  (*samples_per_second) = __builtin_bswap32((*(header + 4)));
  (*channel_count) = __builtin_bswap32((*(header + 5)));
  return (0);
};
SCM sp_io_file_open(SCM path, b8 input_p, SCM channel_count,
                    SCM samples_per_second) {
  int file;
  SCM r;
  b32 samples_per_second_file;
  b32 channel_count_file;
  char *path_c = scm_to_locale_string(path);
  scm_c_local_error_init;
  local_memory_init(1);
  local_memory_add(path_c);
  if (file_exists_p(path_c)) {
    file = open(path_c, O_RDWR);
    scm_c_require_success_system(file);
    b32 encoding;
    if (file_au_read_header(file, &encoding, &samples_per_second_file,
                            &channel_count_file)) {
      scm_c_local_error("header-read", 0);
    };
    if (!(encoding == 6)) {
      scm_c_local_error("wrong-encoding", scm_from_uint32(encoding));
    };
    if (!(scm_is_undefined(channel_count) ||
          ((channel_count_file == scm_to_uint32(channel_count))))) {
      scm_c_local_error(
          "incompatible",
          scm_from_locale_string("file exists but channel count is different "
                                 "from what was requested"));
    };
    if (!(scm_is_undefined(samples_per_second) ||
          ((samples_per_second_file == scm_to_uint32(samples_per_second))))) {
      scm_c_local_error(
          "incompatible",
          scm_from_locale_string("file exists but samples per second are "
                                 "different from what was requested"));
    };
    r = sp_port_create(sp_port_type_file, input_p, samples_per_second_file,
                       channel_count_file, 1, lseek(file, 0, SEEK_CUR), file);
  } else {
    file = open(path_c, (O_RDWR | O_CREAT), 384);
    scm_c_require_success_system(file);
    samples_per_second_file = (scm_is_undefined(samples_per_second)
                                   ? default_samples_per_second
                                   : scm_to_uint32(samples_per_second));
    channel_count_file =
        (scm_is_undefined(samples_per_second) ? default_channel_count
                                              : scm_to_uint32(channel_count));
    if ((file_au_write_header(file, 6, samples_per_second_file,
                              channel_count_file) < 0)) {
      scm_c_local_error("header-write", 0);
    };
    r = sp_port_create(sp_port_type_file, input_p, samples_per_second_file,
                       channel_count_file, 1, lseek(file, 0, SEEK_CUR), file);
  };
  local_memory_free;
  return (r);
error:
  local_memory_free;
  scm_c_local_error_return;
};
SCM scm_sp_io_file_open_input(SCM path) {
  scm_c_local_error_init;
  scm_c_local_error_assert("type-check", scm_is_string(path));
  return (sp_io_file_open(path, 1, SCM_UNDEFINED, SCM_UNDEFINED));
error:
  scm_c_local_error_return;
};
SCM scm_sp_io_file_open_output(SCM path, SCM channel_count,
                               SCM samples_per_second) {
  scm_c_local_error_init;
  scm_c_local_error_assert(
      "type-check",
      (scm_is_string(path) &&
       (scm_is_undefined(channel_count) || scm_is_integer(channel_count)) &&
       (scm_is_undefined(samples_per_second) ||
        scm_is_integer(samples_per_second))));
  return (sp_io_file_open(path, 0, channel_count, samples_per_second));
error:
  scm_c_local_error_return;
};
SCM scm_sp_io_alsa_write(SCM port, SCM sample_count, SCM channel_data) {
  scm_c_local_error_init;
  scm_c_local_error_assert(
      "type-check",
      (scm_c_sp_port_p(port) && scm_is_true(scm_list_p(channel_data)) &&
       !scm_is_null(channel_data) && scm_is_integer(sample_count)));
  port_data_t *port_data = sp_port_to_port_data(port);
  b32 channel_count = (*port_data).channel_count;
  b32 sample_count_c = scm_to_uint32(sample_count);
  local_memory_init(2);
  scm_c_local_define_malloc_and_size(channel_data_c, f32_s *,
                                     (channel_count * sizeof(pointer)));
  local_memory_add(channel_data_c);
  b32 index = 0;
  SCM e;
  scm_c_list_each(channel_data, e, {
    (*(channel_data_c + index)) = ((f32_s *)(SCM_BYTEVECTOR_CONTENTS(e)));
    increment_one(index);
  });
  snd_pcm_sframes_t frames_written;
  frames_written = snd_pcm_writen(((snd_pcm_t *)((*port_data).data)),
                                  ((b0 **)(channel_data_c)),
                                  ((snd_pcm_uframes_t)(sample_count_c)));
  if (((frames_written < 0) &&
       (snd_pcm_recover(((snd_pcm_t *)((*port_data).data)), frames_written, 0) <
        0))) {
    scm_c_local_error("alsa",
                      scm_from_locale_string(snd_strerror(frames_written)));
  };
  local_memory_free;
  return (SCM_BOOL_T);
error:
  local_memory_free;
  scm_c_local_error_return;
};
SCM scm_sp_io_alsa_read(SCM port, SCM sample_count) {
  local_memory_init(1);
  scm_c_local_error_init;
  scm_c_local_error_assert(
      "type-check", (scm_c_sp_port_p(port) && scm_is_integer(sample_count)));
  port_data_t *port_data = sp_port_to_port_data(port);
  b32 channel_count_c = (*port_data).channel_count;
  b64 sample_count_c = scm_to_uint32(sample_count);
  scm_c_local_define_malloc_and_size(channel_data, f32_s *,
                                     (channel_count_c * sizeof(pointer)));
  local_memory_add(channel_data);
  b32 index = channel_count_c;
  f32_s *data;
  while (index) {
    decrement_one(index);
    data = malloc((sample_count_c * sizeof(f32_s)));
    if (!data) {
      scm_c_local_error("memory", 0);
    };
    (*(channel_data + index)) = data;
  };
  snd_pcm_sframes_t frames_read;
  frames_read = snd_pcm_readn(((snd_pcm_t *)((*port_data).data)),
                              ((b0 **)(channel_data)), sample_count_c);
  if (((frames_read < 0) && (snd_pcm_recover(((snd_pcm_t *)((*port_data).data)),
                                             frames_read, 0) < 0))) {
    scm_c_local_error("alsa",
                      scm_from_locale_string(snd_strerror(frames_read)));
  };
  index = channel_count_c;
  SCM r = SCM_EOL;
  while (index) {
    decrement_one(index);
    r = scm_cons(scm_take_f32vector((*(channel_data + index)), sample_count_c),
                 r);
  };
  local_memory_free;
  return (r);
error:
  local_memory_free;
  scm_c_local_error_return;
};
SCM scm_sp_io_file_write(SCM port, SCM sample_count, SCM channel_data) {
  scm_c_local_error_init;
  local_memory_init(2);
  scm_c_local_error_assert(
      "type-check",
      (scm_c_sp_port_p(port) && scm_is_true(scm_list_p(channel_data)) &&
       !scm_is_null(channel_data) &&
       scm_is_true(scm_f32vector_p(scm_first(channel_data))) &&
       scm_is_integer(sample_count)));
  port_data_t *port_data = sp_port_to_port_data(port);
  scm_c_local_error_assert("not-an-output-port", !(*port_data).input_p);
  b32 channel_count = (*port_data).channel_count;
  scm_c_local_error_assert(
      "channel-data-length-mismatch",
      (scm_to_uint32(scm_length(channel_data)) == channel_count));
  b64 sample_count_c = scm_to_uint64(sample_count);
  scm_c_local_define_malloc_and_size(channel_data_c, f32_s *,
                                     (channel_count * sizeof(pointer)));
  local_memory_add(channel_data_c);
  b32 index = 0;
  SCM e;
  scm_c_list_each(channel_data, e, {
    (*(channel_data_c + index)) = ((f32_s *)(SCM_BYTEVECTOR_CONTENTS(e)));
    increment_one(index);
  });
  scm_c_local_define_malloc_and_size(data_interleaved, f32_s,
                                     (sample_count_c * channel_count * 4));
  local_memory_add(data_interleaved);
  sp_interleave_n_and_swap_endian(data_interleaved, channel_data_c,
                                  channel_count, sample_count_c);
  scm_c_require_success_system(write(((int)((*port_data).data)),
                                     data_interleaved,
                                     (channel_count * sample_count_c * 4)));
  local_memory_free;
  return (SCM_BOOL_T);
error:
  local_memory_free;
  scm_c_local_error_return;
};
SCM scm_sp_io_file_read(SCM port, SCM sample_count) {
  scm_c_local_error_init;
  local_memory_init(2);
  scm_c_local_error_assert(
      "type-check", (scm_c_sp_port_p(port) && scm_is_integer(sample_count)));
  port_data_t *port_data = sp_port_to_port_data(port);
  b32 channel_count = (*port_data).channel_count;
  b64 sample_count_c = scm_to_uint64(sample_count);
  scm_c_local_define_malloc_and_size(data_interleaved, f32_s,
                                     (sample_count_c * channel_count * 4));
  local_memory_add(data_interleaved);
  scm_c_require_success_system(read(((int)((*port_data).data)),
                                    data_interleaved,
                                    (sample_count_c * channel_count * 4)));
  scm_c_local_define_malloc_and_size(data_non_interleaved, f32_s *,
                                     (channel_count * sizeof(pointer)));
  local_memory_add(data_non_interleaved);
  b32 index = channel_count;
  while (index) {
    decrement_one(index);
    (*(data_non_interleaved + index)) = malloc((sample_count_c * 4));
    if (!(*(data_non_interleaved + index))) {
      scm_c_local_error("memory", 0);
    };
  };
  sp_deinterleave_n_and_swap_endian(data_non_interleaved, data_interleaved,
                                    channel_count, sample_count_c);
  SCM r = SCM_EOL;
  index = channel_count;
  while (index) {
    decrement_one(index);
    r = scm_cons(
        scm_take_f32vector((*(data_non_interleaved + index)), sample_count_c),
        r);
  };
  local_memory_free;
  return (r);
error:
  local_memory_free;
  scm_c_local_error_return;
};
SCM scm_sp_port_input_p(SCM port) {
  return (scm_from_bool((*sp_port_to_port_data(port)).input_p));
};
SCM scm_sp_port_position(SCM port) {
  return (
      ((*sp_port_to_port_data(port)).position_p
           ? scm_from_uint64(((*sp_port_to_port_data(port)).position * 0.25))
           : SCM_BOOL_F));
};
SCM scm_sp_port_position_p(SCM port) {
  return (scm_from_bool((*sp_port_to_port_data(port)).position));
};
SCM scm_sp_port_channel_count(SCM port) {
  return (scm_from_uint32((*sp_port_to_port_data(port)).channel_count));
};
SCM scm_sp_port_samples_per_second(SCM port) {
  return (scm_from_uint32((*sp_port_to_port_data(port)).samples_per_second));
};
SCM scm_sp_port_p(SCM port) { return (scm_from_bool(scm_c_sp_port_p(port))); };
SCM scm_sp_port_type(SCM port) {
  let_macro(type((*sp_port_to_port_data(port)).type),
            return (((type == sp_port_type_alsa)
                         ? scm_sp_port_type_alsa
                         : ((type == sp_port_type_file) ? scm_sp_port_type_file
                                                        : SCM_BOOL_F))));
};
SCM scm_sp_io_file_set_position(SCM port, SCM sample_position) {
  scm_c_local_error_init;
  scm_c_local_error_assert(
      "type-check", (scm_c_sp_port_p(port) && scm_is_integer(sample_position)));
  port_data_t *port_data = sp_port_to_port_data(port);
  b64_s position_c = (scm_to_int64(sample_position) * 4);
  let_macro(file(((int)((*port_data).data)), position_offset,
                 (*port_data).position_offset),
            if ((position_c >= 0)) {
              scm_c_require_success_system(
                  lseek(file, (position_offset + position_c), SEEK_SET));
            } else {
              off_t end_position = lseek(file, 0, SEEK_END);
              scm_c_require_success_system(end_position);
              position_c = (end_position + position_c);
              if ((position_c >= position_offset)) {
                scm_c_require_success_system(lseek(file, position_c, SEEK_SET));
              } else {
                scm_c_local_error("invalid position", 0);
              };
            });
  (*port_data).position = position_c;
  return (SCM_BOOL_T);
error:
  scm_c_local_error_return;
};
#endif
#include <foreign/kissfft/kiss_fft.h>
#include <foreign/kissfft/tools/kiss_fftr.h>
SCM scm_sp_fft(SCM a) {
  scm_c_local_error_init;
  scm_c_local_error_assert("type-check", scm_is_true(scm_f32vector_p(a)));
  b32 size = (SCM_BYTEVECTOR_LENGTH(a) / 4);
  b32 size_result = (1 + (size * 0.5));
  kiss_fftr_cfg fftr_state = kiss_fftr_alloc(size, 0, 0, 0);
  kiss_fft_cpx *out = malloc((size_result * sizeof(kiss_fft_cpx)));
  kiss_fftr(fftr_state, ((f32_s *)(SCM_BYTEVECTOR_CONTENTS(a))), out);
  SCM r = scm_make_f32vector(scm_from_uint32(size_result), scm_from_uint8(0));
  while (size_result) {
    decrement_one(size_result);
    (*(((f32_s *)(SCM_BYTEVECTOR_CONTENTS(r))) + size_result)) =
        out[size_result].r;
  };
  free(fftr_state);
  free(out);
  return (r);
error:
  scm_c_local_error_return;
};
SCM scm_sp_fft_inverse(SCM a) {
  scm_c_local_error_init;
  scm_c_local_error_assert("type-check", scm_is_true(scm_f32vector_p(a)));
  b32 size = (SCM_BYTEVECTOR_LENGTH(a) / 4);
  b32 size_result = ((size - 1) * 2);
  kiss_fftr_cfg fftr_state = kiss_fftr_alloc(size_result, 1, 0, 0);
  kiss_fft_cpx *inp = malloc((size * sizeof(kiss_fft_cpx)));
  while (size) {
    decrement_one(size);
    inp[size].r = (*(SCM_BYTEVECTOR_CONTENTS(a) + (size * 4)));
  };
  SCM r = scm_make_f32vector(scm_from_uint32(size_result), scm_from_uint8(0));
  kiss_fftri(fftr_state, inp, ((f32_s *)(SCM_BYTEVECTOR_CONTENTS(r))));
  free(fftr_state);
  return (r);
error:
  scm_c_local_error_return;
};
b0 init_sp() {
  init_scm();
  sp_port_scm_type_init;
  SCM t;
  SCM scm_module = scm_c_resolve_module("sph sp");
  scm_sp_port_type_alsa = scm_from_uint8(sp_port_type_alsa);
  scm_sp_port_type_file = scm_from_uint8(sp_port_type_file);
  scm_c_module_define(scm_module, "sp-port-type-alsa", scm_sp_port_type_alsa);
  scm_c_module_define(scm_module, "sp-port-type-file", scm_sp_port_type_file);
  scm_c_define_procedure_c(t, "sp-port-close", 1, 0, 0, scm_sp_port_close,
                           "sp-port -> boolean/error");
  scm_c_define_procedure_c(t, "sp-port-input?", 1, 0, 0, scm_sp_port_input_p,
                           "sp-port -> boolean/error");
  scm_c_define_procedure_c(t, "sp-port-position?", 1, 0, 0,
                           scm_sp_port_position_p, "sp-port -> boolean/error");
  scm_c_define_procedure_c(t, "sp-port-position", 1, 0, 0, scm_sp_port_position,
                           "sp-port -> integer/boolean/error");
  scm_c_define_procedure_c(t, "sp-port-channel-count", 1, 0, 0,
                           scm_sp_port_channel_count,
                           "sp-port -> integer/error");
  scm_c_define_procedure_c(t, "sp-port-samples-per-second", 1, 0, 0,
                           scm_sp_port_samples_per_second,
                           "sp-port -> integer/boolean/error");
  scm_c_define_procedure_c(t, "sp-port?", 1, 0, 0, scm_sp_port_p,
                           "sp-port -> boolean");
  scm_c_define_procedure_c(t, "sp-port-type", 1, 0, 0, scm_sp_port_type,
                           "sp-port -> integer");
  scm_c_define_procedure_c(t, "sp-fft", 1, 0, 0, scm_sp_fft,
                           "f32vector:volumes-per-time -> "
                           "f32vector:frequencies-per-time\n    discrete "
                           "fourier transform on the input data");
  scm_c_define_procedure_c(t, "sp-fft-inverse", 1, 0, 0, scm_sp_fft_inverse,
                           "f32vector:frequencies-per-time -> "
                           "f32vector:volume-per-time\n    inverse discrete "
                           "fourier transform on the input data");
  scm_c_define_procedure_c(
      t, "sp-io-file-open-input", 1, 2, 0, scm_sp_io_file_open_input,
      "string -> sp-port/error\n    path -> sp-port/error");
  scm_c_define_procedure_c(
      t, "sp-io-file-open-output", 1, 2, 0, scm_sp_io_file_open_output,
      "string [integer integer] -> sp-port/error\n    path [channel-count "
      "samples-per-second] -> sp-port/error");
  scm_c_define_procedure_c(t, "sp-io-file-write", 2, 1, 0, scm_sp_io_file_write,
                           "sp-port (f32vector ...):channel-data "
                           "[integer:sample-count] -> boolean/error\n    write "
                           "sample data to the channels of a file port");
  scm_c_define_procedure_c(
      t, "sp-io-file-set-position", 2, 0, 0, scm_sp_io_file_set_position,
      "sp-port integer:sample-offset -> boolean/error\n    sample-offset can "
      "be negative, in which case it is from the end of the file");
  scm_c_define_procedure_c(
      t, "sp-io-file-read", 2, 0, 0, scm_sp_io_file_read,
      "sp-port integer:sample-count -> (f32vector ...):channel-data/error");
  scm_c_define_procedure_c(
      t, "sp-io-alsa-open-input", 0, 4, 0, scm_sp_io_alsa_open_input,
      "[string integer integer integer] -> sp-port/error\n    [device-name "
      "channel-count samples-per-second latency] -> sp-port/error");
  scm_c_define_procedure_c(
      t, "sp-io-alsa-open-output", 0, 4, 0, scm_sp_io_alsa_open_output,
      "[string integer integer integer] -> sp-port/error\n    [device-name "
      "channel-count samples-per-second latency] -> sp-port/error");
  scm_c_define_procedure_c(t, "sp-io-alsa-write", 2, 1, 0, scm_sp_io_alsa_write,
                           "sp-port (f32vector ...):channel-data "
                           "[integer:sample-count] -> boolean/error\n    write "
                           "sample data to the channels of an alsa port - to "
                           "the sound card for sound output for example");
  scm_c_define_procedure_c(t, "sp-io-alsa-read", 2, 0, 0, scm_sp_io_alsa_read,
                           "port sample-count -> channel-data/error\n    "
                           "sp-port integer -> (f32vector ...)/error");
};