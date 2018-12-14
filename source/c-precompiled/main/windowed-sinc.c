/* implementation of a windowed sinc low-pass and high-pass filter for continuous streams of sample arrays.
  sample-rate, radian cutoff frequency and transition band width is variable per call.
  build with the information on https://tomroelandts.com/articles/how-to-create-a-simple-low-pass-filter */
sp_float_t sp_window_blackman(sp_float_t a, sp_sample_count_t width) { return (((0.42 - (0.5 * cos(((2 * M_PI * a) / (width - 1))))) + (0.08 * cos(((4 * M_PI * a) / (width - 1)))))); };
/** approximate impulse response length for a transition factor and
  ensure that the length is odd */
sp_sample_count_t sp_windowed_sinc_lp_hp_ir_length(sp_float_t transition) {
  sp_sample_count_t result;
  result = ceil((4 / transition));
  if (!(result % 2)) {
    result = (1 + result);
  };
  return (result);
};
/** create an impulse response kernel for a windowed sinc low-pass or high-pass filter.
  uses a truncated blackman window.
  allocates out-ir, sets out-len.
  cutoff and transition are as fraction 0..1 of the sampling rate */
status_t sp_windowed_sinc_lp_hp_ir(sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_sample_t** out_ir, sp_sample_count_t* out_len) {
  status_declare;
  sp_float_t center_index;
  sp_sample_count_t i;
  sp_sample_t* ir;
  sp_sample_count_t len;
  sp_float_t sum;
  len = sp_windowed_sinc_lp_hp_ir_length(transition);
  center_index = ((len - 1.0) / 2.0);
  status_require((sph_helper_malloc((len * sizeof(sp_sample_t)), (&ir))));
  /* nan can be set here if the freq and transition values are invalid */
  for (i = 0; (i < len); i = (1 + i)) {
    ir[i] = (sp_window_blackman(i, len) * sp_sinc((2 * cutoff * (i - center_index))));
  };
  /* scale */
  sum = sp_sample_sum(ir, len);
  for (i = 0; (i < len); i = (1 + i)) {
    ir[i] = (ir[i] / sum);
  };
  if (is_high_pass) {
    sp_spectral_inversion_ir(ir, len);
  };
  *out_ir = ir;
  *out_len = len;
exit:
  return (status);
};
/** like sp-windowed-sinc-ir-lp but for a band-pass or band-reject filter */
status_t sp_windowed_sinc_bp_br_ir(sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition, boolean is_reject, sp_sample_t** out_ir, sp_sample_count_t* out_len) {
  status_declare;
  sp_sample_count_t hp_index;
  sp_sample_t* hp_ir;
  sp_sample_count_t hp_len;
  sp_sample_t* lp_ir;
  sp_sample_count_t lp_len;
  /* assumes that lp and hp ir length will be equal */
  status_require((sp_windowed_sinc_lp_hp_ir(cutoff_l, transition, 0, (&lp_ir), (&lp_len))));
  status_require((sp_windowed_sinc_lp_hp_ir(cutoff_h, transition, 1, (&hp_ir), (&hp_len))));
  if (is_reject) {
    for (hp_index = 0; (hp_index < hp_len); hp_index = (1 + hp_index)) {
      lp_ir[hp_index] = (lp_ir[hp_index] + hp_ir[hp_index]);
    };
    *out_len = lp_len;
    *out_ir = lp_ir;
  } else {
    *out_len = ((lp_len + hp_len) - 1);
    status_require((sph_helper_malloc((*out_len * (sizeof(sp_sample_t))), out_ir)));
    sp_convolve_one(lp_ir, lp_len, hp_ir, hp_len, (*out_ir));
  };
exit:
  return (status);
};
/** maps arguments from the generic ir-f-arguments array.
  arguments is (sp-float-t:cutoff sp-float-t:transition boolean:is-high-pass) */
status_t sp_windowed_sinc_lp_hp_ir_f(void* arguments, sp_sample_t** out_ir, sp_sample_count_t* out_len) {
  sp_float_t cutoff;
  sp_float_t transition;
  boolean is_high_pass;
  cutoff = *((sp_float_t*)(arguments));
  transition = *(1 + ((sp_float_t*)(arguments)));
  is_high_pass = *((boolean*)((2 + ((sp_float_t*)(arguments)))));
  return ((sp_windowed_sinc_lp_hp_ir(cutoff, transition, is_high_pass, out_ir, out_len)));
};
/** maps arguments from the generic ir-f-arguments array.
  arguments is (sp-float-t:cutoff-l sp-float-t:cutoff-h sp-float-t:transition boolean:is-reject) */
status_t sp_windowed_sinc_bp_br_ir_f(void* arguments, sp_sample_t** out_ir, sp_sample_count_t* out_len) {
  sp_float_t cutoff_l;
  sp_float_t cutoff_h;
  sp_float_t transition;
  boolean is_reject;
  cutoff_l = *((sp_float_t*)(arguments));
  cutoff_h = *(1 + ((sp_float_t*)(arguments)));
  transition = *(2 + ((sp_float_t*)(arguments)));
  is_reject = *((boolean*)((3 + ((sp_float_t*)(arguments)))));
  return ((sp_windowed_sinc_bp_br_ir(cutoff_l, cutoff_h, transition, is_reject, out_ir, out_len)));
};
/** a windowed sinc low-pass or high-pass filter for segments of continuous streams with
  variable sample-rate, frequency, transition and impulse response type per call.
  * cutoff: as a fraction of the sample rate, 0..1
  * transition: like cutoff
  * is-high-pass: if true then it will reduce low frequencies
  * out-state: if zero then state will be allocated.
  * out-samples: owned by the caller. length must be at least in-len */
status_t sp_windowed_sinc_lp_hp(sp_sample_t* in, sp_sample_count_t in_len, sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples) {
  status_declare;
  uint8_t a[(sizeof(boolean) + (2 * sizeof(sp_float_t)))];
  uint8_t a_len;
  a_len = (sizeof(boolean) + (2 * sizeof(sp_float_t)));
  *((sp_float_t*)(a)) = cutoff;
  *(1 + ((sp_float_t*)(a))) = transition;
  *((boolean*)((2 + ((sp_float_t*)(a))))) = is_high_pass;
  status_require((sp_convolution_filter(in, in_len, sp_windowed_sinc_lp_hp_ir_f, a, a_len, out_state, out_samples)));
exit:
  return (status);
};
/** like sp-windowed-sinc-lp-hp but for a windowed sinc band-pass or band-reject filter */
status_t sp_windowed_sinc_bp_br(sp_sample_t* in, sp_sample_count_t in_len, sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition, boolean is_reject, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples) {
  status_declare;
  uint8_t a[(sizeof(boolean) + (3 * sizeof(sp_float_t)))];
  uint8_t a_len;
  a_len = (sizeof(boolean) + (3 * sizeof(sp_float_t)));
  *((sp_float_t*)(a)) = cutoff_l;
  *(1 + ((sp_float_t*)(a))) = cutoff_h;
  *(2 + ((sp_float_t*)(a))) = transition;
  *((boolean*)((3 + ((sp_float_t*)(a))))) = is_reject;
  status_require((sp_convolution_filter(in, in_len, sp_windowed_sinc_bp_br_ir_f, a, a_len, out_state, out_samples)));
exit:
  return (status);
};