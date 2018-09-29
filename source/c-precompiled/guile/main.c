#include "./helper.c"
define_sp_sine_x(scm_sp_sine_x, sp_sine);
define_sp_sine_x(scm_sp_sine_lq_x, sp_sine);
SCM scm_sp_port_channel_count(SCM scm_a) { return ((scm_from_sp_channel_count(((scm_to_sp_port(scm_a))->channel_count)))); };
SCM scm_sp_port_sample_rate(SCM scm_a) { return ((scm_from_sp_sample_rate(((scm_to_sp_port(scm_a))->sample_rate)))); };
SCM scm_sp_port_position_p(SCM scm_a) { return ((scm_from_bool((sp_port_bit_position & (scm_to_sp_port(scm_a))->flags)))); };
SCM scm_sp_port_input_p(SCM scm_a) { return ((scm_from_bool((sp_port_bit_input & (scm_to_sp_port(scm_a))->flags)))); };
/** returns the current port position offset in number of samples */
SCM scm_sp_port_position(SCM scm_a) {
  sp_sample_count_t position;
  sp_port_position((scm_to_sp_port(scm_a)), (&position));
  return ((scm_from_sp_sample_count(position)));
};
SCM scm_sp_port_close(SCM a) {
  status_declare;
  status = sp_port_close((scm_to_sp_port(a)));
  scm_from_status_return(SCM_UNSPECIFIED);
};
SCM scm_sp_port_set_position(SCM scm_port, SCM scm_sample_offset) {
  status_declare;
  status = sp_port_set_position((scm_to_sp_port(scm_port)), (scm_to_sp_sample_count(scm_sample_offset)));
  scm_from_status_return(SCM_UNSPECIFIED);
};
SCM scm_sp_convolve_x(SCM result, SCM a, SCM b, SCM carryover) {
  sp_sample_count_t a_len;
  sp_sample_count_t b_len;
  sp_sample_count_t c_len;
  a_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(a)));
  b_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(b)));
  c_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(b)));
  if (c_len < b_len) {
    scm_c_error(status_group_sp_guile, "invalid-argument-size", "carryover argument bytevector must be at least as large as the second argument bytevector");
  };
  sp_convolve(((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(a))), a_len, ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(b))), b_len, c_len, ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(carryover))), ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(result))));
  return (SCM_UNSPECIFIED);
};
SCM scm_sp_windowed_sinc_state_create(SCM scm_sample_rate, SCM scm_freq, SCM scm_transition, SCM scm_state) {
  status_declare;
  sp_windowed_sinc_state_t* state;
  state = ((!SCM_UNDEFINED && scm_is_true(scm_state)) ? scm_to_sp_windowed_sinc(scm_state) : 0);
  status_require((sp_windowed_sinc_state_create((scm_to_sp_sample_rate(scm_sample_rate)), (scm_to_sp_float(scm_freq)), (scm_to_sp_float(scm_transition)), (&state))));
  scm_state = scm_from_sp_windowed_sinc(state);
exit:
  scm_from_status_return(scm_state);
};
SCM scm_sp_windowed_sinc_x(SCM scm_result, SCM scm_source, SCM scm_sample_rate, SCM scm_freq, SCM scm_transition, SCM scm_state) {
  status_declare;
  sp_windowed_sinc_state_t* state;
  state = scm_to_sp_windowed_sinc(scm_state);
  status = sp_windowed_sinc(((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_source))), (sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(scm_source)))), (scm_to_sp_sample_rate(scm_sample_rate)), (scm_to_sp_float(scm_freq)), (scm_to_sp_float(scm_transition)), (&state), ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_result))));
  scm_from_status_return(SCM_UNSPECIFIED);
};
SCM scm_sp_moving_average_x(SCM scm_result, SCM scm_source, SCM scm_prev, SCM scm_next, SCM scm_radius, SCM scm_start, SCM scm_end) {
  status_declare;
  sp_sample_count_t source_len;
  sp_sample_count_t prev_len;
  sp_sample_count_t next_len;
  sp_sample_t* prev;
  sp_sample_t* next;
  sp_sample_count_t start;
  sp_sample_count_t end;
  source_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(scm_source)));
  start = ((!scm_is_undefined(scm_start) && scm_is_true(scm_start)) ? scm_to_sp_sample_count(scm_start) : 0);
  end = ((!scm_is_undefined(scm_end) && scm_is_true(scm_end)) ? scm_to_sp_sample_count(scm_end) : (source_len - 1));
  if (scm_is_true(scm_prev)) {
    prev = ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_prev)));
    prev_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(scm_prev)));
  } else {
    prev = 0;
    prev_len = 0;
  };
  if (scm_is_true(scm_next)) {
    next = ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_next)));
    next_len = sp_octets_to_samples((SCM_BYTEVECTOR_LENGTH(scm_next)));
  } else {
    next = 0;
    next_len = 0;
  };
  status_require((sp_moving_average(((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_source))), source_len, prev, prev_len, next, next_len, (scm_to_sp_sample_count(scm_radius)), start, end, ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_result))))));
