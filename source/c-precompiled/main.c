
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
#ifndef sc_included_stdio_h
#include <stdio.h>
#define sc_included_stdio_h
#endif
#ifndef sc_included_libguile_h
#include <libguile.h>
#define sc_included_libguile_h
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
#ifndef sc_included_asoundlib_h
#include <alsa/asoundlib.h>
#define sc_included_asoundlib_h
#endif
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
#ifndef sc_included_sph_one
#define sc_included_sph_one
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
/** defines and registers a c routine as a scheme procedure with documentation.
  like scm-c-define-gsubr but also sets documentation.
  scm-c-define-procedure-c-init must have been called in scope */
#define scm_c_define_procedure_c(name, required, optional, rest, c_function,   \
                                 documentation)                                \
  scm_c_define_procedure_c_temp =                                              \
      scm_c_define_gsubr(name, required, optional, rest, c_function);          \
  scm_set_procedure_property_x(scm_c_define_procedure_c_temp,                  \
                               scm_from_locale_symbol("documentation"),        \
                               scm_from_locale_string(documentation))
;
/** display value with scheme write and add a newline */
b0 scm_debug_log(SCM value) {
  scm_call_2(scm_variable_ref(scm_c_lookup("write")), value,
             scm_current_output_port());
  scm_newline(scm_current_output_port());
};
/** creates a new bytevector of size-octects that contains the given bytevector
 */
SCM scm_c_bytevector_take(size_t size_octets, b8 *a) {
  SCM r = scm_c_make_bytevector(size_octets);
  memcpy(SCM_BYTEVECTOR_CONTENTS(r), a, size_octets);
  return (r);
};
/** SCM SCM c-compound-expression ->
  iterate over scm-list in c */
#define scm_c_list_each(list, e, body)                                         \
  while (!scm_is_null(list)) {                                                 \
    e = scm_first(list);                                                       \
    body;                                                                      \
    list = scm_tail(list);                                                     \
  }
;
#endif
#ifndef sc_included_sph_status
#define sc_included_sph_status
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
#endif
#ifndef sc_included_sph_local_memory
#define sc_included_sph_local_memory
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
#endif
#ifndef sc_included_sp_config
#define sc_included_sp_config
#define sp_sample_t f32_s
#define sp_default_samples_per_second 16000
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
#endif
#ifndef sc_included_sp_status
#define sc_included_sp_status
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
SCM scm_rnrs_raise;
#define scm_c_error(name, description)                                         \
  scm_call_1(scm_rnrs_raise,                                                   \
             scm_list_3(scm_from_latin1_symbol(name),                          \
                        scm_cons(scm_from_latin1_symbol("description"),        \
                                 scm_from_utf8_string(description)),           \
                        scm_cons(scm_from_latin1_symbol("c-routine"),          \
                                 scm_from_latin1_symbol(__FUNCTION__))))
#define status_to_scm_error(a)                                                 \
  scm_c_error(sp_status_name(a), sp_status_description(a))
#define status_to_scm(result)                                                  \
  (status_success_p ? result : status_to_scm_error(status))
/** call scm-c-error if status is not success or return result */
#define status_to_scm_return(result) return (status_to_scm(result))
;
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
#endif
#ifndef sc_included_sp_io
#define sc_included_sp_io
#define sp_port_type_alsa 0
#define sp_port_type_file 1
#define sp_port_bit_input 1
#define sp_port_bit_position 2
/** -> boolean
  returns 1 if the header was successfully written, 0 otherwise.
  assumes file is positioned at offset 0 */
