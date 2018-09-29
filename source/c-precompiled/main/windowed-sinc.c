/* implementation of a hq windowed sinc filter with a blackman window (common truncated version) for continuous streams.
  variable sample-rate, cutoff radian frequency and transition band width per call.
  depends on some sp functions defined in main.sc and sph-sp.sc.
  sp-windowed-sinc-state-t is used to store impulse response, parameters to create the current impulse response,
  and data needed for the next call */
sp_float_t sp_window_blackman(sp_float_t a, sp_sample_count_t width) { return (((0.42 - (0.5 * cos(((2 * M_PI * a) / (width - 1))))) + (0.8 * cos(((4 * M_PI * a) / (width - 1)))))); };
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
  allocates result, sets result-len */
status_t sp_windowed_sinc_ir(sp_sample_rate_t sample_rate, sp_float_t freq, sp_float_t transition, sp_sample_count_t* result_len, sp_sample_t** result_ir) {
  status_declare;
  sp_float_t center_index;
  sp_float_t cutoff;
  sp_sample_count_t i;
  sp_sample_t* ir;
  sp_sample_count_t len;
  sp_float_t sum;
  len = sp_windowed_sinc_ir_length(transition);
  center_index = ((len - 1.0) / 2.0);
  cutoff = sp_windowed_sinc_cutoff(freq, sample_rate);
  status_require((sph_helper_malloc((len * sizeof(sp_sample_t)), (&ir))));
  for (i = 0; (i < len); i = (1 + i)) {
    ir[i] = sp_window_blackman((sp_sinc((2 * cutoff * (i - center_index)))), len);
  };
  sum = sp_sample_sum(ir, len);
  for (i = 0; (i < len); i = (1 + i)) {
    ir[i] = (ir[i] / sum);
  };
  *result_ir = ir;
  *result_len = len;
exit:
  return (status);
};
void sp_windowed_sinc_state_free(sp_windowed_sinc_state_t* state) {
  free((state->ir));
  free((state->carryover));
  free(state);
};
/** create or update a previously created state object. impulse response array properties are calculated
  from sample-rate, freq and transition.
  eventually frees state.ir. */
status_t sp_windowed_sinc_state_create(sp_sample_rate_t sample_rate, sp_float_t freq, sp_float_t transition, sp_windowed_sinc_state_t** result_state) {
  status_declare;
  sp_sample_t* carryover;
  sp_sample_t* ir;
  sp_sample_count_t ir_len;
  sp_windowed_sinc_state_t* state;
  memreg_init(1);
  /* create state if not exists. re-use if exists and return if ir needs not be updated */
  if (*result_state) {
    state = *result_state;
    if ((state->sample_rate == sample_rate) && (state->freq == freq) && (state->transition == transition)) {
      return (status);
    } else {
      if (state->ir) {
        free((state->ir));
      };
    };
  } else {
    status_require((sph_helper_malloc((sizeof(sp_windowed_sinc_state_t)), (&state))));
    memreg_add(state);
    state->sample_rate = 0;
    state->freq = 0;
    state->ir = 0;
    state->ir_len = 0;
    state->transition = 0;
    state->carryover = 0;
    state->carryover_len = 0;
  };
  /* create new ir */
  status_require((sp_windowed_sinc_ir(sample_rate, freq, transition, (&ir_len), (&ir))));
  /* eventually extend carryover array. the array is never shrunk */
  if (state->carryover) {
    if (ir_len > state->carryover_alloc_len) {
      status_require((sph_helper_realloc((ir_len * sizeof(sp_sample_t)), (&carryover))));
      state->carryover_alloc_len = ir_len;
    };
  } else {
    status_require((sph_helper_calloc((ir_len * sizeof(sp_sample_t)), (&carryover))));
    state->carryover_alloc_len = ir_len;
  };
  /* carryover-alloc-len is the length of the whole array.
    carryover-len is the actual number of elements that has to be carried over from the last call
    and is to be written into the next segment */
  state->carryover = carryover;
  state->carryover_len = (state->carryover_len ? state->carryover_len : 0);
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
  possibly variable sample-rate, frequency and transition per call.
  state can be zero and it will be allocated.
  result-samples length is source-len */
status_t sp_windowed_sinc(sp_sample_t* source, sp_sample_count_t source_len, sp_sample_rate_t sample_rate, sp_float_t freq, sp_float_t transition, sp_windowed_sinc_state_t** result_state, sp_sample_t* result_samples) {
  status_declare;
  status_require((sp_windowed_sinc_state_create(sample_rate, freq, transition, result_state)));
  sp_convolve(source, source_len, ((*result_state)->ir), ((*result_state)->ir_len), ((*result_state)->carryover_len), ((*result_state)->carryover), result_samples);
exit:
  return (status);
};