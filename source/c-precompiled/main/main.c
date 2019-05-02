#include <stdio.h>
#include <fcntl.h>
#include <sndfile.h>
#include <foreign/mtwister/mtwister.h>
#include <foreign/mtwister/mtwister.c>
#include <foreign/nayuki-fft/fft.c>
#include "../main/sph-sp.h"
#include "../foreign/sph/float.c"
#include "../foreign/sph/helper.c"
#include "../foreign/sph/memreg.c"
#include "../foreign/sph/spline-path.c"
#include "../foreign/sph/quicksort.c"
#include "../foreign/sph/queue.c"
#include "../foreign/sph/thread-pool.c"
#include "../foreign/sph/futures.c"
#define sp_status_declare status_declare_group(sp_status_group_sp)
#define sp_libc_status_require_id(id) \
  if (id < 0) { \
    status_set_both_goto(sp_status_group_libc, id); \
  }
#define sp_libc_status_require(expression) \
  status.id = expression; \
  if (status.id < 0) { \
    status_set_group_goto(sp_status_group_libc); \
  } else { \
    status_reset; \
  }
/** define a deinterleave, interleave or similar routine.
    a: source
    b: target */
#define define_sp_interleave(name, type, body) \
  void name(type** a, type* b, sp_count_t a_size, sp_channel_count_t channel_count) { \
    sp_count_t b_size; \
    sp_channel_count_t channel; \
    b_size = (a_size * channel_count); \
    while (a_size) { \
      a_size = (a_size - 1); \
      channel = channel_count; \
      while (channel) { \
        channel = (channel - 1); \
        b_size = (b_size - 1); \
        body; \
      }; \
    }; \
  }
define_sp_interleave(sp_interleave, sp_sample_t, ({ b[b_size] = (a[channel])[a_size]; }));
define_sp_interleave(sp_deinterleave, sp_sample_t, ({ (a[channel])[a_size] = b[b_size]; }));
/** display a sample array in one line */
void debug_display_sample_array(sp_sample_t* a, sp_count_t len) {
  sp_count_t i;
  printf(("%.17g"), (a[0]));
  for (i = 1; (i < len); i = (1 + i)) {
    printf((" %.17g"), (a[i]));
  };
  printf("\n");
};
/** get a string description for a status id in a status-t */
uint8_t* sp_status_description(status_t a) {
  char* b;
  if (!strcmp(sp_status_group_sp, (a.group))) {
    if (sp_status_id_eof == a.id) {
      b = "end of file";
    } else if (sp_status_id_input_type == a.id) {
      b = "input argument is of wrong type";
    } else if (sp_status_id_not_implemented == a.id) {
      b = "not implemented";
    } else if (sp_status_id_memory == a.id) {
      b = "memory allocation error";
    } else if (sp_status_id_file_incompatible == a.id) {
      b = "file channel count or sample rate is different from what was requested";
    } else if (sp_status_id_file_incomplete == a.id) {
      b = "incomplete write";
    } else {
      b = "";
    };
  } else if (!strcmp(sp_status_group_sndfile, (a.group))) {
    b = ((char*)(sf_error_number((a.id))));
  } else if (!strcmp(sp_status_group_sph, (a.group))) {
    b = sph_helper_status_description(a);
  } else {
    b = "";
  };
  return (b);
};
/** get a single word identifier for a status id in a status-t */
uint8_t* sp_status_name(status_t a) {
  char* b;
  if (0 == strcmp(sp_status_group_sp, (a.group))) {
    if (sp_status_id_input_type == a.id) {
      b = "input-type";
    } else if (sp_status_id_not_implemented == a.id) {
      b = "not-implemented";
    } else if (sp_status_id_memory == a.id) {
      b = "memory";
    } else {
      b = "unknown";
    };
  } else if (0 == strcmp(sp_status_group_sndfile, (a.group))) {
    b = "sndfile";
  } else {
    b = "unknown";
  };
  return (b);
};
void sp_block_free(sp_block_t a) {
  sp_count_t i;
  for (i = 0; (i < a.channels); i = (1 + i)) {
    free(((a.samples)[i]));
  };
};
/** return a newly allocated array for channels with data arrays for each channel */
status_t sp_block_new(sp_channel_count_t channel_count, sp_count_t sample_count, sp_block_t* out_block) {
  status_declare;
  memreg_init(channel_count);
  sp_sample_t* channel;
  sp_count_t i;
  for (i = 0; (i < channel_count); i = (1 + i)) {
    status_require((sph_helper_calloc((sample_count * sizeof(sp_sample_t)), (&channel))));
    memreg_add(channel);
    (out_block->samples)[i] = channel;
  };
  out_block->size = sample_count;
  out_block->channels = channel_count;
exit:
  if (status_is_failure) {
    memreg_free;
  };
  return (status);
};
/** lower precision version of sin() that should be faster */
sp_sample_t sp_sin_lq(sp_float_t a) {
  sp_sample_t b;
  sp_sample_t c;
  b = (4 / M_PI);
  c = (-4 / (M_PI * M_PI));
  return ((((b * a) + (c * a * abs(a)))));
};
/** the normalised sinc function */
sp_float_t sp_sinc(sp_float_t a) { return (((0 == a) ? 1 : (sin((M_PI * a)) / (M_PI * a)))); };
/** all arrays should be input-len and are managed by the caller */
status_id_t sp_fft(sp_count_t input_len, double* input_or_output_real, double* input_or_output_imag) { return ((!Fft_transform(input_or_output_real, input_or_output_imag, input_len))); };
/** [[real, imaginary], ...]:complex-numbers -> real-numbers
  input-length > 0
  output-length = input-length
  output is allocated and owned by the caller */
