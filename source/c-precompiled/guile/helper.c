#include <libguile.h>
#include "../main/sph-sp.h"
#include "../foreign/sph/helper.c"
#include "../foreign/sph/guile.c"
#define status_group_sp_guile "sp-guile"
#define scm_from_sp_channel_count(a) scm_from_uint32(a)
#define scm_from_sp_sample_count(a) scm_from_uint32(a)
#define scm_from_sp_sample_rate(a) scm_from_uint32(a)
#define scm_from_sp_sample(a) scm_from_double(a)
#define scm_from_sp_float(a) scm_from_double(a)
#define scm_to_sp_channel_count(a) scm_to_uint32(a)
#define scm_to_sp_sample_count(a) scm_to_uint32(a)
#define scm_to_sp_sample_rate(a) scm_to_uint32(a)
#define scm_to_sp_sample(a) scm_to_double(a)
#define scm_to_sp_float(a) scm_to_double(a)
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