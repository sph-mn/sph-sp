
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
/** writes values with current routine name and line info to standard output.
  example: (debug-log "%d" 1)
  otherwise like printf */
#define debug_log(format, ...)                                                 \
  fprintf(stdout, "%s:%d " format "\n", __func__, __LINE__, __VA_ARGS__)
;
#define null ((b0)(0))
#define zero_p(a) (0 == a)
#endif
#ifndef sc_included_sp_config
#define sc_included_sp_config
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
#endif
#define kiss_fft_scalar sp_sample_t
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
#define inc(a) a = (1 + a)
#define dec(a) a = (a - 1)
#define octets_to_samples(a) (a / sizeof(sp_sample_t))
#define samples_to_octets(a) (a * sizeof(sp_sample_t))
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
/** writes values with current routine name and line info to standard output.
  example: (debug-log "%d" 1)
  otherwise like printf */
#define debug_log(format, ...)                                                 \
  fprintf(stdout, "%s:%d " format "\n", __func__, __LINE__, __VA_ARGS__)
;
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
#define optional_sample_rate(a)                                                \
  (scm_is_undefined(a) ? sp_default_sample_rate : scm_to_uint32(a))
#define optional_channel_count(a)                                              \
  (scm_is_undefined(a) ? sp_default_channel_count : scm_to_uint32(a))
#define sp_alsa_status_require_x(expression)                                   \
  status_set_id(expression);                                                   \
  if (status_failure_p) {                                                      \
    status_set_group_goto(sp_status_group_alsa);                               \
  }
/** a: deinterleaved
   b: interleaved */