status_id_t sp_ffti(sp_count_t input_len, double* input_or_output_real, double* input_or_output_imag) { return ((!(1 == Fft_inverseTransform(input_or_output_real, input_or_output_imag, input_len)))); };
#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)
/** apply a centered moving average filter to samples between in-window and in-window-end inclusively and write to out.
   removes high frequencies and smoothes data with little distortion in the time domain but the frequency response has large ripples.
   all memory is managed by the caller.
   * prev and next can be null pointers if not available
   * zero is used for unavailable values
   * rounding errors are kept low by using modified kahan neumaier summation */
status_t sp_moving_average(sp_sample_t* in, sp_sample_t* in_end, sp_sample_t* in_window, sp_sample_t* in_window_end, sp_sample_t* prev, sp_sample_t* prev_end, sp_sample_t* next, sp_sample_t* next_end, sp_count_t radius, sp_sample_t* out) {
  status_declare;
  sp_sample_t* in_left;
  sp_sample_t* in_right;
  sp_sample_t* outside;
  sp_sample_t sums[3];
  sp_count_t outside_count;
  sp_count_t in_missing;
  sp_count_t width;
  width = (1 + radius + radius);
  while ((in_window <= in_window_end)) {
    sums[0] = 0;
    sums[1] = 0;
    sums[2] = 0;
    in_left = max(in, (in_window - radius));
    in_right = min(in_end, (in_window + radius));
    sums[1] = sp_sample_sum(in_left, (1 + (in_right - in_left)));
    if (((in_window - in_left) < radius) && prev) {
      in_missing = (radius - (in_window - in_left));
      outside = max(prev, (prev_end - in_missing));
      outside_count = (prev_end - outside);
      sums[0] = sp_sample_sum(outside, outside_count);
    };
    if (((in_right - in_window) < radius) && next) {
      in_missing = (radius - (in_right - in_window));
      outside = next;
      outside_count = min((next_end - next), in_missing);
      sums[2] = sp_sample_sum(outside, outside_count);
    };
    *out = (sp_sample_sum(sums, 3) / width);
    out = (1 + out);
    in_window = (1 + in_window);
  };
  return (status);
};
/** modify an impulse response kernel for spectral inversion.
   a-len must be odd and "a" must have left-right symmetry.
  flips the frequency response top to bottom */
void sp_spectral_inversion_ir(sp_sample_t* a, sp_count_t a_len) {
  sp_count_t center;
  sp_count_t i;
  for (i = 0; (i < a_len); i = (1 + i)) {
    a[i] = (-1 * a[i]);
  };
  center = ((a_len - 1) / 2);
  a[center] = (1 + a[center]);
};
/** inverts the sign for samples at odd indexes.
  a-len must be odd and "a" must have left-right symmetry.
  flips the frequency response left to right */
