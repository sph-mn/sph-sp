#include <libguile.h>
#include "../main/sph-sp.h"
#include "../foreign/sph/helper.c"
#include "../foreign/sph/guile.c"
#include "./config.c"
#define status_group_sp_guile "sp-guile"
#define scm_from_sp_port(pointer) scm_make_foreign_object_1(scm_type_port, pointer)
#define scm_from_sp_windowed_sinc(pointer) scm_make_foreign_object_1(scm_type_windowed_sinc, pointer)
#define scm_to_sp_port(a) ((sp_port_t*)(scm_foreign_object_ref(a, 0)))
#define scm_to_sp_windowed_sinc(a) ((sp_windowed_sinc_state_t*)(scm_foreign_object_ref(a, 0)))
/** defines scm-sp-sine!, scm-sp-sine-lq! */
#define define_sp_sine_x(scm_id, f) \
  SCM scm_id(SCM scm_data, SCM scm_len, SCM scm_sample_duration, SCM scm_freq, SCM scm_phase, SCM scm_amp) { \
    f((scm_to_sp_sample_count(scm_len)), (scm_to_sp_float(scm_sample_duration)), (scm_to_sp_float(scm_freq)), (scm_to_sp_float(scm_phase)), (scm_to_sp_float(scm_amp)), ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_data)))); \
    return (SCM_UNSPECIFIED); \
  }
#define scm_from_status_error(a) scm_c_error((a.group), (sp_guile_status_name(a)), (sp_guile_status_description(a)))
#define scm_c_error(group, name, description) scm_call_1(scm_rnrs_raise, (scm_list_4((scm_from_latin1_symbol(group)), (scm_from_latin1_symbol(name)), (scm_cons((scm_from_latin1_symbol("description")), (scm_from_utf8_string(description)))), (scm_cons((scm_from_latin1_symbol("c-routine")), (scm_from_latin1_symbol(__FUNCTION__)))))))
#define scm_from_status_return(result) return ((status_is_success ? result : scm_from_status_error(status)))
#define scm_from_status_dynwind_end_return(result) \
  if (status_is_success) { \
    scm_dynwind_end(); \
    return (result); \
  } else { \
    return ((scm_from_status_error(status))); \
  }
/** get the description if available for a status */
uint8_t* sp_guile_status_description(status_t a) {
  char* b;
  if (!strcmp(status_group_sp_guile, (a.group))) {
    b = "";
  } else {
    b = sp_status_description(a);
  };
  return (((uint8_t*)(b)));
};
/** get the name if available for a status */
uint8_t* sp_guile_status_name(status_t a) {
  char* b;
  if (!strcmp(status_group_sp_guile, (a.group))) {
    b = "unknown";
  } else {
    b = sp_status_name(a);
  };
  return (((uint8_t*)(b)));
};
SCM scm_type_port;
SCM scm_type_windowed_sinc;
SCM scm_rnrs_raise;
/** only the result array is allocated, data is referenced to the scm vectors.
  result is set to null if channel-data is empty */
status_t scm_to_channel_data(SCM a, sp_channel_count_t* result_channel_count, sp_sample_count_t* result_sample_count, sp_sample_t*** result_channel_data) {
  status_declare;
  sp_sample_t** channel_data;
  sp_channel_count_t channel_count;
  sp_sample_count_t sample_count;
  sp_channel_count_t i;
  channel_count = scm_to_sp_channel_count((scm_length(a)));
  if (!channel_count) {
    *result_channel_data = 0;
    *result_channel_count = 0;
    *result_sample_count = 0;
    goto exit;
  };
  sample_count = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH((scm_first(a)))));
  status_require((sp_alloc_channel_array(channel_count, sample_count, (&channel_data))));
  for (i = 0; (i < channel_count); i = (1 + i), a = scm_tail(a)) {
    channel_data[i] = ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS((scm_first(a)))));
  };
  *result_channel_data = channel_data;
exit:
  return (status);
};
/** get a guile scheme object for channel data sample arrays. returns a list of sample-vectors.
  eventually frees given data arrays */
SCM scm_c_take_channel_data(sp_sample_t** a, sp_channel_count_t channel_count, sp_sample_count_t sample_count) {
  SCM scm_result;
  scm_result = SCM_EOL;
  while (channel_count) {
    channel_count = (channel_count - 1);
    scm_result = scm_cons((scm_c_take_samples((a[channel_count]), sample_count)), scm_result);
  };
  free(a);
  return (scm_result);
};