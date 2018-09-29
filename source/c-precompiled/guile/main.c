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
  status = sp_port_set_position((scm_to_sp_port(scm_port)), (scm_to_int64(scm_sample_offset)));
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
  sp_windowed_sinc_state_t state_null = { 0 };
  sp_windowed_sinc_state_t* state;
  if (!SCM_UNDEFINED && scm_is_true(scm_state)) {
    state = scm_to_sp_windowed_sinc(scm_state);
  } else {
    state = scm_gc_malloc_pointerless((sizeof(sp_windowed_sinc_state_t)), "windowed-sinc-state");
    *state = state_null;
  };
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
SCM scm_sp_alsa_open(SCM scm_device_name, SCM scm_is_input, SCM scm_channel_count, SCM scm_sample_rate, SCM scm_latency) {
  status_declare;
  uint8_t* device_name;
  SCM scm_result;
  sp_port_t* port;
  scm_dynwind_begin(0);
  device_name = scm_to_locale_string(scm_device_name);
  scm_dynwind_free(device_name);
  port = scm_gc_malloc_pointerless((sizeof(sp_port_t)), "sp-port");
  status_require((sp_alsa_open(device_name, (scm_to_bool(scm_is_input)), (scm_to_sp_channel_count(scm_channel_count)), (scm_to_sp_sample_rate(scm_sample_rate)), (scm_to_sp_sample_count(scm_latency)), port)));
  scm_result = scm_from_sp_port(port);
exit:
  scm_from_status_dynwind_end_return(scm_result);
};
SCM scm_sp_file_open(SCM scm_path, SCM scm_channel_count, SCM scm_sample_rate) {
  status_declare;
  uint8_t* path;
  SCM scm_result;
  sp_port_t* port;
  scm_dynwind_begin(0);
  path = scm_to_locale_string(scm_path);
  scm_dynwind_free(path);
  port = scm_gc_malloc_pointerless((sizeof(sp_port_t)), "sp-port");
  status_require((sp_file_open(path, (scm_to_sp_channel_count(scm_channel_count)), (scm_to_sp_sample_rate(scm_sample_rate)), port)));
  scm_result = scm_from_sp_port(port);
exit:
  scm_from_status_dynwind_end_return(scm_result);
};
SCM scm_f64vector_sum(SCM a, SCM start, SCM end) { return ((scm_from_double((f64_sum(((scm_is_undefined(start) ? 0 : scm_to_size_t(start)) + ((f64*)(SCM_BYTEVECTOR_CONTENTS(a)))), ((scm_is_undefined(end) ? SCM_BYTEVECTOR_LENGTH(a) : (end - (1 + start))) * sizeof(f64))))))); };
SCM scm_f32vector_sum(SCM a, SCM start, SCM end) { return ((scm_from_double((f32_sum(((scm_is_undefined(start) ? 0 : scm_to_size_t(start)) + ((f32*)(SCM_BYTEVECTOR_CONTENTS(a)))), ((scm_is_undefined(end) ? SCM_BYTEVECTOR_LENGTH(a) : (end - (1 + start))) * sizeof(f32))))))); };
SCM scm_f64_nearly_equal_p(SCM a, SCM b, SCM margin) { return ((scm_from_bool((f64_nearly_equal((scm_to_double(a)), (scm_to_double(b)), (scm_to_double(margin))))))); };
SCM scm_sp_port_read(SCM scm_port, SCM scm_sample_count) {
  status_declare;
  sp_sample_count_t sample_count;
  sp_channel_count_t channel_count;
  sp_sample_t** channel_data;
  sp_port_t* port;
  SCM scm_result;
  channel_data = 0;
  port = scm_to_sp_port(scm_port);
  sample_count = scm_to_sp_sample_count(scm_sample_count);
  channel_count = port->channel_count;
  status_require((sp_alloc_channel_array(channel_count, sample_count, (&channel_data))));
  status_require((sp_port_read(port, sample_count, channel_data)));
  scm_result = scm_c_take_channel_data(channel_data, channel_count, sample_count);
exit:
  if (status_is_failure && channel_data) {
    sp_channel_data_free(channel_data, channel_count);
  };
  scm_from_status_return(scm_result);
};
SCM scm_sp_port_write(SCM scm_port, SCM scm_channel_data, SCM scm_sample_count) {
  status_declare;
  sp_sample_t** channel_data;
  sp_channel_count_t channel_count;
  sp_sample_count_t sample_count;
  channel_data = 0;
  status_require((scm_to_channel_data(scm_channel_data, (&channel_count), (&sample_count), (&channel_data))));
  status_require((sp_port_write((scm_to_sp_port(scm_port)), sample_count, channel_data)));
exit:
  if (channel_data) {
    sp_channel_data_free(channel_data, channel_count);
  };
  scm_from_status_return(SCM_UNSPECIFIED);
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
  scm_c_define_procedure_c("sp-windowed-sinc!", 7, 2, 0, scm_sp_windowed_sinc_x, ("result source previous next sample-rate freq transition [start end] -> unspecified\n    sample-vector sample-vector sample-vector sample-vector number number integer integer -> boolean"));
  scm_c_define_procedure_c("sp-windowed-sinc-state", 3, 1, 0, scm_sp_windowed_sinc_state_create, ("sample-rate radian-frequency transition [state] -> state\n    rational rational rational [sp-windowed-sinc] -> sp-windowed-sinc"));
  scm_c_define_procedure_c("sp-moving-average!", 5, 2, 0, scm_sp_moving_average_x, ("result source previous next radius [start end] -> unspecified\n    sample-vector sample-vector sample-vector sample-vector integer integer integer [integer]"));
  scm_c_define_procedure_c("sp-fft", 1, 0, 0, scm_sp_fft, ("sample-vector:values-at-times -> sample-vector:frequencies\n    discrete fourier transform on the input data"));
  scm_c_define_procedure_c("sp-ifft", 1, 0, 0, scm_sp_ifft, ("sample-vector:frequencies -> sample-vector:values-at-times\n    inverse discrete fourier transform on the input data"));
  scm_c_define_procedure_c("sp-alsa-open", 5, 0, 0, scm_sp_alsa_open, ("device-name is-input channel-count sample-rate latency -> sp-port"));
  scm_c_define_procedure_c("sp-file-open", 3, 0, 0, scm_sp_file_open, ("path channel-count sample-rate -> sp-port"));
  scm_c_define_procedure_c("sp-port-close", 1, 0, 0, scm_sp_port_close, ("sp-port -> boolean"));
  scm_c_define_procedure_c("sp-port-input?", 1, 0, 0, scm_sp_port_input_p, ("sp-port -> boolean"));
  scm_c_define_procedure_c("sp-port-position?", 1, 0, 0, scm_sp_port_position_p, ("sp-port -> boolean"));
  scm_c_define_procedure_c("sp-port-position", 1, 0, 0, scm_sp_port_position, ("sp-port -> integer"));
  scm_c_define_procedure_c("sp-port-channel-count", 1, 0, 0, scm_sp_port_channel_count, ("sp-port -> integer"));
  scm_c_define_procedure_c("sp-port-sample-rate", 1, 0, 0, scm_sp_port_sample_rate, ("sp-port -> integer"));
  scm_c_define_procedure_c("f32vector-sum", 1, 2, 0, scm_f32vector_sum, ("f32vector [start end] -> number"));
  scm_c_define_procedure_c("f64vector-sum", 1, 2, 0, scm_f64vector_sum, ("f64vector [start end] -> number"));
  scm_c_define_procedure_c("f64-nearly-equal?", 3, 0, 0, scm_f64_nearly_equal_p, ("a b margin -> boolean\n    number number number -> boolean"));
  scm_c_define_procedure_c("sp-port-read", 2, 0, 0, scm_sp_port_read, ("sp-port integer:sample-count -> (sample-vector ...):channel-data"));
  scm_c_define_procedure_c("sp-port-write", 2, 1, 0, scm_sp_port_write, ("sp-port (sample-vector ...):channel-data [integer:sample-count] -> boolean\n  write sample data to the channels of port"));
  scm_c_define_procedure_c("sp-port-set-position", 2, 0, 0, scm_sp_port_set_position, ("sp-port integer:sample-offset -> boolean\n    sample-offset can be negative, in which case it is from the end of the port"));
};