b8_s file_au_write_header(int file, b32 encoding, b32 samples_per_second,
                          b32 channel_count) {
  b32 header[7];
  (*(header + 0)) = __builtin_bswap32(779316836);
  (*(header + 1)) = __builtin_bswap32(28);
  (*(header + 2)) = __builtin_bswap32(4294967295);
  (*(header + 3)) = __builtin_bswap32(encoding);
  (*(header + 4)) = __builtin_bswap32(samples_per_second);
  (*(header + 5)) = __builtin_bswap32(channel_count);
  (*(header + 6)) = 0;
  ssize_t status = write(file, header, 28);
  return ((status == 28));
};
/** -> boolean
  when successful, the reader is positioned at the beginning of the sample data
*/
b8_s file_au_read_header(int file, b32 *encoding, b32 *samples_per_second,
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
  (*samples_per_second) = __builtin_bswap32((*(header + 4)));
  (*channel_count) = __builtin_bswap32((*(header + 5)));
  return (1);
};
#define optional_samples_per_second(a)                                         \
  ((SCM_UNDEFINED == a) ? sp_default_samples_per_second : scm_to_uint32(a))
#define optional_channel_count(a)                                              \
  (scm_is_undefined(a) ? sp_default_channel_count : scm_to_uint32(a))
#define calc_interleaved_size(channel_count, sample_count)                     \
  (channel_count * sample_count * sizeof(sp_sample_t))
#define sp_alsa_status_require_x(expression)                                   \
  status_set_id(expression);                                                   \
  if (status_failure_p) {                                                      \
    status_set_group_goto(sp_status_group_alsa);                               \
  }
#define define_sp_interleave(name, get_source_element)                         \
  b0 name(sp_sample_t **deinterleaved, sp_sample_t *interleaved,               \
          b32 channel_count, b32 deinterleaved_size) {                         \
    b32 interleaved_size = (deinterleaved_size * channel_count);               \
    b32 current_channel;                                                       \
    while (deinterleaved_size) {                                               \
      decrement(deinterleaved_size);                                           \
      current_channel = channel_count;                                         \
      while (current_channel) {                                                \
        decrement(current_channel);                                            \
        decrement(interleaved_size);                                           \
        (*(interleaved + interleaved_size)) = get_source_element;              \
      };                                                                       \
    };                                                                         \
  }
#define define_sp_deinterleave(name, get_source_element)                       \
  b0 name(sp_sample_t *interleaved, sp_sample_t **deinterleaved,               \
          b32 channel_count, b32 deinterleaved_size) {                         \
    b32 interleaved_size = (deinterleaved_size * channel_count);               \
    b32 current_channel;                                                       \
    while (deinterleaved_size) {                                               \
      decrement(deinterleaved_size);                                           \
      current_channel = channel_count;                                         \
      while (current_channel) {                                                \
        decrement(current_channel);                                            \
        decrement(interleaved_size);                                           \
        (*((*(deinterleaved + current_channel)) + deinterleaved_size)) =       \
            get_source_element;                                                \
      };                                                                       \
    };                                                                         \
  }
define_sp_deinterleave(sp_deinterleave, (*(interleaved + interleaved_size)));
define_sp_interleave(sp_interleave, (*((*(deinterleaved + current_channel)) +
                                       deinterleaved_size)));
define_sp_deinterleave(sp_deinterleave_and_reverse_endian,
                       sample_reverse_endian((*(interleaved +
                                                interleaved_size))));
define_sp_interleave(
    sp_interleave_and_reverse_endian,
    sample_reverse_endian((*((*(deinterleaved + current_channel)) +
                             deinterleaved_size))));
typedef struct {
  b32 samples_per_second;
  b32 channel_count;
  b8 flags;
  b8 type;
  b8 closed_p;
  b64 position;
  b16 position_offset;
  b0 *data;
  int data_int;
} port_data_t;
scm_t_bits sp_port_scm_type;
SCM scm_sp_port_type_alsa;
SCM scm_sp_port_type_file;
#define scm_c_sp_port_p(a) SCM_SMOB_PREDICATE(sp_port_scm_type, a)
#define scm_to_port_data(a) ((port_data_t *)(SCM_SMOB_DATA(a)))
/** integer -> string */
#define sp_port_type_to_name(a)                                                \
  ((sp_port_type_file == a) ? "file"                                           \
                            : ((sp_port_type_alsa == a) ? "alsa" : "unknown"))
