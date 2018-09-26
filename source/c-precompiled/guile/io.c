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
void init_sp_io() {
  scm_c_define_procedure_c_init;
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
};