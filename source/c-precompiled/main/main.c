#include <stdio.h>
#include <fcntl.h>
#include <sndfile.h>
#include "../main/sph-sp.h"
#include "../foreign/sph/float.c"
#include "../foreign/sph/helper.c"
#include "../foreign/sph/memreg.c"
#include <foreign/nayuki-fft/fft.c>
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
  void name(type** a, type* b, sp_sample_count_t a_size, sp_channel_count_t channel_count) { \
    sp_sample_count_t b_size; \
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
void debug_display_sample_array(sp_sample_t* a, sp_sample_count_t len) {
  sp_sample_count_t i;
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
void sp_channel_data_free(sp_sample_t** a, sp_channel_count_t channel_count) {
  while (channel_count) {
    channel_count = (channel_count - 1);
    free((a[channel_count]));
  };
  free(a);
};
/** return a newly allocated array for channels with data arrays for each channel.
  returns zero if memory could not be allocated */
status_t sp_alloc_channel_array(sp_channel_count_t channel_count, sp_sample_count_t sample_count, sp_sample_t*** result_array) {
  status_declare;
  memreg_init((channel_count + 1));
  sp_sample_t* channel;
  sp_sample_t** result;
  status_require((sph_helper_malloc((channel_count * sizeof(sp_sample_t*)), (&result))));
  memreg_add(result);
  while (channel_count) {
    channel_count = (channel_count - 1);
    status_require((sph_helper_calloc((sample_count * sizeof(sp_sample_t)), (&channel))));
    memreg_add(channel);
    result[channel_count] = channel;
  };
  *result_array = result;
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
status_t sp_fft(sp_sample_count_t input_len, sp_sample_t* input_or_output_real, sp_sample_t* input_or_output_imag) {
  sp_status_declare;
  status_id_require((!Fft_transform(input_or_output_real, input_or_output_imag, input_len)));
exit:
  return (status);
};
/** [[real, imaginary], ...]:complex-numbers -> real-numbers
  input-length > 0
  output-length = input-length
  output is allocated and owned by the caller */
status_t sp_ffti(sp_sample_count_t input_len, sp_sample_t* input_or_output_real, sp_sample_t* input_or_output_imag) {
  sp_status_declare;
  status_id_require((!(1 == Fft_inverseTransform(input_or_output_real, input_or_output_imag, input_len))));
exit:
  return (status);
};
/** apply a centered moving average filter to source at index start to end inclusively and write to result.
  removes higher frequencies with little distortion in the time domain.
  result-samples is owned and allocated by the caller.
   * only the result portion corresponding to the subvector from start to end is written to result
   * prev and next are unprocessed segments and can be null pointers,
     for example at the beginning and end of a stream
   * since the result value for a sample is calculated using samples left and right of it,
     a previous and following part of a stream is eventually needed to reference values
     outside the source segment to create an accurate continuous result.
     zero is used for unavailable values outside the source segment
   * available values outside the start/end range are still considered when needed to calculate averages
   * rounding errors are kept low by using modified kahan neumaier summation and not using a
     recursive implementation. both properties which make it much slower than many other implementations */
status_t sp_moving_average(sp_sample_t* source, sp_sample_count_t source_len, sp_sample_t* prev, sp_sample_count_t prev_len, sp_sample_t* next, sp_sample_count_t next_len, sp_sample_count_t radius, sp_sample_count_t start, sp_sample_count_t end, sp_sample_t* result_samples) {
  status_declare;
  memreg_init(1);
  sp_sample_count_t left;
  sp_sample_count_t right;
  sp_sample_count_t width;
  sp_sample_t* window;
  sp_sample_count_t window_index;
  if (!source_len) {
    goto exit;
  };
  width = (1 + (2 * radius));
  window = 0;
  /* check if all required samples are in source array */
  if (!((start >= radius) && ((start + radius + 1) <= source_len))) {
    status_require((sph_helper_malloc((width * sizeof(sp_sample_t)), (&window))));
    memreg_add(window);
  };
  while ((start <= end)) {
    if ((start >= radius) && ((start + radius + 1) <= source_len)) {
      *result_samples = (sp_sample_sum(((source + start) - radius), width) / width);
    } else {
      window_index = 0;
      /* get samples from previous segment */
      if (start < radius) {
        right = (radius - start);
        if (prev) {
          left = ((right > prev_len) ? 0 : (prev_len - right));
          while ((left < prev_len)) {
            window[window_index] = prev[left];
            window_index = (1 + window_index);
            left = (1 + left);
          };
        };
        while ((window_index < right)) {
          window[window_index] = 0;
          window_index = (1 + window_index);
        };
        left = 0;
      } else {
        left = (start - radius);
      };
      /* get samples from source segment */
      right = (start + radius);
      if (right >= source_len) {
        right = (source_len - 1);
      };
      while ((left <= right)) {
        window[window_index] = source[left];
        window_index = (1 + window_index);
        left = (1 + left);
      };
      /* get samples from next segment */
      right = (start + radius);
      if ((right >= source_len) && next) {
        left = 0;
        right = (right - source_len);
        if (right >= next_len) {
          right = (next_len - 1);
        };
        while ((left <= right)) {
          window[window_index] = next[left];
          window_index = (1 + window_index);
          left = (1 + left);
        };
      };
      /* fill unset values in window with zero */
      while ((window_index < width)) {
        window[window_index] = 0;
        window_index = (1 + window_index);
      };
      /* set current value to the window average */
      *result_samples = (sp_sample_sum(window, width) / width);
    };
    result_samples = (1 + result_samples);
    start = (1 + start);
  };
exit:
  memreg_free;
  return (status);
};
/** modify an impulse response kernel for spectral inversion.
   a-len must be odd and "a" must have left-right symmetry.
  flips the frequency response top to bottom */
void sp_spectral_inversion_ir(sp_sample_t* a, sp_sample_count_t a_len) {
  sp_sample_count_t center;
  sp_sample_count_t i;
  for (i = 0; (i < a_len); i = (1 + i)) {
    a[i] = (-1 * a[i]);
  };
  center = ((a_len - 1) / 2);
  a[center] = (1 + a[center]);
};
/** inverts the sign for samples at odd indexes.
  a-len must be odd and "a" must have left-right symmetry.
  flips the frequency response left to right */
void sp_spectral_reversal_ir(sp_sample_t* a, sp_sample_count_t a_len) {
  while ((a_len > 1)) {
    a_len = (a_len - 2);
    a[a_len] = (-1 * a[a_len]);
  };
};
/** discrete linear convolution.
  result-samples must be all zeros, its length must be at least a-len + b-len - 1.
  result-samples is owned and allocated by the caller */
void sp_convolve_one(sp_sample_t* a, sp_sample_count_t a_len, sp_sample_t* b, sp_sample_count_t b_len, sp_sample_t* result_samples) {
  sp_sample_count_t a_index;
  sp_sample_count_t b_index;
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
  continuous processing does not work correctly if result-samples is smaller than b-len - 1, in this case
  result-carryover will contain values after index a-len - 1 that will not be carried over to the next call.
  carryover-len should be zero for the first call or its content should be zeros.
  carryover-len for subsequent calls should be b-len - 1 or if b-len changed b-len - 1  from the previous call.
  if b-len is one then there is no carryover.
  if a-len is smaller than b-len then, with the current implementation, additional performance costs ensue from shifting the carryover array each call */
void sp_convolve(sp_sample_t* a, sp_sample_count_t a_len, sp_sample_t* b, sp_sample_count_t b_len, sp_sample_count_t carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples) {
  sp_sample_count_t size;
  sp_sample_count_t a_index;
  sp_sample_count_t b_index;
  sp_sample_count_t c_index;
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
  sp_sample_count_t carryover_alloc_len;
  sp_sample_t* ir;
  sp_sample_count_t ir_len;
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
  * out-state: if zero then state will be allocated. owned by caller. the state can currently not be reused with varying ir-f-argument sizes.
  * out-samples: owned by the caller. length must be at least in-len and the number of output samples will be in-len */
status_t sp_convolution_filter(sp_sample_t* in, sp_sample_count_t in_len, sp_convolution_filter_ir_f_t ir_f, void* ir_f_arguments, uint8_t ir_f_arguments_len, sp_convolution_filter_state_t** out_state, sp_sample_t* out_samples) {
  status_declare;
  sp_sample_count_t carryover_len;
  carryover_len = (*out_state ? ((*out_state)->ir_len - 1) : 0);
  /* create/update the impulse response kernel */
  status_require((sp_convolution_filter_state_set(ir_f, ir_f_arguments, ir_f_arguments_len, out_state)));
  /* convolve */
  sp_convolve(in, in_len, ((*out_state)->ir), ((*out_state)->ir_len), carryover_len, ((*out_state)->carryover), out_samples);
exit:
  return (status);
};
#include "../main/windowed-sinc.c"
#include "../main/io.c"
