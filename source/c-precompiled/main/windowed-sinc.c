/* implementation of a windowed sinc low-pass and high-pass filter for continuous streams of sample arrays.
  sample-rate, radian cutoff frequency and transition band width is variable per call.
  build with the information on https://tomroelandts.com/articles/how-to-create-a-simple-low-pass-filter */
sp_float_t sp_window_blackman(sp_float_t a, sp_sample_count_t width) { return (((0.42 - (0.5 * cos(((2 * M_PI * a) / (width - 1))))) + (0.08 * cos(((4 * M_PI * a) / (width - 1)))))); };
/** approximate impulse response length for a transition factor and
  ensure that the length is odd */
sp_sample_count_t sp_windowed_sinc_lp_hp_ir_length(sp_float_t transition) {
  sp_sample_count_t a;
  a = ceil((4 / transition));
  if (!(a % 2)) {
    a = (1 + a);
  };
  return (a);
};
status_t sp_null_ir(sp_sample_t** out_ir, sp_sample_count_t* out_len) {
  *out_len = 1;
  return ((sph_helper_calloc((sizeof(sp_sample_t)), out_ir)));
};
status_t sp_passthrough_ir(sp_sample_t** out_ir, sp_sample_count_t* out_len) {
  status_declare;
  status_require((sph_helper_malloc((sizeof(sp_sample_t)), out_ir)));
  **out_ir = 1;
  *out_len = 1;
exit:
  return (status);
};
/** create an impulse response kernel for a windowed sinc low-pass or high-pass filter.
  uses a truncated blackman window.
  allocates out-ir, sets out-len.
  cutoff and transition are as fraction 0..0.5 of the sampling rate */
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
  /* set the windowed sinc
nan can be set here if the freq and transition values are invalid */
  for (i = 0; (i < len); i = (1 + i)) {
    ir[i] = (sp_window_blackman(i, len) * sp_sinc((2 * cutoff * (i - center_index))));
  };
  /* scale gain */
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
/** like sp-windowed-sinc-ir-lp but for a band-pass or band-reject filter.
  optimisation: if one cutoff is at or above maximum then create only either low-pass or high-pass */
