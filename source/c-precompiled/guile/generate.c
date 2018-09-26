/** defines scm-sp-sine!, scm-sp-sine-lq! */
#define define_sp_sine_x(scm_id, id) \
  SCM scm_id(SCM data, SCM len, SCM sample_duration, SCM freq, SCM phase, SCM amp) { \
    id(((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(data))), (scm_to_uint32(len)), (scm_to_double(sample_duration)), (scm_to_double(freq)), (scm_to_double(phase)), (scm_to_double(amp))); \
    return (SCM_UNSPECIFIED); \
  }
define_sp_sine_x(scm_sp_sine_x, sp_sine);
define_sp_sine_x(scm_sp_sine_lq_x, sp_sine);
void init_sp_generate() {
  scm_c_define_procedure_c_init;
  scm_c_define_procedure_c("sp-sine!", 6, 0, 0, scm_sp_sine_x, ("data len sample-duration freq phase amp -> unspecified\n    f32vector integer integer rational rational rational rational"));
  scm_c_define_procedure_c("sp-sine-lq!", 6, 0, 0, scm_sp_sine_lq_x, ("data len sample-duration freq phase amp -> unspecified\n    f32vector integer integer rational rational rational rational\n    faster, lower precision version of sp-sine!.\n    currently faster by a factor of about 2.6"));
};