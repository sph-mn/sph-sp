#define optional_samples(a, a_len, scm) \
  if (scm_is_true(scm)) { \
    a = ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm))); \
    a_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(scm))); \
  } else { \
    a = 0; \
    a_len = 0; \
  }
#define optional_index(a, default) ((!scm_is_undefined(start) && scm_is_true(start)) ? scm_to_uint32(a) : default)
SCM scm_sp_convolve_x(SCM result, SCM a, SCM b, SCM carryover, SCM carryover_len) {
  uint32_t a_len;
  uint32_t b_len;
  a_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(a)));
  b_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(b)));
  sp_convolve(((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(result))), ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(a))), a_len, ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(b))), b_len, ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(carryover))), (scm_to_size_t(carryover_len)));
  return (SCM_UNSPECIFIED);
};
SCM scm_sp_moving_average_x(SCM result, SCM source, SCM scm_prev, SCM scm_next, SCM distance, SCM start, SCM end) {
  uint32_t source_len;
  sp_sample_t* prev;
  uint32_t prev_len;
  sp_sample_t* next;
  uint32_t next_len;
  source_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(source)));
  optional_samples(prev, prev_len, scm_prev);
  optional_samples(next, next_len, scm_next);
  sp_moving_average(((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(result))), ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(source))), source_len, prev, prev_len, next, next_len, (optional_index(start, 0)), (optional_index(end, (source_len - 1))), (scm_to_uint32(distance)));
  return (SCM_UNSPECIFIED);
};
SCM scm_sp_windowed_sinc_state_create(SCM sample_rate, SCM freq, SCM transition, SCM old_state) {
  sp_windowed_sinc_state_t* state;
  if (scm_is_true(old_state)) {
    state = ((sp_windowed_sinc_state_t*)(scm_sp_object_data(old_state)));
  } else {
    state = 0;
  };
  sp_windowed_sinc_state_create((scm_to_uint32(sample_rate)), (scm_to_double(freq)), (scm_to_double(transition)), (&state));
  return ((scm_is_true(old_state) ? old_state : scm_sp_object_create(state, sp_object_type_windowed_sinc)));
};
SCM scm_sp_windowed_sinc_x(SCM result, SCM source, SCM state) { return (SCM_UNSPECIFIED); };
SCM scm_sp_fft(SCM source) {
  status_declare;
  uint32_t result_len;
  SCM result;
  result_len = ((3 * SCM_BYTEVECTOR_LENGTH(source)) / 2);
  result = scm_make_f32vector((scm_from_uint32(result_len)), (scm_from_uint8(0)));
  status_require((sp_fft(((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(result))), result_len, ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(source))), (SCM_BYTEVECTOR_LENGTH(source)))));
exit:
  status_to_scm_return(result);
};
SCM scm_sp_ifft(SCM source) {
  status_declare;
  uint32_t result_len;
  SCM result;
  result_len = ((SCM_BYTEVECTOR_LENGTH(source) - 1) * 2);
  result = scm_make_f32vector((scm_from_uint32(result_len)), (scm_from_uint8(0)));
  status_require((sp_ifft(((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(result))), result_len, ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(source))), (SCM_BYTEVECTOR_LENGTH(source)))));
exit:
  status_to_scm_return(result);
};
void init_sp_transform() {
  scm_c_define_procedure_c_init;
  scm_c_define_procedure_c("sp-fft", 1, 0, 0, scm_sp_fft, ("f32vector:value-per-time -> f32vector:frequencies-per-time\n    discrete fourier transform on the input data"));
  scm_c_define_procedure_c("sp-ifft", 1, 0, 0, scm_sp_ifft, ("f32vector:frequencies-per-time -> f32vector:value-per-time\n    inverse discrete fourier transform on the input data"));
  scm_c_define_procedure_c("sp-moving-average!", 5, 2, 0, scm_sp_moving_average_x, ("result source previous next distance [start end] -> unspecified\n  f32vector f32vector f32vector f32vector integer integer integer [integer]"));
  scm_c_define_procedure_c("sp-windowed-sinc!", 7, 2, 0, scm_sp_windowed_sinc_x, ("result source previous next sample-rate freq transition [start end] -> unspecified\n    f32vector f32vector f32vector f32vector number number integer integer -> boolean"));
  scm_c_define_procedure_c("sp-convolve!", 3, 0, 0, scm_sp_convolve_x, ("a b state:(integer . f32vector) -> state"));
};