status_t sp_windowed_sinc_bp_br_ir(sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_sample_t** out_ir, sp_sample_count_t* out_len) {
  status_declare;
  sp_sample_t* hp_ir;
  sp_sample_count_t hp_len;
  sp_sample_t* lp_ir;
  sp_sample_count_t lp_len;
  sp_sample_count_t i;
  sp_sample_count_t start_i;
  sp_sample_count_t end_i;
  sp_sample_t* out;
  sp_sample_t* over;
  if (is_reject) {
    if (0.0 >= cutoff_l) {
      if (0.5 <= cutoff_h) {
        return ((sp_null_ir(out_ir, out_len)));
      } else {
        return ((sp_windowed_sinc_lp_hp_ir(cutoff_h, transition_h, 1, out_ir, out_len)));
      };
    } else {
      if (0.5 <= cutoff_h) {
        return ((sp_windowed_sinc_lp_hp_ir(cutoff_l, transition_l, 0, out_ir, out_len)));
      };
    };
    status_require((sp_windowed_sinc_lp_hp_ir(cutoff_l, transition_l, 0, (&lp_ir), (&lp_len))));
    status_require((sp_windowed_sinc_lp_hp_ir(cutoff_h, transition_h, 1, (&hp_ir), (&hp_len))));
    /* prepare to add the shorter ir to the longer one center-aligned */
    if (lp_len > hp_len) {
      start_i = (((lp_len - 1) / 2) - ((hp_len - 1) / 2));
      end_i = (hp_len + start_i);
      out = lp_ir;
      over = hp_ir;
      *out_len = lp_len;
    } else {
      start_i = (((hp_len - 1) / 2) - ((lp_len - 1) / 2));
      end_i = (lp_len + start_i);
      out = hp_ir;
      over = lp_ir;
      *out_len = hp_len;
    };
    /* sum lp and hp ir samples */
    for (i = start_i; (i <= end_i); i = (1 + i)) {
      out[i] = (over[(i - start_i)] + out[i]);
    };
    *out_ir = out;
  } else {
    /* meaning of cutoff high/low is switched. */
    if (0.0 >= cutoff_l) {
      if (0.5 <= cutoff_h) {
        return ((sp_passthrough_ir(out_ir, out_len)));
      } else {
        return ((sp_windowed_sinc_lp_hp_ir(cutoff_h, transition_h, 0, out_ir, out_len)));
      };
    } else {
      if (0.5 <= cutoff_h) {
        return ((sp_windowed_sinc_lp_hp_ir(cutoff_l, transition_l, 1, out_ir, out_len)));
      };
    };
    status_require((sp_windowed_sinc_lp_hp_ir(cutoff_l, transition_l, 1, (&hp_ir), (&hp_len))));
    status_require((sp_windowed_sinc_lp_hp_ir(cutoff_h, transition_h, 0, (&lp_ir), (&lp_len))));
    /* convolve lp and hp ir samples */
    *out_len = ((lp_len + hp_len) - 1);
    status_require((sph_helper_calloc((*out_len * sizeof(sp_sample_t)), out_ir)));
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
  sp_float_t transition_l;
  sp_float_t transition_h;
  boolean is_reject;
  cutoff_l = *((sp_float_t*)(arguments));
  cutoff_h = *(1 + ((sp_float_t*)(arguments)));
  transition_l = *(2 + ((sp_float_t*)(arguments)));
  transition_h = *(3 + ((sp_float_t*)(arguments)));
  is_reject = *((boolean*)((4 + ((sp_float_t*)(arguments)))));
  return ((sp_windowed_sinc_bp_br_ir(cutoff_l, cutoff_h, transition_l, transition_h, is_reject, out_ir, out_len)));
};
/** a windowed sinc low-pass or high-pass filter for segments of continuous streams with
  variable sample-rate, frequency, transition and impulse response type per call.
  * cutoff: as a fraction of the sample rate, 0..0.5
  * transition: like cutoff
  * is-high-pass: if true then it will reduce low frequencies
  * out-state: if zero then state will be allocated.
  * out-samples: owned by the caller. length must be at least in-len */
status_t sp_windowed_sinc_lp_hp(sp_sample_t* in, sp_sample_count_t in_len, sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples) {
  status_declare;
  uint8_t a[(sizeof(boolean) + (2 * sizeof(sp_float_t)))];
  uint8_t a_len;
  /* set arguments array for ir-f */
  a_len = (sizeof(boolean) + (2 * sizeof(sp_float_t)));
  *((sp_float_t*)(a)) = cutoff;
  *(1 + ((sp_float_t*)(a))) = transition;
  *((boolean*)((2 + ((sp_float_t*)(a))))) = is_high_pass;
  /* apply filter */
  status_require((sp_convolution_filter(in, in_len, sp_windowed_sinc_lp_hp_ir_f, a, a_len, out_state, out_samples)));
exit:
  return (status);
};
/** like sp-windowed-sinc-lp-hp but for a band-pass or band-reject filter */
status_t sp_windowed_sinc_bp_br(sp_sample_t* in, sp_sample_count_t in_len, sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples) {
  status_declare;
  uint8_t a[(sizeof(boolean) + (3 * sizeof(sp_float_t)))];
  uint8_t a_len;
  /* set arguments array for ir-f */
  a_len = (sizeof(boolean) + (4 * sizeof(sp_float_t)));
  *((sp_float_t*)(a)) = cutoff_l;
  *(1 + ((sp_float_t*)(a))) = cutoff_h;
  *(2 + ((sp_float_t*)(a))) = transition_l;
  *(3 + ((sp_float_t*)(a))) = transition_h;
  *((boolean*)((4 + ((sp_float_t*)(a))))) = is_reject;
  /* apply filter */
  status_require((sp_convolution_filter(in, in_len, sp_windowed_sinc_bp_br_ir_f, a, a_len, out_state, out_samples)));
exit:
  return (status);
};