;
int sp_port_print(SCM a, SCM output_port, scm_print_state *print_state) {
  port_data_t *port_data = scm_to_port_data(a);
  char *result = malloc((70 + 10 + 7 + 10 + 10 + 2 + 2));
  if (!result) {
    return (0);
  };
  sprintf(result,
          "#<sp-port %lx type:%s samples-per-second:%d channel-count:%d "
          "closed?:%s input?:%s>",
          ((b0 *)(a)), sp_port_type_to_name((*port_data).type),
          (*port_data).samples_per_second, (*port_data).channel_count,
          ((*port_data).closed_p ? "#t" : "#f"),
          ((sp_port_bit_input & (*port_data).flags) ? "#t" : "#f"));
  scm_dynwind_free(result);
  scm_display(scm_take_locale_string(result), output_port);
  return (0);
};
/** integer integer integer integer integer pointer integer -> sp-port
   flags is a combination of sp-port-bits.
  memory is allocated with scm-gc-malloc */
SCM sp_port_create(b8 type, b8 flags, b32 samples_per_second, b32 channel_count,
                   b16 position_offset, b0 *data, int data_int) {
  port_data_t *port_data = scm_gc_malloc(sizeof(port_data_t), "sp-port-data");
  (*port_data).channel_count = channel_count;
  (*port_data).samples_per_second = samples_per_second;
  (*port_data).type = type;
  (*port_data).flags = flags;
  (*port_data).data = data;
  (*port_data).data_int = data_int;
  (*port_data).position = 0;
  (*port_data).position_offset = position_offset;
  return (scm_new_smob(sp_port_scm_type, ((scm_t_bits)(port_data))));
};
#define sp_port_scm_type_init                                                  \
  sp_port_scm_type = scm_make_smob_type("sp-port", 0);                         \
  scm_set_smob_print(sp_port_scm_type, sp_port_print)
