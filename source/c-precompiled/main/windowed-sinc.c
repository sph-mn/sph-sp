/* implementation of a hq windowed sinc filter with a blackman window (common truncated version) for continuous streams.
  variable sample-rate, cutoff radian frequency and transition band width per call.
  sp-windowed-sinc-state-t is used to store the impulse response, the parameters that where used to create it, and
  data that has to be carried over between calls */
sp_float_t sp_window_blackman(sp_float_t a, sp_sample_count_t width) { return (((0.42 - (0.5 * cos(((2 * M_PI * a) / (width - 1))))) + (0.08 * cos(((4 * M_PI * a) / (width - 1)))))); };
/** approximate impulse response length for a transition factor and
  ensure that the length is odd */
sp_sample_count_t sp_windowed_sinc_ir_length(sp_float_t transition) {
  sp_sample_count_t result;
  result = ceil((4 / transition));
  if (!(result % 2)) {
    result = (1 + result);
  };
  return (result);
};
/** create an impulse response kernel for a windowed sinc filter. uses a blackman window (truncated version).
  allocates result-ir, sets result-len.
  cutoff and transition are as a fraction 0..1 of the sampling rate */
status_t sp_windowed_sinc_ir(sp_float_t cutoff, sp_float_t transition, sp_sample_count_t* result_len, sp_sample_t** result_ir) {
  status_declare;
  sp_float_t center_index;
  sp_sample_count_t i;
  sp_sample_t* ir;
  sp_sample_count_t len;
  sp_float_t sum;
  len = sp_windowed_sinc_ir_length(transition);
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
  *result_ir = ir;
  *result_len = len;
exit:
  return (status);
};
/** a high-pass filter version of the windowed-sinc impulse response with spectral inversion */
status_t sp_windowed_sinc_hp_ir(sp_float_t cutoff, sp_float_t transition, sp_sample_count_t* result_len, sp_sample_t** result_ir) {
  status_declare;
  status_require((sp_windowed_sinc_ir(cutoff, transition, result_len, result_ir)));
  sp_spectral_inversion_ir((*result_ir), (*result_len));
exit:
  return (status);
};
void sp_windowed_sinc_state_free(sp_windowed_sinc_state_t* state) {
  free((state->ir));
  free((state->carryover));
  free(state);
};
/** create or update a previously created state object. impulse response array properties are calculated
  with ir-f from cutoff and transition.
  eventually frees state.ir */
status_t sp_windowed_sinc_state_set(sp_sample_count_t sample_rate, sp_float_t cutoff, sp_float_t transition, sp_windowed_sinc_ir_f_t ir_f, sp_windowed_sinc_state_t** result_state) {
  status_declare;
  sp_sample_t* carryover;
  sp_sample_t* ir;
  sp_sample_count_t ir_len;
  sp_windowed_sinc_state_t* state;
  memreg_init(1);
  /* create state if not exists. re-use if exists and return early if ir needs not be updated */
  if (*result_state) {
    state = *result_state;
    if ((state->sample_rate == sample_rate) && (state->cutoff == cutoff) && (state->transition == transition) && (state->ir_f == ir_f)) {
      return (status);
    } else {
      if (state->ir) {
        free((state->ir));
      };
    };
  } else {
    status_require((sph_helper_malloc((sizeof(sp_windowed_sinc_state_t)), (&state))));
    memreg_add(state);
    state->carryover_alloc_len = 0;
    state->carryover_len = 0;
    state->carryover = 0;
    state->ir = 0;
  };
  /* create new ir */
  status_require((ir_f((sp_windowed_sinc_ir_cutoff(cutoff, sample_rate)), (sp_windowed_sinc_ir_transition(transition, sample_rate)), (&ir_len), (&ir))));
  /* eventually extend carryover array. the array is never shrunk.
carryover-len is at least ir-len - 1.
carryover-alloc-len is the length of the whole array */
  if (state->carryover) {
    if (ir_len > state->carryover_alloc_len) {
      carryover = state->carryover;
      status_require((sph_helper_realloc(((ir_len - 1) * sizeof(sp_sample_t)), (&carryover))));
      state->carryover_alloc_len = (ir_len - 1);
    } else {
      carryover = state->carryover;
    };
  } else {
    status_require((sph_helper_calloc(((ir_len - 1) * sizeof(sp_sample_t)), (&carryover))));
    state->carryover_alloc_len = (ir_len - 1);
  };
  state->carryover = carryover;
  state->ir = ir;
  state->ir_len = ir_len;
  state->sample_rate = sample_rate;
  state->cutoff = cutoff;
  state->transition = transition;
  *result_state = state;
exit:
  if (status_is_failure) {
    memreg_free;
  };
  return (status);
};
/** a windowed sinc filter for segments of continuous streams with
  variable sample-rate, frequency, transition and impulse response type per call.
  * cutoff: radian frequency
  * transition: radian frequency, describes a frequency band.
  * is-high-pass: if true then it will filter low frequencies
  * state: if zero then state will be allocated.
  * result-samples: owned by the caller. length must be at least source-len */
status_t sp_windowed_sinc(sp_sample_t* source, sp_sample_count_t source_len, sp_sample_rate_t sample_rate, sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_windowed_sinc_state_t** result_state, sp_sample_t* result_samples) {
  status_declare;
  sp_sample_count_t carryover_len;
  carryover_len = (*result_state ? ((*result_state)->ir_len - 1) : 0);
  status_require((sp_windowed_sinc_state_set(sample_rate, cutoff, transition, (is_high_pass ? sp_windowed_sinc_hp_ir : sp_windowed_sinc_ir), result_state)));
  sp_convolve(source, source_len, ((*result_state)->ir), ((*result_state)->ir_len), carryover_len, ((*result_state)->carryover), result_samples);
exit:
  return (status);
};