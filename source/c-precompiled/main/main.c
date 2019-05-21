/* this file contains basics and includes dependencies */
#include <stdio.h>
#include <fcntl.h>
#include <sndfile.h>
#include <foreign/nayuki-fft/fft.c>
#include "../main/sph-sp.h"
#include "../foreign/sph/spline-path.c"
#include "../foreign/sph/float.c"
#include "../foreign/sph/helper.c"
#include "../foreign/sph/memreg.c"
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
/** return a newly allocated array for channels with data arrays for each channel */
status_t sp_block_new(sp_channel_count_t channels, sp_count_t size, sp_block_t* out) {
  status_declare;
  memreg_init(channels);
  sp_sample_t* channel;
  sp_count_t i;
  for (i = 0; (i < channels); i = (1 + i)) {
    status_require((sph_helper_calloc((size * sizeof(sp_sample_t)), (&channel))));
    memreg_add(channel);
    (out->samples)[i] = channel;
  };
  out->size = size;
  out->channels = channels;
exit:
  if (status_is_failure) {
    memreg_free;
  };
  return (status);
};
void sp_block_free(sp_block_t a) {
  sp_count_t i;
  for (i = 0; (i < a.channels); i = (1 + i)) {
    free(((a.samples)[i]));
  };
};
/** add offset to the all channel sample arrays in block */
sp_block_t sp_block_with_offset(sp_block_t a, sp_count_t offset) {
  sp_count_t i;
  for (i = 0; (i < a.channels); i = (1 + i)) {
    (a.samples)[i] = (offset + (a.samples)[i]);
  };
  return (a);
};
status_t sp_samples_new(sp_count_t size, sp_sample_t** out) { return ((sph_helper_calloc((size * sizeof(sp_sample_t)), out))); };
status_t sp_counts_new(sp_count_t size, sp_count_t** out) { return ((sph_helper_calloc((size * sizeof(sp_count_t)), out))); };
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
/** guarantees that all dyadic rationals of the form (k / 2**−53) will be equally likely. this conversion prefers the high bits of x.
    from http://xoshiro.di.unimi.it/ */
#define f64_from_uint64(a) ((a >> 11) * (1.0 / (UINT64_C(1) << 53)))
#define rotl(x, k) ((x << k) | (x >> (64 - k)))
/** use the given uint64 as a seed and set state with splitmix64 results.
  the same seed will lead to the same series of random numbers from sp-random */
sp_random_state_t sp_random_state_new(uint64_t seed) {
  uint8_t i;
  uint64_t z;
  sp_random_state_t result;
  for (i = 0; (i < 4); i = (1 + i)) {
    seed = (seed + UINT64_C(11400714819323198485));
    z = seed;
    z = ((z ^ (z >> 30)) * UINT64_C(13787848793156543929));
    z = ((z ^ (z >> 27)) * UINT64_C(10723151780598845931));
    (result.data)[i] = (z ^ (z >> 31));
  };
  return (result);
};
#define define_sp_random(name, type, transfer) \
  /** return uniformly distributed random real numbers in the range -1 to 1. \
      implements xoshiro256plus from http://xoshiro.di.unimi.it/ \
      referenced by https://nullprogram.com/blog/2017/09/21/ */ \
  sp_random_state_t name(sp_random_state_t state, sp_count_t size, type* out) { \
    uint64_t result_plus; \
    sp_count_t i; \
    uint64_t t; \
    for (i = 0; (i < size); i = (1 + i)) { \
      result_plus = ((state.data)[0] + (state.data)[3]); \
      t = ((state.data)[1] << 17); \
      (state.data)[2] = ((state.data)[2] ^ (state.data)[0]); \
      (state.data)[3] = ((state.data)[3] ^ (state.data)[1]); \
      (state.data)[1] = ((state.data)[1] ^ (state.data)[2]); \
      (state.data)[0] = ((state.data)[0] ^ (state.data)[3]); \
      (state.data)[2] = ((state.data)[2] ^ t); \
      (state.data)[3] = rotl(((state.data)[3]), 45); \
      out[i] = transfer; \
    }; \
    return (state); \
  }
define_sp_random(sp_random, f64, (f64_from_uint64(result_plus)));
define_sp_random(sp_random_samples, sp_sample_t, ((2 * f64_from_uint64(result_plus)) - 1.0));
#define i_array_get_or_null(a) (i_array_in_range(a) ? i_array_get(a) : 0)
#define i_array_forward_if_possible(a) (i_array_in_range(a) ? i_array_forward(a) : 0)
#define sp_samples_zero(a, size) memset(a, 0, (size * sizeof(sp_sample_t)))
/** get the maximum value in samples array, disregarding sign */
sp_sample_t sp_samples_absolute_max(sp_sample_t* in, sp_count_t in_size) {
  sp_sample_t result;
  sp_sample_t a;
  sp_count_t i;
  for (i = 0, result = 0; (i < in_size); i = (1 + i)) {
    a = fabs((in[i]));
    if (a > result) {
      result = a;
    };
  };
  return (result);
};
/** adjust amplitude of out to match the one of in */
void sp_set_unity_gain(sp_sample_t* in, sp_count_t in_size, sp_sample_t* out) {
  sp_count_t i;
  sp_sample_t in_max;
  sp_sample_t out_max;
  sp_sample_t difference;
  sp_sample_t correction;
  in_max = sp_samples_absolute_max(in, in_size);
  out_max = sp_samples_absolute_max(out, in_size);
  if ((0 == in_max) || (0 == out_max)) {
    return;
  };
  difference = (out_max / in_max);
  correction = (1 + ((1 - difference) / difference));
  for (i = 0; (i < in_size); i = (1 + i)) {
    out[i] = (correction * out[i]);
  };
};
#include "../main/io.c"
#include "../main/plot.c"
#include "../main/filter.c"
#include "../main/synthesiser.c"
#include "../main/sequencer.c"
/** fills the sine wave lookup table */
status_t sp_initialise(uint16_t cpu_count) {
  status_declare;
  if (cpu_count) {
    status.id = future_init(cpu_count);
  };
  if (status.id) {
    return (status);
  };
  sp_default_random_state = sp_random_state_new(1557083953);
  return ((sp_sine_table_new((&sp_sine_96_table), 96000)));
};