exit:
  scm_from_status_return(SCM_UNSPECIFIED);
};
SCM scm_sp_fft(SCM scm_source) {
  status_declare;
  sp_sample_count_t result_len;
  SCM scm_result;
  result_len = ((3 * SCM_BYTEVECTOR_LENGTH(scm_source)) / 2);
  scm_result = scm_c_make_sp_samples(result_len);
  status_require((sp_fft(result_len, ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_source))), (SCM_BYTEVECTOR_LENGTH(scm_source)), ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_result))))));
exit:
  scm_from_status_return(scm_result);
};
SCM scm_sp_ifft(SCM scm_source) {
  status_declare;
  sp_sample_count_t result_len;
  SCM scm_result;
  result_len = ((SCM_BYTEVECTOR_LENGTH(scm_source) - 1) * 2);
  scm_result = scm_c_make_sp_samples(result_len);
  status_require((sp_ifft(result_len, ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_source))), (SCM_BYTEVECTOR_LENGTH(scm_source)), ((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(scm_result))))));
exit:
  scm_from_status_return(scm_result);
};
void sp_guile_init() {
  SCM type_slots;
  SCM scm_symbol_data;
  scm_c_define_procedure_c_init;
  scm_rnrs_raise = scm_c_public_ref("rnrs exceptions", "raise");
  scm_symbol_data = scm_from_latin1_symbol("data");
  type_slots = scm_list_1(scm_symbol_data);
  scm_type_port = scm_make_foreign_object_type((scm_from_latin1_symbol("sp-port")), type_slots, 0);
  scm_type_windowed_sinc = scm_make_foreign_object_type((scm_from_latin1_symbol("sp-windowed-sinc")), type_slots, 0);
  scm_c_define_procedure_c("sp-sine!", 6, 0, 0, scm_sp_sine_x, ("data len sample-duration freq phase amp -> unspecified\n    sample-vector integer integer rational rational rational rational"));
  scm_c_define_procedure_c("sp-sine-lq!", 6, 0, 0, scm_sp_sine_lq_x, ("data len sample-duration freq phase amp  -> unspecified\n    sample-vector integer integer rational rational rational rational\n    faster, lower precision version of sp-sine!.\n    currently faster by a factor of about 2.6"));
  scm_c_define_procedure_c("sp-convolve!", 4, 0, 0, scm_sp_convolve_x, ("result a b carryover -> unspecified"));
  scm_c_define_procedure_c("sp-windowed-sinc!", 7, 2, 0, scm_sp_windowed_sinc_x, ("result source previous next sample-rate freq transition [start end] -> unspecified\n    f32vector f32vector f32vector f32vector number number integer integer -> boolean"));
  scm_c_define_procedure_c("sp-windowed-sinc-state", 3, 1, 0, scm_sp_windowed_sinc_state_create, ("sample-rate radian-frequency transition [state] -> state\n    rational rational rational [sp-windowed-sinc] -> sp-windowed-sinc"));
  scm_c_define_procedure_c("sp-fft", 1, 0, 0, scm_sp_fft, ("sample-vector:values-at-times -> sample-vector:frequencies\n    discrete fourier transform on the input data"));
  scm_c_define_procedure_c("sp-ifft", 1, 0, 0, scm_sp_ifft, ("sample-vector:frequencies -> sample-vector:values-at-times\n    inverse discrete fourier transform on the input data"));
};