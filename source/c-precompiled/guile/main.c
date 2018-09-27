/** defines scm-sp-sine!, scm-sp-sine-lq! */
#define define_sp_sine_x(scm_id, id) \
  SCM scm_id(SCM data, SCM len, SCM sample_duration, SCM freq, SCM phase, SCM amp) { \
    id(((sp_sample_t*)(SCM_BYTEVECTOR_CONTENTS(data))), (scm_to_uint32(len)), (scm_to_double(sample_duration)), (scm_to_double(freq)), (scm_to_double(phase)), (scm_to_double(amp))); \
    return (SCM_UNSPECIFIED); \
  }
define_sp_sine_x(scm_sp_sine_x, sp_sine);
define_sp_sine_x(scm_sp_sine_lq_x, sp_sine);
#define scm_sp_port(a) ((sp_port_t*)(scm_sp_object_data(a)))
SCM scm_sp_port_p(SCM a) { return ((scm_from_bool((SCM_SMOB_PREDICATE(scm_type_sp_object, a) && (sp_object_type_port == scm_sp_object_type(a)))))); };
SCM scm_sp_port_channel_count(SCM port) { return ((scm_from_uint32(((scm_sp_port(port))->channel_count)))); };
SCM scm_sp_port_sample_rate(SCM port) { return ((scm_from_uint32(((scm_sp_port(port))->sample_rate)))); };
SCM scm_sp_port_position_p(SCM port) { return ((scm_from_bool((sp_port_bit_position & (scm_sp_port(port))->flags)))); };
SCM scm_sp_port_input_p(SCM port) { return ((scm_from_bool((sp_port_bit_input & (scm_sp_port(port))->flags)))); };
/** returns the current port position in number of octets */
SCM scm_sp_port_position(SCM port) {
  size_t position;
  sp_port_position((&position), (scm_sp_port(port)));
  return ((scm_from_size_t(position)));
};
SCM scm_sp_port_close(SCM a) {
  status_declare;
  status = sp_port_close((scm_sp_port(a)));
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_port_read(SCM scm_port, SCM scm_sample_count) {
  status_declare;
  sp_port_t* port;
  uint32_t sample_count;
  uint32_t channel_count;
  sp_sample_t** data;
  SCM result;
  port = scm_sp_port(scm_port);
  sample_count = scm_to_uint32(scm_sample_count);
  channel_count = port->channel_count;
  data = sp_alloc_channel_array(channel_count, sample_count);
  sp_status_require_alloc(data);
  status_require((sp_port_read(data, port, sample_count)));
  result = scm_take_channel_data(data, channel_count, sample_count);
exit:
  status_to_scm_return(result);
};
SCM scm_sp_port_write(SCM scm_port, SCM scm_channel_data, SCM scm_sample_count) {
  status_declare;
  sp_port_t* port;
  sp_sample_t** data;
  uint32_t channel_count;
  size_t sample_count;
  local_memory_init(1);
  port = scm_sp_port(scm_port);
  data = scm_to_channel_data(scm_channel_data, (&channel_count), (&sample_count));
  sp_status_require_alloc(data);
  local_memory_add(data);
  status_require((sp_port_write(port, sample_count, data)));
exit:
  local_memory_free;
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_port_set_position(SCM scm_port, SCM scm_sample_offset) {
  status_declare;
  status = sp_port_set_position((scm_sp_port(scm_port)), (scm_to_uint64(scm_sample_offset)));
  status_to_scm_return(SCM_UNSPECIFIED);
};
SCM scm_sp_file_open(SCM scm_path, SCM scm_channel_count, SCM scm_sample_rate) {
  status_declare;
  uint8_t* path;
  uint32_t channel_count;
  uint32_t sample_rate;
  SCM result;
  path = scm_to_locale_string(scm_path);
  channel_count = scm_to_uint32(scm_channel_count);
  sample_rate = scm_to_uint32(scm_sample_rate);
  sp_alloc_define(sp_port, sp_port_t*, (sizeof(sp_port_t)));
  status_require((sp_file_open(sp_port, path, channel_count, sample_rate)));
  result = scm_sp_object_create(sp_port, sp_object_type_port);
exit:
  status_to_scm_return(result);
};
SCM scm_sp_alsa_open(SCM scm_device_name, SCM scm_input_p, SCM scm_channel_count, SCM scm_sample_rate, SCM scm_latency) {
  status_declare;
  uint8_t* device_name;
  boolean input_p;
  uint32_t channel_count;
  uint32_t sample_rate;
  uint32_t latency;
  SCM result;
  device_name = scm_to_locale_string(scm_device_name);
  input_p = scm_to_bool(scm_input_p);
  channel_count = scm_to_uint32(scm_channel_count);
  sample_rate = scm_to_uint32(scm_sample_rate);
  latency = scm_to_uint32(scm_latency);
  sp_alloc_define_zero(sp_port, sp_port_t*, (sizeof(sp_port_t)));
  status_require((sp_alsa_open(sp_port, device_name, input_p, channel_count, sample_rate, latency)));
  result = scm_sp_object_create(sp_port, sp_object_type_port);
exit:
  status_to_scm_return(result);
};
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
void sp_guile_init() {
  SCM type_slots;
  SCM scm_symbol_data;
  scm_symbol_data = scm_from_latin1_symbol("data");
  type_slots = scm_list_1(scm_symbol_data);
  scm_type_port = scm_make_foreign_object_type((scm_from_latin1_symbol("sp-port")), type_slots, 0);
  scm_type_windowed_sinc = scm_make_foreign_object_type((scm_from_latin1_symbol("sp-windowed-sinc")), type_slots, 0);
  scm_type_txn;
  scm_make_foreign_object_type((scm_from_latin1_symbol("db-txn")), type_slots, 0);
  init_sp_io();
  init_sp_generate();
  init_sp_transform();
  scm_c_define_procedure_c_init;
  scm_rnrs_raise = scm_c_public_ref("rnrs exceptions", "raise");
  scm_c_define_procedure_c("f32vector-sum", 1, 2, 0, scm_f32vector_sum, ("f32vector [start end] -> number"));
  scm_c_define_procedure_c("f64vector-sum", 1, 2, 0, scm_f64vector_sum, ("f64vector [start end] -> number"));
  scm_c_define_procedure_c("float-nearly-equal?", 3, 0, 0, scm_float_nearly_equal_p, ("a b margin -> boolean\n    number number number -> boolean"));
  scm_c_define_procedure_c("sp-sine!", 6, 0, 0, scm_sp_sine_x, ("data len sample-duration freq phase amp -> unspecified\n    f32vector integer integer rational rational rational rational"));
  scm_c_define_procedure_c("sp-sine-lq!", 6, 0, 0, scm_sp_sine_lq_x, ("data len sample-duration freq phase amp -> unspecified\n    f32vector integer integer rational rational rational rational\n    faster, lower precision version of sp-sine!.\n    currently faster by a factor of about 2.6"));
  scm_c_define_procedure_c("sp-port-close", 1, 0, 0, scm_sp_port_close, ("sp-port -> boolean"));
  scm_c_define_procedure_c("sp-port-input?", 1, 0, 0, scm_sp_port_input_p, ("sp-port -> boolean"));
  scm_c_define_procedure_c("sp-port-position?", 1, 0, 0, scm_sp_port_position_p, ("sp-port -> boolean"));
  scm_c_define_procedure_c("sp-port-position", 1, 0, 0, scm_sp_port_position, ("sp-port -> integer/boolean"));
  scm_c_define_procedure_c("sp-port-channel-count", 1, 0, 0, scm_sp_port_channel_count, ("sp-port -> integer"));
  scm_c_define_procedure_c("sp-port-sample-rate", 1, 0, 0, scm_sp_port_sample_rate, ("sp-port -> integer/boolean"));
  scm_c_define_procedure_c("sp-port?", 1, 0, 0, scm_sp_port_p, ("sp-port -> boolean"));
  scm_c_define_procedure_c("sp-alsa-open", 5, 0, 0, scm_sp_alsa_open, ("device-name input? channel-count sample-rate latency -> sp-port"));
  scm_c_define_procedure_c("sp-file-open", 3, 0, 0, scm_sp_file_open, ("path channel-count sample-rate -> sp-port"));
  scm_c_define_procedure_c("sp-port-write", 2, 1, 0, scm_sp_port_write, ("sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean\n    write sample data to the channels of port"));
  scm_c_define_procedure_c("sp-port-read", 2, 0, 0, scm_sp_port_read, ("sp-port integer:sample-count -> (f32vector ...):channel-data"));
  scm_c_define_procedure_c("sp-port-set-position", 2, 0, 0, scm_sp_port_set_position, ("sp-port integer:sample-offset -> boolean\n    sample-offset can be negative, in which case it is from the end of the port"));
  scm_c_define_procedure_c("sp-fft", 1, 0, 0, scm_sp_fft, ("f32vector:value-per-time -> f32vector:frequencies-per-time\n    discrete fourier transform on the input data"));
  scm_c_define_procedure_c("sp-ifft", 1, 0, 0, scm_sp_ifft, ("f32vector:frequencies-per-time -> f32vector:value-per-time\n    inverse discrete fourier transform on the input data"));
  scm_c_define_procedure_c("sp-moving-average!", 5, 2, 0, scm_sp_moving_average_x, ("result source previous next distance [start end] -> unspecified\n  f32vector f32vector f32vector f32vector integer integer integer [integer]"));
  scm_c_define_procedure_c("sp-windowed-sinc!", 7, 2, 0, scm_sp_windowed_sinc_x, ("result source previous next sample-rate freq transition [start end] -> unspecified\n    f32vector f32vector f32vector f32vector number number integer integer -> boolean"));
  scm_c_define_procedure_c("sp-convolve!", 3, 0, 0, scm_sp_convolve_x, ("a b state:(integer . f32vector) -> state"));
};