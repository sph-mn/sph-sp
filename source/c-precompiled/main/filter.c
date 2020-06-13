/* implementation of a windowed sinc low-pass and high-pass filter for continuous streams of sample arrays.
   sample-rate, cutoff frequency and transition band width is variable per call.
   build with the information from https://tomroelandts.com/articles/how-to-create-a-simple-low-pass-filter */
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
        status_require((sph_helper_realloc(ir_f_arguments_len, (&(state->ir_f_arguments)))));
      };
      if (state->ir) {
        free((state->ir));
      };
    };
  } else {
    /* new */
    status_require((sph_helper_malloc((sizeof(sp_convolution_filter_state_t)), (&state))));
    status_require((sph_helper_malloc(ir_f_arguments_len, (&(state->ir_f_arguments)))));
    memreg_add(state);
    memreg_add((state->ir_f_arguments));
    state->carryover_alloc_len = 0;
    state->carryover_len = 0;
    state->carryover = 0;
    state->ir_f = ir_f;
    state->ir_f_arguments_len = ir_f_arguments_len;
  };
  memcpy((state->ir_f_arguments), ir_f_arguments, ir_f_arguments_len);
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
        status_require((sph_helper_realloc((carryover_alloc_len * sizeof(sp_sample_t)), (&carryover))));
        state->carryover_alloc_len = carryover_alloc_len;
      };
      /* in any case reset the extension area */
      memset(((state->ir_len - 1) + carryover), 0, ((ir_len - state->ir_len) * sizeof(sp_sample_t)));
    };
  } else {
    status_require((sph_helper_calloc((carryover_alloc_len * sizeof(sp_sample_t)), (&carryover))));
    state->carryover_alloc_len = carryover_alloc_len;
  };
  state->carryover = carryover;
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
  sp_time_t carryover_len;
  carryover_len = (*out_state ? ((*out_state)->ir_len - 1) : 0);
  /* create/update the impulse response kernel */
  status_require((sp_convolution_filter_state_set(ir_f, ir_f_arguments, ir_f_arguments_len, out_state)));
  /* convolve */
  sp_convolve(in, in_len, ((*out_state)->ir), ((*out_state)->ir_len), carryover_len, ((*out_state)->carryover), out_samples);
exit:
  status_return;
}
sp_float_t sp_window_blackman(sp_float_t a, sp_time_t width) { return (((0.42 - (0.5 * cos(((2 * M_PI * a) / (width - 1))))) + (0.08 * cos(((4 * M_PI * a) / (width - 1)))))); }
/** approximate impulse response length for a transition factor and
   ensure that the length is odd */
sp_time_t sp_windowed_sinc_lp_hp_ir_length(sp_float_t transition) {
  sp_time_t a;
  a = ceil((4 / transition));
  if (!(a % 2)) {
    a = (1 + a);
  };
  return (a);
}
status_t sp_null_ir(sp_sample_t** out_ir, sp_time_t* out_len) {
  *out_len = 1;
  return ((sph_helper_calloc((sizeof(sp_sample_t)), out_ir)));
}
status_t sp_passthrough_ir(sp_sample_t** out_ir, sp_time_t* out_len) {
  status_declare;
  status_require((sph_helper_malloc((sizeof(sp_sample_t)), out_ir)));
  **out_ir = 1;
  *out_len = 1;
exit:
  status_return;
}
/** create an impulse response kernel for a windowed sinc low-pass or high-pass filter.
   uses a truncated blackman window.
   allocates out-ir, sets out-len.
   cutoff and transition are as fraction 0..0.5 of the sampling rate */
status_t sp_windowed_sinc_lp_hp_ir(sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_sample_t** out_ir, sp_time_t* out_len) {
  status_declare;
  sp_float_t center_index;
  sp_time_t i;
  sp_sample_t* ir;
  sp_time_t len;
  sp_float_t sum;
  len = sp_windowed_sinc_lp_hp_ir_length(transition);
  center_index = ((len - 1.0) / 2.0);
  status_require((sph_helper_malloc((len * sizeof(sp_sample_t)), (&ir))));
  /* set the windowed sinc
nan can be set here if the freq and transition values are invalid */
  for (i = 0; (i < len); i = (1 + i)) {
    ir[i] = (sp_window_blackman(i, len) * sp_sinc((2 * cutoff * (i - center_index))));
  };
  /* scale to get unity gain */
  sum = sp_sample_array_sum(ir, len);
  for (i = 0; (i < len); i = (1 + i)) {
    ir[i] = (ir[i] / sum);
  };
  if (is_high_pass) {
    sp_spectral_inversion_ir(ir, len);
  };
  *out_ir = ir;
  *out_len = len;
exit:
  status_return;
}
/** like sp-windowed-sinc-ir-lp but for a band-pass or band-reject filter.
   optimisation: if one cutoff is at or above maximum then create only either low-pass or high-pass */
