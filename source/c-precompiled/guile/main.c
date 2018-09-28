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
};