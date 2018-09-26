/* implementation of a hq windowed sinc filter with a blackman window (common truncated version) for continuous streams.
  variable sample-rate, cutoff radian frequency and transition band width per call.
  depends on some sp functions defined in main.sc and sph-sp.sc.
  sp-windowed-sinc-state-t is used to store impulse response, parameters to create the current impulse response,
  and data needed for the next call */
/** approximate impulse response length for a transition factor and
  ensure that the length is odd */
size_t sp_windowed_sinc_ir_length(sp_float_t transition) {
  uint32_t result;
  result = ceil((4 / transition));
  if (!(result % 2)) {
    inc(result);
  };
  return (result);
};
/** create an impulse response kernel for a windowed sinc filter. uses a blackman window (truncated version).
  allocates result, sets result-len.
  failed if result is null */
void sp_windowed_sinc_ir(sp_sample_t** result, size_t* result_len, uint32_t sample_rate, sp_float_t freq, sp_float_t transition) {
  size_t len;
  sp_float_t center_index;
  sp_float_t cutoff;
  sp_sample_t* result_temp;
  size_t index;
  sp_float_t result_sum;
  len = sp_windowed_sinc_ir_length(transition);
  *result_len = len;
  center_index = ((len - 1.0) / 2.0);
  cutoff = sp_windowed_sinc_cutoff(freq, sample_rate);
  result_temp = malloc((len * sizeof(sp_sample_t)));
  if (!result_temp) {
    *result = 0;
    return;
  };
  index = 0;
  while ((index < len)) {
    result_temp[index] = sp_window_blackman((sp_sinc((2 * cutoff * (index - center_index)))), len);
    inc(index);
  };
  result_sum = sp_sample_sum(result_temp, len);
  while (len) {
    len = (len - 1);
    result_temp[index] = (result_temp[index] / result_sum);
  };
  *result = result_temp;
};
void sp_windowed_sinc_state_destroy(sp_windowed_sinc_state_t* state) {
  free((state->ir));
  free((state->data));
  free(state);
};
/** create or update a state object. impulse response array properties are calculated
  from sample-rate, freq and transition.
  eventually frees state.ir.
  ir-len-prev data elements have to be copied to the next result */
uint8_t sp_windowed_sinc_state_create(uint32_t sample_rate, sp_float_t freq, sp_float_t transition, sp_windowed_sinc_state_t** state) {
  sp_windowed_sinc_state_t* state_temp;
  size_t ir_len;
  sp_sample_t* ir;
  sp_sample_t* data;
  if (!*state) {
    state_temp = malloc((sizeof(sp_windowed_sinc_state_t)));
    if (!state_temp) {
      return (1);
    };
    state_temp->sample_rate = 0;
    state_temp->freq = 0;
    state_temp->transition = 0;
    state_temp->data = 0;
    state_temp->data_len = 0;
    state_temp->ir_len_prev = 0;
  } else {
    state_temp = *state;
  };
  if ((state_temp->sample_rate == sample_rate) && (state_temp->freq == freq) && (state_temp->transition == transition)) {
    return (0);
  };
  if (state) {
    free((state_temp->ir));
  };
  sp_windowed_sinc_ir((&ir), (&ir_len), sample_rate, freq, transition);
  if (!ir) {
    if (!*state) {
      free(state_temp);
    };
    return (1);
  };
  state_temp->ir_len_prev = (state ? state_temp->ir_len : ir_len);
  if (ir_len > state_temp->data_len) {
    data = calloc(ir_len, (sizeof(sp_sample_t)));
    if (!data) {
      free(ir);
      if (!*state) {
        free(state_temp);
      };
      return (1);
    };
    if (state_temp->data) {
      memcpy(data, (state_temp->data), (state_temp->ir_len_prev * sizeof(sp_sample_t)));
      free((state_temp->data));
    };
    state_temp->data = data;
    state_temp->data_len = ir_len;
  };
  state_temp->ir = ir;
  state_temp->ir_len = ir_len;
  state_temp->sample_rate = sample_rate;
  state_temp->freq = freq;
  statet_temp->transition = transition;
  *state = state_temp;
  return (0);
};
/** a windowed sinc filter for segments of continuous streams with
  sample-rate, frequency and transition variable per call.
  state can be zero and it will be allocated */
status_i_t sp_windowed_sinc(sp_sample_t* result, sp_sample_t* source, size_t source_len, uint32_t sample_rate, sp_float_t freq, sp_float_t transition, sp_windowed_sinc_state_t** state) {
  if (sp_windowed_sinc_state_create(sample_rate, freq, transition, state)) {
    return (1);
  };
  sp_convolve(result, source, source_len, ((*state)->ir), ((*state)->ir_len), ((*state)->data), ((*state)->ir_len_prev));
  return (0);
};