status_t sp_windowed_sinc_bp_br_ir(sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_sample_t** out_ir, sp_time_t* out_len) {
  status_declare;
  sp_sample_t* hp_ir;
  sp_time_t hp_len;
  sp_sample_t* lp_ir;
  sp_time_t lp_len;
  sp_time_t i;
  sp_time_t start;
  sp_time_t end;
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
      start = (((lp_len - 1) / 2) - ((hp_len - 1) / 2));
      end = (hp_len + start);
      out = lp_ir;
      over = hp_ir;
      *out_len = lp_len;
    } else {
      start = (((hp_len - 1) / 2) - ((lp_len - 1) / 2));
      end = (lp_len + start);
      out = hp_ir;
      over = lp_ir;
      *out_len = hp_len;
    };
    /* sum lp and hp ir samples */
    for (i = start; (i < end); i = (1 + i)) {
      out[i] = (over[(i - start)] + out[i]);
    };
    free(over);
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
  status_return;
}
/** maps arguments from the generic ir-f-arguments array.
   arguments is (sp-float-t:cutoff sp-float-t:transition boolean:is-high-pass) */
status_t sp_windowed_sinc_lp_hp_ir_f(void* arguments, sp_sample_t** out_ir, sp_time_t* out_len) {
  sp_float_t cutoff;
  sp_float_t transition;
  boolean is_high_pass;
  cutoff = *((sp_float_t*)(arguments));
  transition = *(1 + ((sp_float_t*)(arguments)));
  is_high_pass = *((boolean*)((2 + ((sp_float_t*)(arguments)))));
  return ((sp_windowed_sinc_lp_hp_ir(cutoff, transition, is_high_pass, out_ir, out_len)));
}
/** maps arguments from the generic ir-f-arguments array.
   arguments is (sp-float-t:cutoff-l sp-float-t:cutoff-h sp-float-t:transition boolean:is-reject) */
status_t sp_windowed_sinc_bp_br_ir_f(void* arguments, sp_sample_t** out_ir, sp_time_t* out_len) {
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
}
/** a windowed sinc low-pass or high-pass filter for segments of continuous streams with
   variable sample-rate, frequency, transition and impulse response type per call.
   * cutoff: as a fraction of the sample rate, 0..0.5
   * transition: like cutoff
   * is-high-pass: if true then it will reduce low frequencies
   * out-state: if zero then state will be allocated.
   * out-samples: owned by the caller. length must be at least in-len */
status_t sp_windowed_sinc_lp_hp(sp_sample_t* in, sp_time_t in_len, sp_float_t cutoff, sp_float_t transition, boolean is_high_pass, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples) {
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
  status_return;
}
/** like sp-windowed-sinc-lp-hp but for a band-pass or band-reject filter */
status_t sp_windowed_sinc_bp_br(sp_sample_t* in, sp_time_t in_len, sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples) {
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
  status_return;
}
/** samples real real pair [integer integer integer] -> state
     define a routine for a fast filter that also supports multiple filter types in one.
     state must hold two elements and is to be allocated and owned by the caller.
     cutoff is as a fraction of the sample rate between 0 and 0.5.
     uses the state-variable filter described here:
     * http://www.cytomic.com/technical-papers
     * http://www.earlevel.com/main/2016/02/21/filters-for-synths-starting-out/ */
#define define_sp_state_variable_filter(suffix, transfer) \
  void sp_state_variable_filter_##suffix(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_time_t q_factor, sp_sample_t* state) { \
    sp_sample_t a1; \
    sp_sample_t a2; \
    sp_sample_t g; \
    sp_sample_t ic1eq; \
    sp_sample_t ic2eq; \
    sp_time_t i; \
    sp_sample_t k; \
    sp_sample_t v0; \
    sp_sample_t v1; \
    sp_sample_t v2; \
    ic1eq = state[0]; \
    ic2eq = state[1]; \
    g = tan((M_PI * cutoff)); \
    k = (2 - (2 * q_factor)); \
    a1 = (1 / (1 + (g * (g + k)))); \
    a2 = (g * a1); \
    for (i = 0; (i < in_count); i = (1 + i)) { \
      v0 = in[i]; \
      v1 = ((a1 * ic1eq) + (a2 * (v0 - ic2eq))); \
      v2 = (ic2eq + (g * v1)); \
      ic1eq = ((2 * v1) - ic1eq); \
      ic2eq = ((2 * v2) - ic2eq); \
      out[i] = transfer; \
    }; \
    state[0] = ic1eq; \
    state[1] = ic2eq; \
  }
define_sp_state_variable_filter(lp, v2)
  define_sp_state_variable_filter(hp, (v0 - (k * v1) - v2))
    define_sp_state_variable_filter(bp, v1)
      define_sp_state_variable_filter(br, (v0 - (k * v1)))
        define_sp_state_variable_filter(peak, (((2 * v2) - v0) + (k * v1)))
          define_sp_state_variable_filter(all, (v0 - (2 * k * v1)))
  /** the sph-sp default precise filter. processing intensive if parameters are change frequently.
   memory for out-state will be allocated and has to be freed with sp-filter-state-free */
  status_t sp_filter(sp_sample_t* in, sp_time_t in_size, sp_float_t cutoff_l, sp_float_t cutoff_h, sp_float_t transition_l, sp_float_t transition_h, boolean is_reject, sp_filter_state_t** out_state, sp_sample_t* out_samples) { sp_windowed_sinc_bp_br(in, in_size, cutoff_l, cutoff_h, transition_l, transition_h, is_reject, out_state, out_samples); }