void sp_spectral_reversal_ir(sp_sample_t* a, sp_count_t a_len) {
  while ((a_len > 1)) {
    a_len = (a_len - 2);
    a[a_len] = (-1 * a[a_len]);
  };
};
/** discrete linear convolution.
  result-samples must be all zeros, its length must be at least a-len + b-len - 1.
  result-samples is owned and allocated by the caller */
void sp_convolve_one(sp_sample_t* a, sp_count_t a_len, sp_sample_t* b, sp_count_t b_len, sp_sample_t* result_samples) {
  sp_count_t a_index;
  sp_count_t b_index;
  a_index = 0;
  b_index = 0;
  while ((a_index < a_len)) {
    while ((b_index < b_len)) {
      result_samples[(a_index + b_index)] = (result_samples[(a_index + b_index)] + (a[a_index] * b[b_index]));
      b_index = (1 + b_index);
    };
    b_index = 0;
    a_index = (1 + a_index);
  };
};
/** discrete linear convolution for sample arrays, possibly of a continuous stream. maps segments (a, a-len) to result-samples
  using (b, b-len) as the impulse response. b-len must be greater than zero.
  all heap memory is owned and allocated by the caller.
  result-samples length is a-len.
  result-carryover is previous carryover or an empty array.
  result-carryover length must at least b-len - 1.
  carryover-len should be zero for the first call or its content should be zeros.
  carryover-len for subsequent calls should be b-len - 1 or if b-len changed b-len - 1  from the previous call.
  if b-len is one then there is no carryover.
  if a-len is smaller than b-len then, with the current implementation, additional performance costs ensue from shifting the carryover array each call.
  carryover is the extension of result-samples for generated values that dont fit */
void sp_convolve(sp_sample_t* a, sp_count_t a_len, sp_sample_t* b, sp_count_t b_len, sp_count_t carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples) {
  sp_count_t size;
  sp_count_t a_index;
  sp_count_t b_index;
  sp_count_t c_index;
  if (carryover_len) {
    if (carryover_len <= a_len) {
      /* copy all entries to result and reset */
      memcpy(result_samples, result_carryover, (carryover_len * sizeof(sp_sample_t)));
      memset(result_carryover, 0, (carryover_len * sizeof(sp_sample_t)));
      memset((carryover_len + result_samples), 0, ((a_len - carryover_len) * sizeof(sp_sample_t)));
    } else {
      /* carryover is larger. set result-samples to all carryover entries that fit.
shift remaining carryover to the left */
      memcpy(result_samples, result_carryover, (a_len * sizeof(sp_sample_t)));
      memmove(result_carryover, (a_len + result_carryover), ((carryover_len - a_len) * sizeof(sp_sample_t)));
      memset(((carryover_len - a_len) + result_carryover), 0, (a_len * sizeof(sp_sample_t)));
    };
  } else {
    memset(result_samples, 0, (a_len * sizeof(sp_sample_t)));
  };
  /* result values.
first process values that dont lead to carryover */
  size = ((a_len < b_len) ? 0 : (a_len - (b_len - 1)));
  if (size) {
    sp_convolve_one(a, size, b, b_len, result_samples);
  };
  /* process values with carryover */
  for (a_index = size; (a_index < a_len); a_index = (1 + a_index)) {
    for (b_index = 0; (b_index < b_len); b_index = (1 + b_index)) {
      c_index = (a_index + b_index);
      if (c_index < a_len) {
        result_samples[c_index] = (result_samples[c_index] + (a[a_index] * b[b_index]));
      } else {
        c_index = (c_index - a_len);
        result_carryover[c_index] = (result_carryover[c_index] + (a[a_index] * b[b_index]));
      };
    };
  };
};
void sp_convolution_filter_state_free(sp_convolution_filter_state_t* state) {
  if (!state) {
    return;
  };
  free((state->ir));
  free((state->carryover));
  free((state->ir_f_arguments));
  free(state);
};
/** create or update a previously created state object.
  impulse response array properties are calculated with ir-f using ir-f-arguments.
  eventually frees state.ir.
  the state object is used to store the impulse response, the parameters that where used to create it and
  overlapping data that has to be carried over between calls.
  ir-f-arguments can be stack allocated and will be copied to state on change */
