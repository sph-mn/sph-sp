
/** discrete linear convolution for sample arrays, possibly of a continuous stream. maps segments (a, a-len) to out
   using (b, b-len) as the impulse response. b-len must be greater than zero.
   all heap memory is owned and allocated by the caller.
   out length is a-len.
   carryover is previous carryover or an empty array.
   carryover length must at least b-len - 1.
   carryover-len should be zero for the first call or its content should be zeros.
   carryover-len for subsequent calls should be b-len - 1.
   if b-len changed it should be b-len - 1 from the previous call for the first call with the changed b-len.
   if b-len is one then there is no carryover.
   if a-len is smaller than b-len then, with the current implementation, additional performance costs ensue from shifting the carryover array each call.
   carryover is the extension of out for generated values that dont fit into out,
   as a and b are always fully convolved */
void sp_convolve(sp_sample_t* a, sp_time_t a_len, sp_sample_t* b, sp_time_t b_len, sp_time_t carryover_len, sp_sample_t* carryover, sp_sample_t* out) {
  sp_time_t size;
  sp_time_t a_index;
  sp_time_t b_index;
  sp_time_t c_index;
  /* prepare out and carryover */
  if (carryover_len) {
    if (carryover_len <= a_len) {
      /* copy all entries to out and reset */
      memcpy(out, carryover, (carryover_len * sizeof(sp_sample_t)));
      sp_samples_zero(carryover, carryover_len);
      sp_samples_zero((carryover_len + out), (a_len - carryover_len));
    } else {
      /* carryover is larger. move all carryover entries that fit into out */
      memcpy(out, carryover, (a_len * sizeof(sp_sample_t)));
      memmove(carryover, (a_len + carryover), ((carryover_len - a_len) * sizeof(sp_sample_t)));
      sp_samples_zero(((carryover_len - a_len) + carryover), a_len);
    };
  } else {
    sp_samples_zero(out, a_len);
  };
  /* process values that dont lead to carryover */
  size = ((a_len < b_len) ? 0 : (a_len - (b_len - 1)));
  if (size) {
    sp_convolve_one(a, size, b, b_len, out);
  };
  /* process values with carryover */
  for (a_index = size; (a_index < a_len); a_index += 1) {
    for (b_index = 0; (b_index < b_len); b_index += 1) {
      c_index = (a_index + b_index);
      if (c_index < a_len) {
        out[c_index] += (a[a_index] * b[b_index]);
      } else {
        c_index = (c_index - a_len);
        carryover[c_index] += (a[a_index] * b[b_index]);
      };
    };
  };
}
void sp_convolution_filter_state_free(sp_convolution_filter_state_t* state) {
  if (!state) {
    return;
  };
  free((state->ir));
  free((state->carryover));
  free((state->ir_f_arguments));
  free(state);
}

/** create or update a previously created state object.
   impulse response array properties are calculated with ir-f using ir-f-arguments.
   eventually frees state.ir.
   the state object is used to store the impulse response, the parameters that where used to create it and
   overlapping data that has to be carried over between calls.
   ir-f-arguments can be stack allocated and will be copied to state on change */
status_t sp_convolution_filter_state_set(sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state) {
  status_declare;
  sp_sample_t* carryover;
  sp_time_t carryover_alloc_len;
  sp_sample_t* ir;
  sp_time_t ir_len;
  sp_convolution_filter_state_t* state;
  memreg_init(2);
  /* create state if not exists. re-use if exists and return early if ir needs not be updated */
  if (*out_state) {
    /* existing */
    state = *out_state;
    if ((state->ir_f == ir_f) && (ir_f_arguments_len == state->ir_f_arguments_len) && (0 == memcmp((state->ir_f_arguments), ir_f_arguments, ir_f_arguments_len))) {
      /* unchanged */
      status_return;
    } else {
      /* changed */
      if (ir_f_arguments_len > state->ir_f_arguments_len) {
        status_require((sph_realloc(ir_f_arguments_len, (&(state->ir_f_arguments)))));
      };
      if (state->ir) {
        free((state->ir));
      };
    };
  } else {
    /* new */
    status_require((sph_malloc((sizeof(sp_convolution_filter_state_t)), (&state))));
    status_require((sph_malloc(ir_f_arguments_len, (&(state->ir_f_arguments)))));
    memreg_add(state);
    memreg_add((state->ir_f_arguments));
    state->carryover_alloc_len = 0;
    state->carryover = 0;
    state->ir_f = ir_f;
    state->ir_f_arguments_len = ir_f_arguments_len;
  };
  memcpy((state->ir_f_arguments), ir_f_arguments, ir_f_arguments_len);
  /* assumes that ir-len is always greater zero */
  status_require((ir_f(ir_f_arguments, (&ir), (&ir_len))));
  /* eventually extend carryover array. the array is never shrunk.
  carryover-len is at least ir-len - 1.
  carryover-alloc-len is the length of the whole array.
  new and extended areas must be set to zero */
  carryover_alloc_len = (ir_len - 1);
  if (state->carryover) {
    carryover = state->carryover;
    if (ir_len > state->ir_len) {
      if (carryover_alloc_len > state->carryover_alloc_len) {
        status_require((sph_realloc((carryover_alloc_len * sizeof(sp_sample_t)), (&carryover))));
        state->carryover_alloc_len = carryover_alloc_len;
      };
      /* in any case reset the extended area */
      memset(((state->ir_len - 1) + carryover), 0, ((ir_len - state->ir_len) * sizeof(sp_sample_t)));
    };
  } else {
    if (carryover_alloc_len) {
      status_require((sp_samples_new(carryover_alloc_len, (&carryover))));
    } else {
      carryover = 0;
    };
    state->carryover_alloc_len = carryover_alloc_len;
  };
  state->carryover = carryover;
  state->carryover_len = (ir_len - 1);
  state->ir = ir;
  state->ir_len = ir_len;
  *out_state = state;
exit:
  if (status_is_failure) {
    memreg_free;
  };
  status_return;
}

