/* implementation of a hq windowed sinc filter with a blackman window (common truncated version) for continuous streams.
  variable sample-rate, cutoff radian frequency and transition band width per call.
  depends on some sp functions defined in main.sc and sph-sp.sc.
  sp-windowed-sinc-state-t is used to store impulse response, parameters to create the current impulse response,
  and data needed for the next call */
sp_float_t sp_window_blackman(sp_float_t a, size_t width) { return (((0.42 - (0.5 * cos(((2 * M_PI * a) / (width - 1))))) + (0.8 * cos(((4 * M_PI * a) / (width - 1)))))); };
/** approximate impulse response length for a transition factor and
  ensure that the length is odd */
size_t sp_windowed_sinc_ir_length(sp_float_t transition) {
  size_t result;
  result = ceil((4 / transition));
  if (!(result % 2)) {
    result = (1 + result);
  };
  return (result);
};
/** create an impulse response kernel for a windowed sinc filter. uses a blackman window (truncated version).
  allocates result, sets result-len */
status_t sp_windowed_sinc_ir(sp_sample_rate_t sample_rate, sp_float_t freq, sp_float_t transition, size_t* result_len, sp_sample_t** result_ir) {
  status_declare;
  sp_float_t center_index;
  sp_float_t cutoff;
  size_t i;
  size_t len;
  sp_float_t sum;
  sp_sample_t* ir;
  len = sp_windowed_sinc_ir_length(transition);
  center_index = ((len - 1.0) / 2.0);
  cutoff = sp_windowed_sinc_cutoff(freq, sample_rate);
  status_require((sph_helper_malloc((len * sizeof(sp_sample_t)), (&ir))));
  for (i = 0; (i < len); i = (1 + i)) {
    ir[i] = sp_window_blackman((sp_sinc((2 * cutoff * (i - center_index)))), len);
  };
  sum = sp_sample_sum(ir, len);
  while (len) {
    len = (len - 1);
    ir[i] = (ir[i] / sum);
  };
  *result_ir = ir;
  *result_len = len;
exit:
  return (status);
};
void sp_windowed_sinc_state_destroy(sp_windowed_sinc_state_t* state) {
  free((state->ir));
  free((state->carryover));
  free(state);
};
/** create or update a state object. impulse response array properties are calculated
  from sample-rate, freq and transition.
  eventually frees state.ir.
  ir-len-prev carryover elements have to be copied to the next result */
status_t sp_windowed_sinc_state_create(sp_sample_rate_t sample_rate, sp_float_t freq, sp_float_t transition, sp_windowed_sinc_state_t** result_state) {
  status_declare;
  sp_sample_t* carryover;
  sp_sample_t* ir;
  size_t ir_len;
  sp_windowed_sinc_state_t* state;
  memreg_init(2);
  /* create state if not exists */
  if (*result_state) {
    state = *result_state;
  } else {
    status_require((sph_helper_malloc((sizeof(sp_windowed_sinc_state_t)), state)));
    memreg_add(state);
    state->sample_rate = 0;
    state->freq = 0;
    state->transition = 0;
    state->carryover = 0;
    state->carryover_len = 0;
    state->ir_len_prev = 0;
  };
  /* re-use ir if nothing changed */
  if ((state->sample_rate == sample_rate) && (state->freq == freq) && (state->transition == transition)) {
    goto exit;
  };
  /* replace ir */
  if (result_state) {
    free((state->ir));
  };
  status_require((sp_windowed_sinc_ir(sample_rate, freq, transition, (&ir_len), (&ir))));
  state->ir_len_prev = (result_state ? state->ir_len : ir_len);
  /* set bigger carryover buffer if needed */
  if (ir_len > state->carryover_len) {
    status_require((sph_helper_calloc((ir_len * sizeof(sp_sample_t)), (&carryover))));
    memreg_add(carryover);
    if (state->carryover) {
      memcpy(carryover, (state->carryover), (state->ir_len_prev * sizeof(sp_sample_t)));
      free((state->carryover));
    };
    state->carryover = carryover;
    state->carryover_len = ir_len;
  };
  state->ir = ir;
  state->ir_len = ir_len;
  state->sample_rate = sample_rate;
  state->freq = freq;
  state->transition = transition;
  *result_state = state;
exit:
  if (status_is_failure) {
    memreg_free;
  };
  return (status);
};
/** a windowed sinc filter for segments of continuous streams with
  sample-rate, frequency and transition variable per call.
  state can be zero and it will be allocated */
status_t sp_windowed_sinc(sp_sample_t* source, size_t source_len, uint32_t sample_rate, sp_float_t freq, sp_float_t transition, sp_sample_t* result_samples, sp_windowed_sinc_state_t** result_state) {
  status_declare;
  status_require((sp_windowed_sinc_state_create(sample_rate, freq, transition, result_state)));
  sp_convolve(source, source_len, ((*result_state)->ir), ((*result_state)->ir_len), ((*result_state)->ir_len_prev), ((*result_state)->carryover), result_samples);
exit:
  return (status);
};