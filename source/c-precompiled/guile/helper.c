#include <libguile.h>
#include "./guile/main.c"
#include "./foreign/guile/sph/guile.c"
#define optional_sample_rate(a) (scm_is_undefined(a) ? -1 : scm_to_int32(a))
#define optional_channel_count(a) (scm_is_undefined(a) ? -1 : scm_to_int32(a))
#define optional_samples(a, a_len, scm) \
  if (scm_is_true(scm)) { \
    a = ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm))); \
    a_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(scm))); \
  } else { \
    a = 0; \
    a_len = 0; \
  }
#define optional_index(a, default) ((!scm_is_undefined(start) && scm_is_true(start)) ? scm_to_uint32(a) : default)
/** integer -> string */
#define sp_port_type_to_name(a) ((sp_port_type_file == a) ? "file" : ((sp_port_type_alsa == a) ? "alsa" : "unknown"))
#define sp_object_type_port 0
#define sp_object_type_windowed_sinc 1
#define scm_sp_object_type SCM_SMOB_FLAGS
#define scm_sp_object_data SCM_SMOB_DATA
#define status_group_db_guile "db-guile"
#define scm_from_status_error(a) scm_c_error((a.group), (db_guile_status_name(a)), (db_guile_status_description(a)))
#define scm_c_error(group, name, description) scm_call_1(scm_rnrs_raise, (scm_list_4((scm_from_latin1_symbol(group)), (scm_from_latin1_symbol(name)), (scm_cons((scm_from_latin1_symbol("description")), (scm_from_utf8_string(description)))), (scm_cons((scm_from_latin1_symbol("c-routine")), (scm_from_latin1_symbol(__FUNCTION__)))))))
#define scm_from_status_return(result) return ((status_is_success ? result : scm_from_status_error(status)))
#define scm_from_status_dynwind_end_return(result) \
  if (status_is_success) { \
    scm_dynwind_end(); \
    return (result); \
  } else { \
    return ((scm_from_status_error(status))); \
  }
scm_type_windowed_sinc scm_type_port;
SCM scm_rnrs_raise;
/** get a guile scheme object for channel data sample arrays. returns a list of f64vectors.
  eventually frees given data arrays */
SCM scm_take_channel_data(sp_sample_t** a, uint32_t channel_count, uint32_t sample_count) {
  SCM result;
  result = SCM_EOL;
  while (channel_count) {
    dec(channel_count);
    result = scm_cons((scm_take_f64vector((a[channel_count]), sample_count)), result);
  };
  free(a);
  return (result);
};
/** only the result array is allocated, data is referenced to the scm vectors */
sp_sample_t** scm_to_channel_data(SCM a, uint32_t* channel_count, size_t* sample_count) {
  *channel_count = scm_to_uint32((scm_length(a)));
  if (!*channel_count) {
    return (0);
  };
  sp_sample_t** result;
  size_t index;
  result = malloc((*channel_count * sizeof(sp_sample_t*)));
  if (!result) {
    return (0);
  };
  *sample_count = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH((scm_first(a)))));
  index = 0;
  while (!scm_is_null(a)) {
    result[index] = ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS((scm_first(a)))));
    inc(index);
    a = scm_tail(a);
  };
  return (result);
};
/** sp-object type for storing arbitrary pointers */
SCM scm_sp_object_create(void* pointer, uint8_t sp_object_type) {
  SCM result;
  result = scm_new_smob(scm_type_sp_object, ((scm_t_bits)(pointer)));
  SCM_SET_SMOB_FLAGS(result, ((scm_t_bits)(sp_object_type)));
  return (result);
};
int scm_sp_object_print(SCM a, SCM output_port, scm_print_state* print_state) {
  char* result;
  uint8_t type;
  sp_port_t* sp_port;
  result = malloc((70 + 10 + 7 + 10 + 10 + 2 + 2));
  if (!result) {
    return (0);
  };
  type = scm_sp_object_type(a);
  if (sp_object_type_port == type) {
    sp_port = ((sp_port_t*)(scm_sp_object_data(a)));
    sprintf(result, "#<sp-port %lx type:%s sample-rate:%d channel-count:%d closed?:%s input?:%s>", ((void*)(a)), (sp_port_type_to_name((sp_port->type))), (sp_port->sample_rate), (sp_port->channel_count), ((sp_port_bit_closed & sp_port->flags) ? "#t" : "#f"), ((sp_port_bit_input & sp_port->flags) ? "#t" : "#f"));
  } else if (sp_object_type_windowed_sinc == type) {
    sprintf(result, "#<sp-state %lx type:windowed-sinc>", ((void*)(a)));
  } else {
    sprintf(result, "#<sp %lx>", ((void*)(a)));
  };
  scm_display((scm_take_locale_string(result)), output_port);
  return (0);
};
size_t scm_sp_object_free(SCM a) {
  uint8_t type;
  void* data;
  type = SCM_SMOB_FLAGS(a);
  data = ((void*)(scm_sp_object_data(a)));
  if (sp_object_type_windowed_sinc == type) {
    sp_windowed_sinc_state_destroy(data);
  } else if (sp_object_type_port == type) {
    sp_port_close(data);
  };
  return (0);
};
SCM scm_float_nearly_equal_p(SCM a, SCM b, SCM margin) { return ((scm_from_bool((f64_nearly_equal_p((scm_to_double(a)), (scm_to_double(b)), (scm_to_double(margin))))))); };
SCM scm_f64vector_sum(SCM a, SCM start, SCM end) { return ((scm_from_double((f64_sum(((scm_is_undefined(start) ? 0 : scm_to_size_t(start)) + ((f64_s*)(SCM_BYTEVECTOR_CONTENTS(a)))), ((scm_is_undefined(end) ? SCM_BYTEVECTOR_LENGTH(a) : (end - (1 + start))) * sizeof(f64_s))))))); };
SCM scm_f32vector_sum(SCM a, SCM start, SCM end) { return ((scm_from_double((f32_sum(((scm_is_undefined(start) ? 0 : scm_to_size_t(start)) + ((f32_s*)(SCM_BYTEVECTOR_CONTENTS(a)))), ((scm_is_undefined(end) ? SCM_BYTEVECTOR_LENGTH(a) : (end - (1 + start))) * sizeof(f32_s))))))); };