/** convolute samples "in", which can be a segment of a continuous stream, with an impulse response
   kernel created by ir-f with ir-f-arguments. can be used for many types of convolution with dynamic impulse response.
   ir-f is only used when ir-f-arguments changed.
   values that need to be carried over with calls are kept in out-state.
   * out-state: if zero then state will be allocated. owned by caller
   * out-samples: owned by the caller. length must be at least in-len and the number of output samples will be in-len */
status_t sp_convolution_filter(sp_sample_t* in, sp_time_t in_len, sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples) {
  status_declare;
  sp_time_t carryover_len = (*out_state ? (*out_state)->carryover_len : 0);
  /* create/update the impulse response kernel */
  status_require((sp_convolution_filter_state_set(ir_f, ir_f_arguments, ir_f_arguments_len, out_state)));
  /* convolve */
  sp_convolve(in, in_len, ((*out_state)->ir), ((*out_state)->ir_len), carryover_len, ((*out_state)->carryover), out_samples);
exit:
  status_return;
}
sp_sample_t sp_bessel_i0(sp_sample_t value) {
  sp_sample_t absolute_value;
  sp_sample_t squared_value;
  sp_sample_t quarter_squared_value;
  sp_sample_t term_value;
  sp_sample_t sum_value;
  sp_sample_t threshold_value;
  int step_index;
  absolute_value = fabs(value);
  squared_value = (absolute_value * absolute_value);
  quarter_squared_value = (0.25 * squared_value);
  sum_value = 1.0;
  term_value = 1.0;
  threshold_value = 1.0e-16;
  step_index = 0;
  do {
    step_index = (step_index + 1);
    term_value = (term_value * (quarter_squared_value / (step_index * step_index)));
    sum_value = (sum_value + term_value);
  } while ((term_value >= (threshold_value * sum_value)));
  return (sum_value);
}
sp_time_t sp_kaiser_window_length(sp_sample_t transition_width, sp_sample_t beta_value) {
  sp_sample_t attenuation_value;
  sp_sample_t numerator_value;
  sp_sample_t denominator_value;
  sp_sample_t raw_length_value;
  sp_time_t window_length;
  attenuation_value = (8.7 + (beta_value / 0.1102));
  numerator_value = (attenuation_value - 7.95);
  denominator_value = (2.285 * 2.0 * sp_pi * transition_width);
  raw_length_value = (numerator_value / denominator_value);
  window_length = ((sp_time_t)(ceil(raw_length_value)));
  window_length = ((2 * ((window_length + 2) / 2)) + 1);
  return (((0 == window_length) ? 9 : window_length));
}
sp_sample_t sp_kaiser_window(sp_time_t sample_index, sp_time_t window_length, sp_sample_t beta_value) {
  sp_sample_t position_value;
  sp_sample_t absolute_position_value;
  sp_sample_t argument_value;
  sp_sample_t numerator_value;
  sp_sample_t denominator_value;
  position_value = (((2.0 * sample_index) / (window_length - 1.0)) - 1.0);
  absolute_position_value = abs(position_value);
  if (absolute_position_value >= 1.0) {
    return ((0.0));
  };
  argument_value = (beta_value * sqrt((1.0 - (position_value * position_value))));
  numerator_value = sp_bessel_i0(argument_value);
  denominator_value = sp_bessel_i0(beta_value);
  return ((numerator_value / denominator_value));
}
status_t sp_sinc_make_minimum_phase(sp_sample_t* impulse_response, sp_time_t sample_count) {
  status_declare;
  sp_sample_t* real_value_list;
  sp_sample_t* imaginary_value_list;
  sp_time_t sample_index;
  sp_sample_t magnitude_value;
  sp_sample_t phase_value;
  real_value_list = impulse_response;
  status_require((sp_calloc_type(sample_count, sp_sample_t, (&imaginary_value_list))));
  sp_fft(sample_count, real_value_list, imaginary_value_list);
  sample_index = 0;
  while ((sample_index < sample_count)) {
    magnitude_value = sqrt(((real_value_list[sample_index] * real_value_list[sample_index]) + (imaginary_value_list[sample_index] * imaginary_value_list[sample_index])));
    real_value_list[sample_index] = log((magnitude_value + 1.0e-20));
    imaginary_value_list[sample_index] = 0.0;
    sample_index = (sample_index + 1);
  };
  sp_ffti(sample_count, real_value_list, imaginary_value_list);
  sample_index = 0;
  while ((sample_index < sample_count)) {
    real_value_list[sample_index] = (real_value_list[sample_index] / ((sp_sample_t)(sample_count)));
    sample_index = (sample_index + 1);
  };
  if ((sample_count % 2) == 0) {
    sample_index = 1;
    while ((sample_index < (sample_count / 2))) {
      real_value_list[sample_index] = (real_value_list[sample_index] * 2.0);
      real_value_list[(sample_count - sample_index)] = 0.0;
      sample_index = (sample_index + 1);
    };
  } else {
    sample_index = 1;
    while ((sample_index <= (sample_count / 2))) {
      real_value_list[sample_index] = (real_value_list[sample_index] * 2.0);
      real_value_list[(sample_count - sample_index)] = 0.0;
      sample_index = (sample_index + 1);
    };
  };
  sp_fft(sample_count, real_value_list, imaginary_value_list);
  sample_index = 0;
  while ((sample_index < sample_count)) {
    magnitude_value = exp((real_value_list[sample_index]));
    phase_value = imaginary_value_list[sample_index];
    real_value_list[sample_index] = (magnitude_value * cos(phase_value));
    imaginary_value_list[sample_index] = (magnitude_value * sin(phase_value));
    sample_index = (sample_index + 1);
  };
  sp_ffti(sample_count, real_value_list, imaginary_value_list);
  sample_index = 0;
  while ((sample_index < sample_count)) {
    real_value_list[sample_index] = (real_value_list[sample_index] / ((sp_sample_t)(sample_count)));
    sample_index = (sample_index + 1);
  };
  free(imaginary_value_list);
exit:
  status_return;
}
void sp_resonator_normalize_unit_energy(sp_sample_t* impulse_response, sp_time_t sample_count) {
  sp_time_t sample_index;
  sp_sample_t energy_value;
  sp_sample_t gain_value;
  energy_value = 0.0;
  sample_index = 0;
  while ((sample_index < sample_count)) {
    energy_value = (energy_value + (impulse_response[sample_index] * impulse_response[sample_index]));
    sample_index = (sample_index + 1);
  };
  if (energy_value <= 0.0) {
    return;
  };
  gain_value = (1.0 / sqrt(energy_value));
  sample_index = 0;
  while ((sample_index < sample_count)) {
    impulse_response[sample_index] = (impulse_response[sample_index] * gain_value);
    sample_index = (sample_index + 1);
  };
}
status_t sp_resonator_ir(sp_sample_t cutoff_low, sp_sample_t cutoff_high, sp_sample_t transition, sp_sample_t** out_ir, sp_time_t* out_len) {
  status_declare;
  sp_sample_t beta_value;
  sp_time_t sample_count;
  sp_sample_t center_index;
  sp_sample_t* impulse_response;
  sp_time_t sample_index;
  sp_sample_t delta_index;
  sp_sample_t low_value;
  sp_sample_t high_value;
  sp_sample_t window_value;
  beta_value = 8.6;
  if (cutoff_low < 0.0) {
    cutoff_low = 0.0;
  };
  if (cutoff_high > 0.5) {
    cutoff_high = 0.5;
  };
  if (transition <= 0.0) {
    transition = (cutoff_high - cutoff_low);
  };
  sample_count = sp_kaiser_window_length(transition, beta_value);
  center_index = ((((sp_sample_t)(sample_count)) - 1.0) * 0.5);
  status_require((sph_malloc((sample_count * sizeof(sp_sample_t)), (&impulse_response))));
  sample_index = 0;
  while ((sample_index < sample_count)) {
    delta_index = (((sp_sample_t)(sample_index)) - center_index);
    low_value = (2.0 * cutoff_low * sp_sinc((2.0 * cutoff_low * delta_index)));
    high_value = (2.0 * cutoff_high * sp_sinc((2.0 * cutoff_high * delta_index)));
    impulse_response[sample_index] = (high_value - low_value);
    window_value = sp_kaiser_window(sample_index, sample_count, beta_value);
    impulse_response[sample_index] = (impulse_response[sample_index] * window_value);
    sample_index = (sample_index + 1);
  };
  sp_resonator_normalize_unit_energy(impulse_response, sample_count);
  status_require((sp_sinc_make_minimum_phase(impulse_response, sample_count)));
  *out_ir = impulse_response;
  *out_len = sample_count;
exit:
  status_return;
}
status_t sp_resonator_ir_f(void* arguments, sp_sample_t** out_ir, sp_time_t* out_len) {
  sp_sample_t cutoff_low;
  sp_sample_t cutoff_high;
  sp_sample_t transition;
  cutoff_low = *((sp_sample_t*)(arguments));
  cutoff_high = *(((sp_sample_t*)(arguments)) + 1);
  transition = *(((sp_sample_t*)(arguments)) + 2);
  return ((sp_resonator_ir(cutoff_low, cutoff_high, transition, out_ir, out_len)));
}