status_t sp_cheap_filter_state_new(sp_time_t max_size, sp_time_t max_passes, sp_cheap_filter_state_t* out_state) {
  /* allocates and prepares memory and checks if max-passes doesnt exceed the limit
heap memory is to be freed with sp-cheap-filter-state-free but only allocated if max-passes is greater than one */
  status_declare;
  sp_sample_t* in_temp = 0;
  sp_sample_t* out_temp = 0;
  if (1 < max_passes) {
    if (sp_cheap_filter_passes_limit < max_passes) {
      status_set_goto(sp_s_group_sp, sp_s_id_not_implemented);
    } else {
      status_require((sp_sample_array_new(max_size, (&in_temp))));
      status_require((sp_sample_array_new(max_size, (&out_temp))));
    };
  };
  out_state->in_temp = in_temp;
  out_state->out_temp = out_temp;
  memset((out_state->svf_state), 0, (sizeof(sp_sample_t) * 2 * sp_cheap_filter_passes_limit));
exit:
  if (status_is_failure) {
    free(in_temp);
  };
  status_return;
}
void sp_cheap_filter_state_free(sp_cheap_filter_state_t* a) {
  free((a->in_temp));
  free((a->out_temp));
}
/** the sph-sp default fast filter. caller has to manage the state object with sp-cheap-filter-state-new sp-cheap-filter-state-free */
void sp_cheap_filter(sp_state_variable_filter_t type, sp_sample_t* in, sp_time_t in_size, sp_float_t cutoff, sp_time_t passes, sp_float_t q_factor, uint8_t unity_gain, sp_cheap_filter_state_t* state, sp_sample_t* out) {
  status_declare;
  sp_sample_t* in_swap;
  sp_sample_t* in_temp;
  sp_sample_t* out_temp;
  if (1 == passes) {
    type(out, in, in_size, cutoff, q_factor, (state->svf_state));
  } else {
    type((state->in_temp), in, in_size, cutoff, q_factor, (state->svf_state));
    passes = (passes - 1);
    in_temp = state->in_temp;
    out_temp = state->out_temp;
  loop:
    if (1 < passes) {
      type(out_temp, in_temp, in_size, cutoff, q_factor, (passes + state->svf_state));
      sp_sample_array_zero(in_temp, in_size);
      passes = (passes - 1);
      in_swap = in_temp;
      in_temp = out_temp;
      out_temp = in_swap;
      goto loop;
    } else {
      type(out, in_temp, in_size, cutoff, q_factor, (passes + state->svf_state));
    };
  };
  /* reset unused state values */
  if (sp_cheap_filter_passes_limit > passes) {
    memset((state->svf_state), 0, (sizeof(sp_sample_t) * 2 * (sp_cheap_filter_passes_limit - passes)));
  };
  if (unity_gain) {
    sp_sample_array_set_unity_gain(in, in_size, out);
  };
}
/** apply a centered moving average filter to samples between in-window and in-window-end inclusively and write to out.
   removes high frequencies and smoothes data with little distortion in the time domain but the frequency response has large ripples.
   all memory is managed by the caller.
   * prev and next can be null pointers if not available
   * zero is used for unavailable values
   * rounding errors are kept low by using modified kahan neumaier summation */
status_t sp_moving_average(sp_sample_t* in, sp_sample_t* in_end, sp_sample_t* in_window, sp_sample_t* in_window_end, sp_sample_t* prev, sp_sample_t* prev_end, sp_sample_t* next, sp_sample_t* next_end, sp_time_t radius, sp_sample_t* out) {
  status_declare;
  sp_sample_t* in_left;
  sp_sample_t* in_right;
  sp_sample_t* outside;
  sp_sample_t sums[3];
  sp_time_t outside_count;
  sp_time_t in_missing;
  sp_time_t width;
  width = (1 + radius + radius);
  while ((in_window <= in_window_end)) {
    sums[0] = 0;
    sums[1] = 0;
    sums[2] = 0;
    in_left = max(in, (in_window - radius));
    in_right = min(in_end, (in_window + radius));
    sums[1] = sp_sample_array_sum(in_left, (1 + (in_right - in_left)));
    if (((in_window - in_left) < radius) && prev) {
      in_missing = (radius - (in_window - in_left));
      outside = max(prev, (prev_end - in_missing));
      outside_count = (prev_end - outside);
      sums[0] = sp_sample_array_sum(outside, outside_count);
    };
    if (((in_right - in_window) < radius) && next) {
      in_missing = (radius - (in_right - in_window));
      outside = next;
      outside_count = min((next_end - next), in_missing);
      sums[2] = sp_sample_array_sum(outside, outside_count);
    };
    *out = (sp_sample_array_sum(sums, 3) / width);
    out = (1 + out);
    in_window = (1 + in_window);
  };
  status_return;
}