SCM sp_io_alsa_open(b8 input_p, SCM device_name, SCM channel_count,
                    SCM samples_per_second, SCM latency) {
  status_init;
  local_memory_init(1);
  snd_pcm_t *alsa_port = 0;
  char *device_name_c;
  if (scm_is_undefined(device_name)) {
    device_name_c = "default";
  } else {
    device_name_c = scm_to_locale_string(device_name);
    local_memory_add(device_name_c);
  };
  sp_alsa_status_require_x(snd_pcm_open(
      &alsa_port, device_name_c,
      (input_p ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK), 0));
  b32 latency_c = (scm_is_undefined(latency) ? sp_default_alsa_latency
                                             : scm_to_uint32(latency));
  b32 channel_count_c = optional_channel_count(channel_count);
  b32 samples_per_second_c = optional_samples_per_second(samples_per_second);
  sp_alsa_status_require_x(snd_pcm_set_params(
      alsa_port, SND_PCM_FORMAT_FLOAT_LE, SND_PCM_ACCESS_RW_NONINTERLEAVED,
      channel_count_c, samples_per_second_c,
      sp_default_alsa_enable_soft_resample, latency_c));
  SCM result = sp_port_create(
      sp_port_type_alsa, (input_p ? sp_port_bit_input : 0),
      samples_per_second_c, channel_count_c, 0, ((b0 *)(alsa_port)), 0);
exit:
  local_memory_free;
  if (status_failure_p) {
    if (alsa_port) {
      snd_pcm_close(alsa_port);
    };
  };
  status_to_scm_return(result);
};
SCM sp_io_file_open(SCM path, b8 input_p, SCM channel_count,
                    SCM samples_per_second) {
  int file;
  SCM result;
  b32 samples_per_second_file;
  b32 channel_count_file;
  char *path_c = scm_to_locale_string(path);
  sp_status_init;
  local_memory_init(1);
  local_memory_add(path_c);
  b8 flags = (input_p ? (sp_port_bit_input | sp_port_bit_position)
                      : sp_port_bit_position);
  if (file_exists_p(path_c)) {
    file = open(path_c, O_RDWR);
    sp_system_status_require_id(file);
    b32 encoding;
    if (!file_au_read_header(file, &encoding, &samples_per_second_file,
                             &channel_count_file)) {
      status_set_id_goto(sp_status_id_file_header);
    };
    if (!(encoding == 6)) {
      status_set_id_goto(sp_status_id_file_encoding);
    };
    if (!(scm_is_undefined(channel_count) ||
          ((channel_count_file == scm_to_uint32(channel_count))))) {
      status_set_id_goto(sp_status_id_file_incompatible);
    };
    if (!(scm_is_undefined(samples_per_second) ||
          ((samples_per_second_file == scm_to_uint32(samples_per_second))))) {
      status_set_id_goto(sp_status_id_file_incompatible);
    };
    off_t offset = lseek(file, 0, SEEK_CUR);
    sp_system_status_require_id(offset);
    result = sp_port_create(sp_port_type_file, flags, samples_per_second_file,
                            channel_count_file, offset, 0, file);
  } else {
    file = open(path_c, (O_RDWR | O_CREAT), 384);
    sp_system_status_require_id(file);
    samples_per_second_file = optional_samples_per_second(samples_per_second);
    channel_count_file = optional_channel_count(channel_count);
    if (!file_au_write_header(file, 6, samples_per_second_file,
                              channel_count_file)) {
      status_set_id_goto(sp_status_id_file_header);
    };
    off_t offset = lseek(file, 0, SEEK_CUR);
    sp_system_status_require_id(offset);
    result = sp_port_create(sp_port_type_file, flags, samples_per_second_file,
                            channel_count_file, offset, 0, file);
  };
exit:
  local_memory_free;
  status_to_scm_return(result);
};
SCM scm_sp_port_close(SCM a) {
  status_init;
  port_data_t *port_data = scm_to_port_data(a);
  if ((*port_data).closed_p) {
    goto exit;
  };
  if ((sp_port_type_alsa == (*port_data).type)) {
    sp_alsa_status_require_x(snd_pcm_close(((snd_pcm_t *)((*port_data).data))));
  } else {
    if ((sp_port_type_file == (*port_data).type)) {
      sp_system_status_require_x(close((*port_data).data_int));
    };
  };
  (*port_data).closed_p = 1;
exit:
  status_to_scm_return(SCM_BOOL_T);
};
/** returns the current port position in number of octets */
SCM scm_sp_port_position(SCM port) {
  return (scm_from_uint64((*scm_to_port_data(port)).position));
};
SCM scm_sp_port_position_p(SCM port) {
  return (
      scm_from_bool((sp_port_bit_position & (*scm_to_port_data(port)).flags)));
};
SCM scm_sp_port_input_p(SCM port) {
  return (scm_from_bool((sp_port_bit_input & (*scm_to_port_data(port)).flags)));
};
SCM scm_sp_port_channel_count(SCM port) {
  return (scm_from_uint32((*scm_to_port_data(port)).channel_count));
};
SCM scm_sp_port_samples_per_second(SCM port) {
  return (scm_from_uint32((*scm_to_port_data(port)).samples_per_second));
};
SCM scm_sp_port_p(SCM port) { return (scm_from_bool(scm_c_sp_port_p(port))); };
SCM scm_sp_port_type(SCM port) {
  return (((sp_port_type_alsa == (*scm_to_port_data(port)).type)
               ? scm_sp_port_type_alsa
               : ((sp_port_type_file == (*scm_to_port_data(port)).type)
                      ? scm_sp_port_type_file
                      : SCM_BOOL_F)));
};
SCM scm_sp_io_file_open_input(SCM path) {
  return (sp_io_file_open(path, 1, SCM_UNDEFINED, SCM_UNDEFINED));
};
SCM scm_sp_io_file_open_output(SCM path, SCM channel_count,
                               SCM samples_per_second) {
  return (sp_io_file_open(path, 0, channel_count, samples_per_second));
};
SCM scm_sp_io_alsa_write(SCM port, SCM sample_count, SCM channel_data) {
  status_init;
  port_data_t *port_data = scm_to_port_data(port);
  b32 channel_count = (*port_data).channel_count;
  b32 sample_count_c = scm_to_uint32(sample_count);
  local_memory_init(1);
  sp_define_malloc(channel_data_c, sp_sample_t **,
                   (channel_count * sizeof(sp_sample_t *)));
  local_memory_add(channel_data_c);
  b32 channel = 0;
  SCM a;
  scm_c_list_each(channel_data, a, {
    (*(channel_data_c + channel)) =
        ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(a)));
    increment(channel);
  });
  snd_pcm_sframes_t frames_written = snd_pcm_writen(
      ((snd_pcm_t *)((*port_data).data)), ((b0 **)(channel_data_c)),
      ((snd_pcm_uframes_t)(sample_count_c)));
  if (((frames_written < 0) &&
       (snd_pcm_recover(((snd_pcm_t *)((*port_data).data)), frames_written, 0) <
        0))) {
    status_set_both_goto(sp_status_group_alsa, frames_written);
  };
