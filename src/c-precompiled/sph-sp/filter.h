
#define sp_filter_state_t sp_convolution_filter_state_t
#define sp_filter_state_free sp_convolution_filter_state_free
typedef status_t (*sp_convolution_filter_ir_f_t)(void*, sp_sample_t**, sp_time_t*);
typedef struct {
  sp_sample_t* carryover;
  sp_time_t carryover_len;
  sp_time_t carryover_alloc_len;
  sp_sample_t* ir;
  sp_convolution_filter_ir_f_t ir_f;
  void* ir_f_arguments;
  uint8_t ir_f_arguments_len;
  sp_time_t ir_len;
} sp_convolution_filter_state_t;
typedef void (*sp_state_variable_filter_t)(sp_sample_t*, sp_sample_t*, sp_sample_t, sp_sample_t, sp_time_t, sp_sample_t*);
void sp_moving_average(sp_sample_t* in, sp_time_t in_size, sp_sample_t* prev, sp_sample_t* next, sp_time_t radius, sp_sample_t* out);
sp_time_t sp_windowed_sinc_lp_hp_ir_length(sp_sample_t transition);
status_t sp_windowed_sinc_ir(sp_sample_t cutoff, sp_sample_t transition, sp_time_t* result_len, sp_sample_t** result_ir);
void sp_convolution_filter_state_free(sp_convolution_filter_state_t* state);
status_t sp_convolution_filter_state_set(sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state);
status_t sp_convolution_filter(sp_sample_t* in, sp_time_t in_len, sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples);
status_t sp_windowed_sinc_lp_hp_ir(sp_sample_t cutoff, sp_sample_t transition, sp_bool_t is_high_pass, sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_windowed_sinc_bp_br_ir(sp_sample_t cutoff_l, sp_sample_t cutoff_h, sp_sample_t transition_l, sp_sample_t transition_h, sp_bool_t is_reject, sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_windowed_sinc_lp_hp_ir_f(void* arguments, sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_windowed_sinc_bp_br_ir_f(void* arguments, sp_sample_t** out_ir, sp_time_t* out_len);
status_t sp_windowed_sinc_lp_hp(sp_sample_t* in, sp_time_t in_len, sp_sample_t cutoff, sp_sample_t transition, sp_bool_t is_high_pass, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples);
status_t sp_windowed_sinc_bp_br(sp_sample_t* in, sp_time_t in_len, sp_sample_t cutoff_l, sp_sample_t cutoff_h, sp_sample_t transition_l, sp_sample_t transition_h, sp_bool_t is_reject, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples);
status_t sp_windowed_sinc_lp_hp_ir(sp_sample_t cutoff, sp_sample_t transition, sp_bool_t is_high_pass, sp_sample_t** out_ir, sp_time_t* out_len);