status_t sp_convolution_filter_state_set(sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state) {
  status_declare;
  sp_sample_t* carryover;
  sp_count_t carryover_alloc_len;
  sp_sample_t* ir;
  sp_count_t ir_len;
  sp_convolution_filter_state_t* state;
  memreg_init(2);
  /* create state if not exists. re-use if exists and return early if ir needs not be updated */
  if (*out_state) {
    /* existing */
    state = *out_state;
    if ((state->ir_f == ir_f) && (ir_f_arguments_len == state->ir_f_arguments_len) && (0 == memcmp((state->ir_f_arguments), ir_f_arguments, ir_f_arguments_len))) {
      /* unchanged */
      return (status);
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
  return (status);
};
/** convolute samples "in", which can be a segment of a continuous stream, with an impulse response
  kernel created by ir-f with ir-f-arguments.
  ir-f is only used when ir-f-arguments changed.
  values that need to be carried over with calls are kept in out-state.
  * out-state: if zero then state will be allocated. owned by caller
  * out-samples: owned by the caller. length must be at least in-len and the number of output samples will be in-len */
status_t sp_convolution_filter(sp_sample_t* in, sp_count_t in_len, sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples) {
  status_declare;
  sp_count_t carryover_len;
  carryover_len = (*out_state ? ((*out_state)->ir_len - 1) : 0);
  /* create/update the impulse response kernel */
  status_require((sp_convolution_filter_state_set(ir_f, ir_f_arguments, ir_f_arguments_len, out_state)));
  /* convolve */
  sp_convolve(in, in_len, ((*out_state)->ir), ((*out_state)->ir_len), carryover_len, ((*out_state)->carryover), out_samples);
exit:
  return (status);
};
/** samples real real pair [integer integer integer] -> state
    define a routine for a fast filter that supports multiple filter types in one.
    state must hold two elements and is allocated and owned by the caller.
    cutoff is as a fraction of the sample rate between 0 and 0.5.
    uses the state-variable filter described here:
    * http://www.cytomic.com/technical-papers
    * http://www.earlevel.com/main/2016/02/21/filters-for-synths-starting-out/ */
#define define_sp_state_variable_filter(suffix, transfer) \
  void sp_state_variable_filter_##suffix(sp_sample_t* out, sp_sample_t* in, sp_float_t in_count, sp_float_t cutoff, sp_count_t q_factor, sp_sample_t* state) { \
    sp_sample_t a1; \
    sp_sample_t a2; \
    sp_sample_t g; \
    sp_sample_t ic1eq; \
    sp_sample_t ic2eq; \
    sp_count_t i; \
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
define_sp_state_variable_filter(lp, v2);
define_sp_state_variable_filter(hp, (v0 - (k * v1) - v2));
define_sp_state_variable_filter(bp, v1);
define_sp_state_variable_filter(br, (v0 - (k * v1)));
define_sp_state_variable_filter(peak, (((2 * v2) - v0) + (k * v1)));
define_sp_state_variable_filter(all, (v0 - (2 * k * v1)));
/** writes a sine wave of size into out. can be used as a lookup table */
status_t sp_sine_table_new(sp_sample_t** out, sp_count_t size) {
  status_declare;
  sp_count_t i;
  sp_sample_t* out_array;
  status_require((sph_helper_malloc((size * sizeof(sp_sample_t*)), (&out_array))));
  for (i = 0; (i < size); i = (1 + i)) {
    out_array[i] = sin((i * (M_PI / (size / 2))));
  };
  *out = out_array;
exit:
  return (status);
};
/** fills the sine wave lookup table */
status_t sp_initialise() { return ((sp_sine_table_new((&sp_sine_96_table), 96000))); };
/** accumulate an integer phase and reset it after cycles.
  float value phases would be harder to reset */
sp_count_t sp_cheap_phase_96(sp_count_t current, sp_count_t change) {
  sp_count_t result;
  result = (current + change);
  return (((96000 <= result) ? (result % 96000) : result));
};
/** accumulate an integer phase with change given as a float value.
  change must be a positive value and is rounded to the next larger integer */
sp_count_t sp_cheap_phase_96_float(sp_count_t current, double change) { return ((sp_cheap_phase_96(current, (sp_cheap_ceiling_positive(change))))); };
/** contains the initial phase offsets per partial and channel
  as a flat array */
status_t sp_synth_state_new(sp_count_t channel_count, sp_synth_count_t config_len, sp_synth_partial_t* config, sp_count_t** out_state) {
  status_declare;
  sp_count_t i;
  sp_count_t channel_i;
  status_require((sph_helper_calloc((channel_count * config_len * sizeof(sp_count_t)), out_state)));
  for (i = 0; (i < config_len); i = (1 + i)) {
    for (channel_i = 0; (channel_i < channel_count); channel_i = (1 + channel_i)) {
      (*out_state)[(channel_i + (channel_count * i))] = ((config[i]).phs)[channel_i];
    };
  };
exit:
  return (status);
};
/** create sines that start and end at specific times and can optionally modulate the frequency of others.
  sp-synth output is summed into out.
  amplitude and wavelength can be controlled by arrays separately for each partial and channel.
  modulators can be modulated themselves in chains. state has to be allocated by the caller with sp-synth-state-new.
  modulator amplitude is relative to carrier wavelength.
  paths are relative to the start of partials.
  # requirements
  * modulators must come after carriers in config
  * config-len must not change between calls with the same state
  * all amplitude/wavelength arrays must be of sufficient size and available for all channels
  * sp-initialise must have been called once before using sp-synth
  # algorithm
  * read config from the end to the start
  * write modulator output to temporary buffers that are indexed by modified partial id
  * apply modulator output from the buffers and sum to output for final carriers
  * each partial has integer phases that are reset in cycles and kept in state between calls */
status_t sp_synth(sp_block_t out, sp_count_t start, sp_count_t duration, sp_synth_count_t config_len, sp_synth_partial_t* config, sp_count_t* phases) {
  status_declare;
  sp_sample_t amp;
  sp_sample_t* carrier;
  sp_count_t channel_i;
  sp_count_t end;
  sp_count_t i;
  sp_sample_t modulated_wvl;
  sp_sample_t* modulation_index[sp_synth_partial_limit][sp_synth_channel_limit];
  sp_sample_t* modulation;
  sp_count_t phs;
  sp_count_t prt_duration;
  sp_synth_count_t prt_i;
  sp_count_t prt_offset_right;
  sp_count_t prt_offset;
  sp_synth_partial_t prt;
  sp_count_t prt_start;
  sp_count_t wvl;
  /* modulation blocks (channel array + data. at least one is carrier and writes only to output) */
  memreg_init(((config_len - 1) * out.channels));
  memset(modulation_index, 0, (sizeof(modulation_index)));
  end = (start + duration);
  prt_i = config_len;
  while (prt_i) {
    prt_i = (prt_i - 1);
    prt = config[prt_i];
    if (end < prt.start) {
      break;
    };
    if (prt.end <= start) {
      continue;
    };
    prt_start = ((prt.start < start) ? (start - prt.start) : 0);
    prt_offset = ((prt.start > start) ? (prt.start - start) : 0);
    prt_offset_right = ((prt.end > end) ? 0 : (end - prt.end));
    prt_duration = (duration - prt_offset - prt_offset_right);
    if (prt.modifies) {
      for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
        carrier = modulation_index[(prt.modifies - 1)][channel_i];
        if (!carrier) {
          status_require((sph_helper_calloc((duration * sizeof(sp_sample_t)), (&carrier))));
          memreg_add(carrier);
        };
        phs = phases[(channel_i + (out.channels * prt_i))];
        modulation = modulation_index[prt_i][channel_i];
        for (i = 0; (i < prt_duration); i = (1 + i)) {
          amp = (prt.amp)[channel_i][(prt_start + i)];
          carrier[(prt_offset + i)] = (carrier[(prt_offset + i)] + (amp * sp_sine_96(phs)));
          wvl = (prt.wvl)[channel_i][(prt_start + i)];
          modulated_wvl = (modulation ? (wvl + (wvl * modulation[(prt_offset + i)])) : wvl);
          phs = sp_cheap_phase_96_float(phs, (48000 / modulated_wvl));
        };
        phases[(channel_i + (out.channels * prt_i))] = phs;
        modulation_index[(prt.modifies - 1)][channel_i] = carrier;
      };
    } else {
      for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
        phs = phases[(channel_i + (out.channels * prt_i))];
        modulation = modulation_index[prt_i][channel_i];
        for (i = 0; (i < prt_duration); i = (1 + i)) {
          amp = (prt.amp)[channel_i][(prt_start + i)];
          wvl = (prt.wvl)[channel_i][(prt_start + i)];
          modulated_wvl = (modulation ? (wvl + (wvl * modulation[(prt_offset + i)])) : wvl);
          phs = sp_cheap_phase_96_float(phs, (48000 / modulated_wvl));
          (out.samples)[channel_i][(prt_offset + i)] = ((out.samples)[channel_i][(prt_offset + i)] + (amp * sp_sine_96(phs)));
        };
        phases[(channel_i + (out.channels * prt_i))] = phs;
      };
    };
  };
exit:
  memreg_free;
  return (status);
};
#include "../main/windowed-sinc.c"
#include "../main/io.c"
void sp_event_sort_swap(void* a, void* b) {
  sp_event_t c;
  c = *((sp_event_t*)(a));
  *((sp_event_t*)(b)) = *((sp_event_t*)(a));
  *((sp_event_t*)(b)) = c;
};
uint8_t sp_event_sort_less_p(void* a, void* b) { return ((((sp_event_t*)(a))->start < ((sp_event_t*)(b))->start)); };
void sp_seq_events_prepare(sp_events_t a) { quicksort(sp_event_sort_less_p, sp_event_sort_swap, (sizeof(sp_event_t)), (a.start), (i_array_length(a))); };
/** add offset to the all channel sample arrays in block. modifies the block object */
void sp_block_at_offset(sp_block_t* a, sp_count_t offset) {
  sp_count_t i;
  for (i = 0; (i < a->channels); i = (1 + i)) {
    (a->samples)[i] = (offset + (a->samples)[i]);
  };
};
/** event arrays must have been prepared/sorted with sp-seq-event-prepare for seq to work correctly */
void sp_seq(sp_count_t time, sp_count_t offset, sp_count_t size, sp_block_t out, sp_events_t events) {
  sp_count_t e_offset_right;
  sp_count_t e_offset;
  sp_event_t e;
  sp_count_t e_time;
  sp_count_t i;
  sp_count_t time_end;
  time_end = (time + size);
  for (i = 0; (i < i_array_length(events)); i = (1 + i)) {
    e = (events.start)[i];
    if (time_end <= e.start) {
      break;
    } else if (e.end <= time) {
      continue;
    } else {
      printf("from %lu to %lu time %lu\n", (e.start), (e.end), time);
      e_time = ((e.start < time) ? (time - e.start) : 0);
      e_offset = ((e.start > time) ? (e.start - time) : 0);
      e_offset_right = ((e.end > time_end) ? 0 : (time_end - e.end));
      if (offset + e_offset) {
        sp_block_at_offset((&out), (offset + e_offset));
      };
      (e.f)(e_time, (size - e_offset - e_offset_right), out, (&e));
    };
  };
};
void sp_synth_event_f(sp_count_t time, sp_count_t size, sp_block_t out, sp_event_t* event) {
  sp_synth_event_state_t* state;
  state = event->state;
  if (!size) {
    printf("size %lu at time %lu\n", size, time);
  };
  sp_synth(out, time, size, (state->config_len), (state->config), (state->state));
};
/** memory for event.state is allocated and owned by the caller.
  config is copied into event.state */