exit:
  local_memory_free;
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_io_alsa_read(SCM port, SCM sample_count) {
  status_init;
  port_data_t *port_data = scm_to_port_data(port);
  b32 channel_count = (*port_data).channel_count;
  b32 sample_count_c = scm_to_uint32(sample_count);
  local_memory_init((1 + channel_count));
  sp_define_malloc(channel_data, sp_sample_t **,
                   (channel_count * sizeof(sp_sample_t *)));
  local_memory_add(channel_data);
  b32 channel = channel_count;
  while (channel) {
    decrement(channel);
    sp_define_malloc(data, sp_sample_t *,
                     (sample_count_c * sizeof(sp_sample_t)));
    local_memory_add(data);
    (*(channel_data + channel)) = data;
  };
  snd_pcm_sframes_t frames_read =
      snd_pcm_readn(((snd_pcm_t *)((*port_data).data)), ((b0 **)(channel_data)),
                    sample_count_c);
  if (((frames_read < 0) && (snd_pcm_recover(((snd_pcm_t *)((*port_data).data)),
                                             frames_read, 0) < 0))) {
    status_set_both_goto(sp_status_group_alsa, frames_read);
  };
  SCM result = SCM_EOL;
  channel = channel_count;
  while (channel) {
    decrement(channel);
    result = scm_cons(
        scm_take_f32vector((*(channel_data + channel)), sample_count_c),
        result);
  };
exit:
  local_memory_free;
  status_to_scm_return(result);
};
SCM scm_sp_io_file_write(SCM port, SCM sample_count, SCM channel_data) {
  status_init;
  port_data_t *port_data = scm_to_port_data(port);
  if ((sp_port_bit_input & (*port_data).flags)) {
    status_set_both_goto(sp_status_group_sp, sp_status_id_port_type);
  };
  b32 channel_count = (*port_data).channel_count;
  if (!(scm_to_uint32(scm_length(channel_data)) == channel_count)) {
    status_set_both_goto(sp_status_group_sp,
                         sp_status_id_file_channel_mismatch);
  };
  local_memory_init(2);
  b32 sample_count_c = scm_to_uint32(sample_count);
  sp_define_malloc(channel_data_c, sp_sample_t **,
                   (channel_count * sizeof(sp_sample_t *)));
  local_memory_add(channel_data_c);
  b32 channel = 0;
  SCM a;
  scm_c_list_each(channel_data, a, {
    (*(channel_data_c + channel)) =
        ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(a)));
    increment(channel);
  });
  size_t interleaved_size =
      calc_interleaved_size(channel_count, sample_count_c);
  sp_define_malloc(data_interleaved, sp_sample_t *, interleaved_size);
  local_memory_add(data_interleaved);
  sp_interleave_and_reverse_endian(channel_data_c, data_interleaved,
                                   channel_count, sample_count_c);
  int status_2 =
      write((*port_data).data_int, data_interleaved, interleaved_size);
  if (!(interleaved_size == status_2)) {
    if ((status_2 < 0)) {
      status_set_both_goto(sp_status_group_libc, status_2);
    } else {
      status_set_both_goto(sp_status_group_sp, sp_status_id_file_incomplete);
    };
  };