#define define_sp_interleave(name, type, body)                                 \
  b0 name(type **a, type *b, size_t a_size, b32 channel_count) {               \
    size_t b_size = (a_size * channel_count);                                  \
    b32 channel;                                                               \
    while (a_size) {                                                           \
      decrement(a_size);                                                       \
      channel = channel_count;                                                 \
      while (channel) {                                                        \
        decrement(channel);                                                    \
        decrement(b_size);                                                     \
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
typedef struct {
  b32 sample_rate;
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
          "#<sp-port %lx type:%s sample-rate:%d channel-count:%d closed?:%s "
          "input?:%s>",
          ((b0 *)(a)), sp_port_type_to_name((*port_data).type),
          (*port_data).sample_rate, (*port_data).channel_count,
          ((*port_data).closed_p ? "#t" : "#f"),
          ((sp_port_bit_input & (*port_data).flags) ? "#t" : "#f"));
  scm_display(scm_take_locale_string(result), output_port);
  return (0);
};
/** integer integer integer integer integer pointer integer -> sp-port
   flags is a combination of sp-port-bits.
  memory is allocated with scm-gc-malloc */
SCM sp_port_create(b8 type, b8 flags, b32 sample_rate, b32 channel_count,
                   b16 position_offset, b0 *data, int data_int) {
  port_data_t *port_data = scm_gc_malloc(sizeof(port_data_t), "sp-port-data");
  (*port_data).channel_count = channel_count;
  (*port_data).sample_rate = sample_rate;
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
SCM sp_alsa_open(b8 input_p, SCM scm_device_name, SCM scm_channel_count,
                 SCM scm_sample_rate, SCM scm_latency) {
  status_init;
  snd_pcm_t *alsa_port = 0;
  local_memory_init(1);
  char *device_name;
  if (scm_is_undefined(scm_device_name)) {
    device_name = "default";
  } else {
    device_name = scm_to_locale_string(scm_device_name);
    local_memory_add(device_name);
  };
  sp_alsa_status_require_x(snd_pcm_open(
      &alsa_port, device_name,
      (input_p ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK), 0));
  b32 latency = (scm_is_undefined(scm_latency) ? sp_default_alsa_latency
                                               : scm_to_uint32(scm_latency));
  b32 channel_count = optional_channel_count(scm_channel_count);
  b32 sample_rate = optional_sample_rate(scm_sample_rate);
  sp_alsa_status_require_x(snd_pcm_set_params(
      alsa_port, SND_PCM_FORMAT_FLOAT_LE, SND_PCM_ACCESS_RW_NONINTERLEAVED,
      channel_count, sample_rate, sp_default_alsa_enable_soft_resample,
      latency));
  SCM result =
      sp_port_create(sp_port_type_alsa, (input_p ? sp_port_bit_input : 0),
                     sample_rate, channel_count, 0, ((b0 *)(alsa_port)), 0);
exit:
  if ((status_failure_p && alsa_port)) {
    snd_pcm_close(alsa_port);
  };
  local_memory_free;
  status_to_scm_return(result);
};
SCM sp_file_open(SCM path, b8 input_p, SCM channel_count, SCM sample_rate) {
  int file;
  SCM result;
  b32 sample_rate_file;
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
    if (!file_au_read_header(file, &encoding, &sample_rate_file,
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
    if (!(scm_is_undefined(sample_rate) ||
          ((sample_rate_file == scm_to_uint32(sample_rate))))) {
      status_set_id_goto(sp_status_id_file_incompatible);
    };
    off_t offset = lseek(file, 0, SEEK_CUR);
    sp_system_status_require_id(offset);
    result = sp_port_create(sp_port_type_file, flags, sample_rate_file,
                            channel_count_file, offset, 0, file);
  } else {
    file = open(path_c, (O_RDWR | O_CREAT), 384);
    sp_system_status_require_id(file);
    sample_rate_file = optional_sample_rate(sample_rate);
    channel_count_file = optional_channel_count(channel_count);
    if (!file_au_write_header(file, 6, sample_rate_file, channel_count_file)) {
      status_set_id_goto(sp_status_id_file_header);
    };
    off_t offset = lseek(file, 0, SEEK_CUR);
    sp_system_status_require_id(offset);
    result = sp_port_create(sp_port_type_file, flags, sample_rate_file,
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
SCM scm_sp_port_sample_rate(SCM port) {
  return (scm_from_uint32((*scm_to_port_data(port)).sample_rate));
};
SCM scm_sp_port_p(SCM port) { return (scm_from_bool(scm_c_sp_port_p(port))); };
SCM scm_sp_port_type(SCM port) {
  return (((sp_port_type_alsa == (*scm_to_port_data(port)).type)
               ? scm_sp_port_type_alsa
               : ((sp_port_type_file == (*scm_to_port_data(port)).type)
                      ? scm_sp_port_type_file
                      : SCM_BOOL_F)));
};
SCM scm_sp_alsa_write(SCM port, SCM scm_sample_count, SCM scm_channel_data) {
  status_init;
  port_data_t *port_data = scm_to_port_data(port);
  b32 channel_count = (*port_data).channel_count;
  size_t deinterleaved_count = scm_to_size_t(scm_sample_count);
  local_memory_init(1);
  sp_define_malloc(deinterleaved, sp_sample_t **,
                   (channel_count * sizeof(sp_sample_t *)));
  local_memory_add(deinterleaved);
  b32 channel = 0;
  SCM a;
  scm_c_list_each(scm_channel_data, a, {
    (*(deinterleaved + channel)) =
        ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(a)));
    increment(channel);
  });
  snd_pcm_sframes_t frames_written = snd_pcm_writen(
      ((snd_pcm_t *)((*port_data).data)), ((b0 **)(deinterleaved)),
      ((snd_pcm_uframes_t)(deinterleaved_count)));
  if (((frames_written < 0) &&
       (snd_pcm_recover(((snd_pcm_t *)((*port_data).data)), frames_written, 0) <
        0))) {
    status_set_both_goto(sp_status_group_alsa, frames_written);
  };
exit:
  local_memory_free;
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_alsa_read(SCM port, SCM scm_sample_count) {
  status_init;
  port_data_t *port_data = scm_to_port_data(port);
  b32 channel_count = (*port_data).channel_count;
  size_t deinterleaved_count = scm_to_size_t(scm_sample_count);
  local_memory_init(1);
  sp_define_malloc(deinterleaved, sp_sample_t **,
                   (channel_count * sizeof(sp_sample_t *)));
  local_memory_add(deinterleaved);
  b32 channel = channel_count;
  size_t deinterleaved_size = (deinterleaved_count * sizeof(sp_sample_t));
  while (channel) {
    decrement(channel);
    sp_define_malloc(data, sp_sample_t *, deinterleaved_size);
    (*(deinterleaved + channel)) = data;
  };
  snd_pcm_sframes_t frames_read =
      snd_pcm_readn(((snd_pcm_t *)((*port_data).data)),
                    ((b0 **)(deinterleaved)), deinterleaved_count);
  if (((frames_read < 0) && (snd_pcm_recover(((snd_pcm_t *)((*port_data).data)),
                                             frames_read, 0) < 0))) {
    status_set_both_goto(sp_status_group_alsa, frames_read);
  };
  SCM result = SCM_EOL;
  channel = channel_count;
  while (channel) {
    decrement(channel);
    result = scm_cons(
        scm_take_f32vector((*(deinterleaved + channel)), deinterleaved_count),
        result);
  };
exit:
  local_memory_free;
  status_to_scm_return(result);
};
SCM scm_sp_file_write(SCM port, SCM scm_sample_count, SCM scm_channel_data) {
  status_init;
  port_data_t *port_data = scm_to_port_data(port);
  if ((sp_port_bit_input & (*port_data).flags)) {
    status_set_both_goto(sp_status_group_sp, sp_status_id_port_type);
  };
  b32 channel_count = (*port_data).channel_count;
  if (!(scm_to_uint32(scm_length(scm_channel_data)) == channel_count)) {
    status_set_both_goto(sp_status_group_sp,
                         sp_status_id_file_channel_mismatch);
  };
  local_memory_init(2);
  sp_define_malloc(deinterleaved, sp_sample_t **,
                   (channel_count * sizeof(sp_sample_t *)));
  local_memory_add(deinterleaved);
  b32 channel = 0;
  SCM a;
  scm_c_list_each(scm_channel_data, a, {
    (*(deinterleaved + channel)) =
        ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(a)));
    increment(channel);
  });
  size_t deinterleaved_count = scm_to_size_t(scm_sample_count);
  size_t interleaved_size =
      (channel_count * deinterleaved_count * sizeof(sp_sample_t *));
  sp_define_malloc(interleaved, sp_sample_t *, interleaved_size);
  local_memory_add(interleaved);
  sp_interleave_and_reverse_endian(deinterleaved, interleaved,
                                   deinterleaved_count, channel_count);
  int count = write((*port_data).data_int, interleaved, interleaved_size);
  if (!(interleaved_size == count)) {
    if ((count < 0)) {
      status_set_both_goto(sp_status_group_libc, count);
    } else {
      status_set_both_goto(sp_status_group_sp, sp_status_id_file_incomplete);
    };
  };
exit:
  local_memory_free;
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_file_read(SCM port, SCM scm_sample_count) {
  status_init;
  SCM result;
  port_data_t *port_data = scm_to_port_data(port);
  b32 channel_count = (*port_data).channel_count;
  local_memory_init(2);
  size_t deinterleaved_count = scm_to_size_t(scm_sample_count);
  size_t interleaved_size =
      (channel_count * deinterleaved_count * sizeof(sp_sample_t));
  sp_define_malloc(interleaved, sp_sample_t *, interleaved_size);
  local_memory_add(interleaved);
  int count = read((*port_data).data_int, interleaved, interleaved_size);
  if (!count) {
    result = SCM_EOF_VAL;
    goto exit;
  } else {
    if ((count < 0)) {
      status_set_both_goto(sp_status_group_libc, count);
    } else {
      if (!(interleaved_size == count)) {
        interleaved_size = count;
        deinterleaved_count =
            (interleaved_size / channel_count / sizeof(sp_sample_t));
      };
    };
  };
  sp_define_malloc(deinterleaved, sp_sample_t **,
                   (channel_count * sizeof(sp_sample_t *)));
  local_memory_add(deinterleaved);
  size_t deinterleaved_size = (interleaved_size / channel_count);
  b32 channel = channel_count;
  while (channel) {
    decrement(channel);
    sp_define_malloc(data, sp_sample_t *, deinterleaved_size);
    (*(deinterleaved + channel)) = data;
  };
  sp_deinterleave_and_reverse_endian(deinterleaved, interleaved,
                                     deinterleaved_count, channel_count);
  result = SCM_EOL;
  channel = channel_count;
  while (channel) {
    decrement(channel);
    result = scm_cons(
        scm_take_f32vector((*(deinterleaved + channel)), deinterleaved_count),
        result);
  };
exit:
  local_memory_free;
  status_to_scm_return(result);
};
/** sp-port integer -> unspecified
   set port to offset in sample data */
SCM scm_sp_file_set_position(SCM port, SCM scm_sample_index) {
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
SCM scm_sp_alsa_open_input(SCM device_name, SCM channel_count, SCM sample_rate,
                           SCM latency) {
  return (sp_alsa_open(1, device_name, channel_count, sample_rate, latency));
};
SCM scm_sp_alsa_open_output(SCM device_name, SCM channel_count, SCM sample_rate,
                            SCM latency) {
  return (sp_alsa_open(0, device_name, channel_count, sample_rate, latency));
};
SCM scm_sp_file_open_input(SCM path) {
  return (sp_file_open(path, 1, SCM_UNDEFINED, SCM_UNDEFINED));
};
SCM scm_sp_file_open_output(SCM path, SCM channel_count, SCM sample_rate) {
  return (sp_file_open(path, 0, channel_count, sample_rate));
};
#endif
#define optional_samples(a, a_len, scm)                                        \
  if (scm_is_true(scm)) {                                                      \
    a = ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(scm)));                       \
    a_len = octets_to_samples(SCM_BYTEVECTOR_LENGTH(scm));                     \
  } else {                                                                     \
    a = 0;                                                                     \
    a_len = 0;                                                                 \
  }
#define optional_index(a, default)                                             \
  ((!scm_is_undefined(start) && scm_is_true(start)) ? scm_to_uint32(a)         \
                                                    : default)
/** lower precision version of sin() that is faster to compute */
f32_s sin_lq(f32_s a) {
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
status_t sp_fft_inverse(sp_sample_t *result, b32 result_len,
                        sp_sample_t *source, b32 source_len) {
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
/** sum numbers with rounding error compensation using kahan summation with
 * neumaier modification */
f32_s float_sum(f32_s *numbers, b32 len) {
  f32_s temp;
  f32_s element;
  f32_s correction = 0;
  dec(len);
  f32_s result = (*(numbers + len));
  while (len) {
    dec(len);
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
f32_s sinc(f32_s a) {
  return (((0 == a) ? 1 : (sin((M_PI * a)) / (M_PI * a))));
};
f32_s sp_blackman(f32_s a, size_t width) {
  return (((0.42 - (0.5 * cos(((2 * M_PI * a) / (width - 1))))) +
           (0.8 * cos(((4 * M_PI * a) / (width - 1))))));
};
/** discrete linear convolution.
  result length must be at least a-len + b-len - 1 */
b0 sp_convolve(sp_sample_t *result, sp_sample_t *a, size_t a_len,
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
/** discrete linear convolution mapping equally sized segments from a continuous
  stream. result length will be a-len. state length will be b-len. if b-len
  sizes changes between calls for the same stream, state.data must be at least
  b-len large and state.size should be unchanged and will be updated
  automatically */
b0 sp_convolve_stream(sp_sample_t *result, sp_sample_t *a, size_t a_len,
                      sp_sample_t *b, size_t b_len, sp_sample_t *state,
                      size_t *state_len) {
  size_t size = (*state_len);
  if (!(size == b_len)) {
    (*state_len) = b_len;
  };
  while (size) {
    dec(size);
    (*(result + size)) = (*(state + size));
  };
  size = ((a_len > b_len) ? a_len : (a_len - b_len));
  sp_convolve(result, a, size, b, b_len);
  size_t a_index = size;
  size_t b_index = 0;
  while ((a_index < a_len)) {
    while ((b_index < b_len)) {
      size = (a_index + b_index);
      if ((size >= a_len)) {
        size = (a_len - (a_index + b_index));
        (*(state + size)) =
            ((*(state + size)) + ((*(a + a_index)) * (*(b + b_index))));
      } else {
        (*(result + size)) =
            ((*(result + size)) + ((*(a + a_index)) * (*(b + b_index))));
      };
      inc(b_index);
    };
    b_index = 0;
    inc(a_index);
  };
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
  while (a_len) {
    a_len = (a_len - 2);
    (*(a + a_len)) = (-1 * (*(a + a_len)));
  };
};
/** f32-s integer -> f32-s
  radians-per-second samples-per-second -> cutoff-value */
#define sp_windowed_sinc_cutoff(freq, sample_rate)                             \
  ((2 * M_PI * freq) / sample_rate)
;
/** approximate impulse response length for a transition factor and
  ensure that the result is odd */
size_t sp_windowed_sinc_ir_length(f32_s transition) {
  b32 result = ceil((4 / transition));
  if (!(result % 2)) {
    inc(result);
  };
  return (result);
};
/** write an impulse response kernel for a windowed sinc filter. uses a blackman
 * window (truncated version) */
b0 sp_windowed_sinc_ir(sp_sample_t *a, size_t a_len, b32 sample_rate,
                       f32_s freq, f32_s transition) {
  b32 index = 0;
  f32_s center_index = ((a_len - 1.0) / 2.0);
  f32_s cutoff = sp_windowed_sinc_cutoff(freq, sample_rate);
  while ((index < a_len)) {
    (*(a + index)) =
        sp_blackman(sinc((2 * cutoff * (index - center_index))), a_len);
    inc(index);
  };
  f32_s a_sum = float_sum(a, a_len);
  while (a_len) {
    dec(a_len);
    (*(a + index)) = ((*(a + index)) / a_sum);
  };
};
boolean sp_windowed_sinc(sp_sample_t *result, sp_sample_t *source,
                         size_t source_len, sp_sample_t *prev, size_t prev_len,
                         sp_sample_t *next, size_t next_len, size_t start,
                         size_t end, b32 sample_rate, f32_s freq,
                         f32_s transition) {
  b32 ir_len = sp_windowed_sinc_ir_length(transition);
  sp_sample_t *ir = malloc((ir_len * sizeof(sp_sample_t)));
  if (!ir) {
    return (1);
  };
  sp_convolve(result, source, source_len, ir, ir_len);
  free(ir);
  return (0);
};
SCM scm_sp_windowed_sinc_x(SCM result, SCM source, SCM scm_prev, SCM scm_next,
                           SCM sample_rate, SCM freq, SCM transition, SCM start,
                           SCM end) {
  b32 source_len = octets_to_samples(SCM_BYTEVECTOR_LENGTH(source));
  sp_sample_t *prev;
  b32 prev_len;
  sp_sample_t *next;
  b32 next_len;
  optional_samples(prev, prev_len, scm_prev);
  optional_samples(next, next_len, scm_next);
  sp_windowed_sinc(
      ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(result))),
      ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(source))), source_len, prev,
      prev_len, next, next_len, optional_index(start, 0),
      optional_index(end, (source_len - 1)), scm_to_uint32(sample_rate),
      scm_to_double(freq), scm_to_double(transition));
  return (SCM_UNSPECIFIED);
};
/** state: (size . data) */
SCM scm_sp_convolve_x(SCM result, SCM a, SCM b, SCM state) {
  b32 a_len = octets_to_samples(SCM_BYTEVECTOR_LENGTH(a));
  b32 b_len = octets_to_samples(SCM_BYTEVECTOR_LENGTH(b));
  SCM scm_state_len = scm_first(state);
  SCM scm_state_data = scm_tail(state);
  size_t state_len = scm_to_size_t(scm_state_len);
  sp_convolve_stream(((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(result))),
                     ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(a))), a_len,
                     ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(b))), b_len,
                     ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(scm_state_data))),
                     &state_len);
  return (((b_len == state_len)
               ? state
               : scm_cons(scm_from_size_t(state_len), scm_state_data)));
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
SCM scm_sp_fft_inverse(SCM source) {
  status_init;
  b32 result_len = ((SCM_BYTEVECTOR_LENGTH(source) - 1) * 2);
  SCM result =
      scm_make_f32vector(scm_from_uint32(result_len), scm_from_uint8(0));
  status_require_x(sp_fft_inverse(
      ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(result))), result_len,
      ((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(source))),
      SCM_BYTEVECTOR_LENGTH(source)));
