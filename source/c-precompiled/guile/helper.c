#include <libguile.h>
#include "../main/sph-sp.h"
#include "../foreign/sph/helper.c"
#include "../foreign/sph/guile.c"
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
/** defines scm-sp-sine!, scm-sp-sine-lq! */
#define define_sp_sine_x(scm_id, f) \
  SCM scm_id(SCM scm_data, SCM scm_len, SCM scm_sample_duration, SCM scm_freq, SCM scm_phase, SCM scm_amp) { \
    f((scm_to_sp_sample_count(scm_len)), (scm_to_sp_float(scm_sample_duration)), (scm_to_sp_float(scm_freq)), (scm_to_sp_float(scm_phase)), (scm_to_sp_float(scm_amp)), ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_data)))); \
    return (SCM_UNSPECIFIED); \
  }