exit:
  local_memory_free;
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_io_file_read(SCM port, SCM sample_count) {
  status_init;
  port_data_t *port_data = scm_to_port_data(port);
  b32 channel_count = (*port_data).channel_count;
  b32 sample_count_c = scm_to_uint32(sample_count);
  local_memory_init((2 + channel_count));
  size_t interleaved_size =
      calc_interleaved_size(channel_count, sample_count_c);
  sp_define_malloc(data_interleaved, sp_sample_t *, interleaved_size);
  local_memory_add(data_interleaved);
  int status_2 =
      read((*port_data).data_int, data_interleaved, interleaved_size);
  if (!(interleaved_size == status_2)) {
    if ((status_2 < 0)) {
      status_set_both_goto(sp_status_group_libc, status_2);
    } else {
      status_set_both_goto(sp_status_group_sp, sp_status_id_file_incomplete);
    };
  };
  sp_define_malloc(data_deinterleaved, sp_sample_t **,
                   (channel_count * sizeof(sp_sample_t *)));
  local_memory_add(data_deinterleaved);
  b32 channel = channel_count;
  while (channel) {
    decrement(channel);
    sp_define_malloc(data, sp_sample_t *,
                     (sample_count_c * sizeof(sp_sample_t)));
    local_memory_add(data);
    (*(data_deinterleaved + channel)) = data;
  };
  sp_deinterleave_and_reverse_endian(data_interleaved, data_deinterleaved,
                                     channel_count, sample_count_c);
  SCM result = SCM_EOL;
  channel = channel_count;
  while (channel) {
    decrement(channel);
    result = scm_cons(
        scm_take_f32vector((*(data_deinterleaved + channel)), sample_count_c),
        result);
  };
exit:
  local_memory_free;
  status_to_scm_return(result);
};
/** sp-port integer -> unspecified
   set port to offset in sample data */
SCM scm_sp_io_file_set_position(SCM port, SCM scm_sample_index) {
  status_init;
  port_data_t *port_data = scm_to_port_data(port);
  b32 sample_index = scm_to_int64(scm_sample_index);
  b64_s index = (sizeof(sp_sample_t) * sample_index);
#define file (*port_data).data_int
#define header_size (*port_data).position_offset
  if ((index >= 0)) {
    sp_system_status_require_x(lseek(file, (header_size + index), SEEK_SET));
  } else {
    off_t end_position = lseek(file, 0, SEEK_END);
    sp_system_status_require_id(end_position);
    index = (end_position + index);
    if ((index >= header_size)) {
      sp_system_status_require_x(lseek(file, index, SEEK_SET));
    } else {
      status_set_both_goto(sp_status_group_sp, sp_status_id_port_position);
    };
  };
#undef file
#undef header_size
  (*port_data).position = sample_index;
exit:
  status_to_scm_return(SCM_BOOL_T);
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
#endif
SCM scm_sp_fft(SCM a) {
  status_init;
  b32 size = (SCM_BYTEVECTOR_LENGTH(a) / 4);
  b32 size_result = (1 + (size * 0.5));
  local_memory_init(2);
  kiss_fftr_cfg fftr_state = kiss_fftr_alloc(size, 0, 0, 0);
  local_memory_add(fftr_state);
  sp_define_malloc(out, kiss_fft_cpx *, (size_result * sizeof(kiss_fft_cpx)));
  local_memory_add(out);
  kiss_fftr(fftr_state, ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(a))), out);
  SCM result =
      scm_make_f32vector(scm_from_uint32(size_result), scm_from_uint8(0));
  while (size_result) {
    decrement(size_result);
    (*(((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(result))) + size_result)) =
        (*(out + size_result)).r;
  };
exit:
  local_memory_free;
  status_to_scm_return(result);
};
SCM scm_sp_fft_inverse(SCM a) {
  status_init;
  b32 size = (SCM_BYTEVECTOR_LENGTH(a) / 4);
  b32 size_result = ((size - 1) * 2);
  local_memory_init(2);
  kiss_fftr_cfg fftr_state = kiss_fftr_alloc(size_result, 1, 0, 0);
  local_memory_add(fftr_state);
  sp_define_malloc(in, kiss_fft_cpx *, (size * sizeof(kiss_fft_cpx)));
  local_memory_add(in);
  while (size) {
    decrement(size);
    (*(in + size)).r = (*(SCM_BYTEVECTOR_CONTENTS(a) + (size * 4)));
  };
  SCM result =
      scm_make_f32vector(scm_from_uint32(size_result), scm_from_uint8(0));
  kiss_fftri(fftr_state, in,
             ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(result))));