exit:
  status_to_scm_return(result);
};
SCM scm_sp_moving_average_x(SCM result, SCM source, SCM scm_prev, SCM scm_next,
                            SCM distance, SCM start, SCM end) {
  b32 source_len = octets_to_samples(SCM_BYTEVECTOR_LENGTH(source));
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
SCM scm_float_nearly_equal_p(SCM a, SCM b, SCM margin) {
  return (scm_from_bool(float_nearly_equal_p(scm_to_double(a), scm_to_double(b),
                                             scm_to_double(margin))));
};
SCM scm_f32vector_sum(SCM a, SCM start, SCM end) {
  return (scm_from_double(
      float_sum(((scm_is_undefined(start) ? 0 : scm_to_uint32(start)) +
                 ((f32_s *)(SCM_BYTEVECTOR_CONTENTS(a)))),
                ((scm_is_undefined(end) ? SCM_BYTEVECTOR_LENGTH(a)
                                        : (end - (1 + start))) *
                 sizeof(f32_s)))));
}; /* write samples for a sine wave into data between start at end.
also defines scm-sp-sine!, scm-sp-sine-lq! */
#define define_sp_sine_x(id, sin)                                              \
  b0 id(sp_sample_t *data, b32 start, b32 end, f32_s sample_duration,          \
        f32_s freq, f32_s phase, f32_s amp) {                                  \
    while ((start <= end)) {                                                   \
      (*(data + start)) = (amp * sin((freq * phase * sample_duration)));       \
      inc(phase);                                                              \
      inc(start);                                                              \
    };                                                                         \
  };                                                                           \
  SCM scm_##id(SCM data, SCM start, SCM end, SCM sample_duration, SCM freq,    \
               SCM phase, SCM amp) {                                           \
    id(((sp_sample_t *)(SCM_BYTEVECTOR_CONTENTS(data))), scm_to_uint32(start), \
       scm_to_uint32(end), scm_to_double(sample_duration),                     \
       scm_to_double(freq), scm_to_double(phase), scm_to_double(amp));         \
    return (SCM_UNSPECIFIED);                                                  \
  }