status_t sp_synth_event(sp_count_t start, sp_count_t end, sp_count_t channel_count, sp_count_t config_len, sp_synth_partial_t* config, sp_event_t* out_event) {
  status_declare;
  sp_event_t e;
  sp_synth_event_state_t* state;
  status_require((sph_helper_malloc((channel_count * sizeof(sp_synth_event_state_t)), (&state))));
  status_require((sp_synth_state_new(channel_count, config_len, config, (&(state->state)))));
  memcpy((state->config), config, (config_len * sizeof(sp_synth_partial_t)));
  state->config_len = config_len;
  e.start = start;
  e.end = end;
  e.f = sp_synth_event_f;
  e.state = state;
  *out_event = e;
exit:
  return (status);
};
uint32_t sp_plot_temp_file_index = 0;
#define sp_plot_temp_path "/tmp/sp-plot"
#define sp_plot_temp_file_index_maxlength 10
#define sp_plot_command_pattern_lines "gnuplot --persist -e 'set key off; set size ratio 0.5; plot \"%s\" with lines lc rgb \"blue\"'"
#define sp_plot_command_pattern_steps "gnuplot --persist -e 'set key off; set size ratio 0.5; plot \"%s\" with histeps lc rgb \"blue\"'"
void sp_plot_samples_to_file(sp_sample_t* a, sp_count_t a_size, uint8_t* path) {
  FILE* file;
  sp_count_t i;
  file = fopen(path, "w");
  for (i = 0; (i < a_size); i = (1 + i)) {
    fprintf(file, ("%.3f\n"), (a[i]));
  };
  fclose(file);
};
void sp_plot_samples_file(uint8_t* path, uint8_t use_steps) {
  uint8_t* command;
  uint8_t* command_pattern;
  size_t command_size;
  command_pattern = (use_steps ? sp_plot_command_pattern_steps : sp_plot_command_pattern_lines);
  command_size = (strlen(path) + strlen(command_pattern));
  command = malloc(command_size);
  if (!command) {
    return;
  };
  snprintf(command, command_size, command_pattern, path);
  system(command);
  free(command);
};
void sp_plot_samples(sp_sample_t* a, sp_count_t a_size) {
  uint8_t path_size = (1 + sp_plot_temp_file_index_maxlength + strlen(sp_plot_temp_path));
  uint8_t* path = calloc(path_size, 1);
  if (!path) {
    return;
  };
  snprintf(path, path_size, "%s-%lu", sp_plot_temp_path, sp_plot_temp_file_index);
  sp_plot_temp_file_index = (1 + sp_plot_temp_file_index);
  sp_plot_samples_to_file(a, a_size, path);
  sp_plot_samples_file(path, 0);
  free(path);
};
/** take the fft for given samples, convert complex values to magnitudes and write plot data to file */
void sp_plot_spectrum_to_file(sp_sample_t* a, sp_count_t a_size, uint8_t* path) {
  FILE* file;
  sp_count_t i;
  double* imag;
  double* real;
  imag = calloc(a_size, (sizeof(sp_sample_t)));
  if (!imag) {
    return;
  };
  real = malloc((sizeof(sp_sample_t) * a_size));
  if (!real) {
    return;
  };
  memcpy(real, a, (sizeof(sp_sample_t) * a_size));
  if (sp_fft(a_size, real, imag)) {
    return;
  };
  file = fopen(path, "w");
  for (i = 0; (i < a_size); i = (1 + i)) {
    fprintf(file, ("%.3f\n"), (2 * (sqrt(((real[i] * real[i]) + (imag[i] * imag[i]))) / a_size)));
  };
  fclose(file);
  free(imag);
  free(real);
};
void sp_plot_spectrum_file(uint8_t* path) { sp_plot_samples_file(path, 1); };
void sp_plot_spectrum(sp_sample_t* a, sp_count_t a_size) {
  uint8_t path_size = (1 + sp_plot_temp_file_index_maxlength + strlen(sp_plot_temp_path));
  uint8_t* path = calloc(path_size, 1);
  if (!path) {
    return;
  };
  snprintf(path, path_size, "%s-%lu", sp_plot_temp_path, sp_plot_temp_file_index);
  sp_plot_temp_file_index = (1 + sp_plot_temp_file_index);
  sp_plot_spectrum_to_file(a, a_size, path);
  sp_plot_spectrum_file(path);
  free(path);
};