exit:
  local_memory_free;
  status_to_scm_return(result);
};
b0 init_sp() {
  sp_port_scm_type_init;
  SCM scm_module = scm_c_resolve_module("sph sp");
  scm_sp_port_type_alsa = scm_from_latin1_symbol("alsa");
  scm_sp_port_type_file = scm_from_latin1_symbol("file");
  scm_rnrs_raise = scm_c_public_ref("rnrs exceptions", "raise");
  scm_c_module_define(scm_module, "sp-port-type-alsa", scm_sp_port_type_alsa);
  scm_c_module_define(scm_module, "sp-port-type-file", scm_sp_port_type_file);
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
  scm_c_define_procedure_c("sp-port-samples-per-second", 1, 0, 0,
                           scm_sp_port_samples_per_second,
                           "sp-port -> integer/boolean");
  scm_c_define_procedure_c("sp-port?", 1, 0, 0, scm_sp_port_p,
                           "sp-port -> boolean");
  scm_c_define_procedure_c("sp-port-type", 1, 0, 0, scm_sp_port_type,
                           "sp-port -> integer");
  scm_c_define_procedure_c("sp-fft", 1, 0, 0, scm_sp_fft,
                           "f32vector:value-per-time -> "
                           "f32vector:frequencies-per-time\n    discrete "
                           "fourier transform on the input data");
  scm_c_define_procedure_c("sp-fft-inverse", 1, 0, 0, scm_sp_fft_inverse,
                           "f32vector:frequencies-per-time -> "
                           "f32vector:value-per-time\n    inverse discrete "
                           "fourier transform on the input data");
  scm_c_define_procedure_c("sp-io-file-open-input", 1, 2, 0,
                           scm_sp_io_file_open_input,
                           "string -> sp-port\n    path -> sp-port");
  scm_c_define_procedure_c("sp-io-file-open-output", 1, 2, 0,
                           scm_sp_io_file_open_output,
                           "string [integer integer] -> sp-port\n    path "
                           "[channel-count samples-per-second] -> sp-port");
  scm_c_define_procedure_c("sp-io-file-write", 2, 1, 0, scm_sp_io_file_write,
                           "sp-port (f32vector ...):channel-data "
                           "[integer:sample-count] -> boolean\n    write "
                           "sample data to the channels of a file port");
  scm_c_define_procedure_c(
      "sp-io-file-set-position", 2, 0, 0, scm_sp_io_file_set_position,
      "sp-port integer:sample-offset -> boolean\n    sample-offset can be "
      "negative, in which case it is from the end of the file");
  scm_c_define_procedure_c(
      "sp-io-file-read", 2, 0, 0, scm_sp_io_file_read,
      "sp-port integer:sample-count -> (f32vector ...):channel-data");
  scm_c_define_procedure_c(
      "sp-io-alsa-open-input", 0, 4, 0, scm_sp_io_alsa_open_input,
      "[string integer integer integer] -> sp-port\n    [device-name "
      "channel-count samples-per-second latency] -> sp-port");
  scm_c_define_procedure_c(
      "sp-io-alsa-open-output", 0, 4, 0, scm_sp_io_alsa_open_output,
      "[string integer integer integer] -> sp-port\n    [device-name "
      "channel-count samples-per-second latency] -> sp-port");
  scm_c_define_procedure_c("sp-io-alsa-write", 2, 1, 0, scm_sp_io_alsa_write,
                           "sp-port (f32vector ...):channel-data "
                           "[integer:sample-count] -> boolean\n    write "
                           "sample data to the channels of an alsa port - to "
                           "the sound card for sound output for example");
  scm_c_define_procedure_c("sp-io-alsa-read", 2, 0, 0, scm_sp_io_alsa_read,
                           "port sample-count -> channel-data\n    sp-port "
                           "integer -> (f32vector ...)");
};