define_sp_sine_x(sp_sine_x, sin);
define_sp_sine_x(sp_sine_lq_x, sin_lq);
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
  scm_c_define_procedure_c("sp-port-sample-rate", 1, 0, 0,
                           scm_sp_port_sample_rate,
                           "sp-port -> integer/boolean");
  scm_c_define_procedure_c("sp-port?", 1, 0, 0, scm_sp_port_p,
                           "sp-port -> boolean");
  scm_c_define_procedure_c("sp-port-type", 1, 0, 0, scm_sp_port_type,
                           "sp-port -> integer");
  scm_c_define_procedure_c("sp-file-open-input", 1, 2, 0,
                           scm_sp_file_open_input,
                           "string -> sp-port\n    path -> sp-port");
  scm_c_define_procedure_c("sp-file-open-output", 1, 2, 0,
                           scm_sp_file_open_output,
                           "string [integer integer] -> sp-port\n    path "
                           "[channel-count sample-rate] -> sp-port");
  scm_c_define_procedure_c("sp-file-write", 2, 1, 0, scm_sp_file_write,
                           "sp-port (f32vector ...):channel-data "
                           "[integer:sample-count] -> boolean\n    write "
                           "sample data to the channels of a file port");
  scm_c_define_procedure_c(
      "sp-file-set-position", 2, 0, 0, scm_sp_file_set_position,
      "sp-port integer:sample-offset -> boolean\n    sample-offset can be "
      "negative, in which case it is from the end of the file");
  scm_c_define_procedure_c(
      "sp-file-read", 2, 0, 0, scm_sp_file_read,
      "sp-port integer:sample-count -> (f32vector ...):channel-data");
  scm_c_define_procedure_c(
      "sp-alsa-open-input", 0, 4, 0, scm_sp_alsa_open_input,
      "[string integer integer integer] -> sp-port\n    [device-name "
      "channel-count sample-rate latency] -> sp-port");
  scm_c_define_procedure_c(
      "sp-alsa-open-output", 0, 4, 0, scm_sp_alsa_open_output,
      "[string integer integer integer] -> sp-port\n    [device-name "
      "channel-count sample-rate latency] -> sp-port");
  scm_c_define_procedure_c("sp-alsa-write", 2, 1, 0, scm_sp_alsa_write,
                           "sp-port (f32vector ...):channel-data "
                           "[integer:sample-count] -> boolean\n    write "
                           "sample data to the channels of an alsa port - to "
                           "the sound card for sound output for example");
  scm_c_define_procedure_c("sp-alsa-read", 2, 0, 0, scm_sp_alsa_read,
                           "port sample-count -> channel-data\n    sp-port "
                           "integer -> (f32vector ...)");
  scm_c_define_procedure_c("sp-fft", 1, 0, 0, scm_sp_fft,
                           "f32vector:value-per-time -> "
                           "f32vector:frequencies-per-time\n    discrete "
                           "fourier transform on the input data");
  scm_c_define_procedure_c("sp-fft-inverse", 1, 0, 0, scm_sp_fft_inverse,
                           "f32vector:frequencies-per-time -> "
                           "f32vector:value-per-time\n    inverse discrete "
                           "fourier transform on the input data");
  scm_c_define_procedure_c("sp-sine!", 7, 0, 0, scm_sp_sine_x,
                           "data start end sample-duration freq phase amp -> "
                           "unspecified\n    f32vector integer integer "
                           "rational rational rational rational");
  scm_c_define_procedure_c("sp-sine-lq!", 7, 0, 0, scm_sp_sine_lq_x,
                           "data start end sample-duration freq phase amp -> "
                           "unspecified\n    f32vector integer integer "
                           "rational rational rational rational\n    faster, "
                           "lower precision version of sp-sine!.\n    "
                           "currently faster by a factor of about 2.6");
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
  scm_c_define_procedure_c("f32vector-sum", 1, 2, 0, scm_f32vector_sum,
                           "f32vector [start end] -> number");
  scm_c_define_procedure_c(
      "float-nearly-equal?", 3, 0, 0, scm_float_nearly_equal_p,
      "a b margin -> boolean\n    number number number -> boolean");
  scm_c_define_procedure_c("sp-convolve!", 3, 0, 0, scm_sp_convolve_x,
                           "a b state:(integer . f32vector) -> state");
};