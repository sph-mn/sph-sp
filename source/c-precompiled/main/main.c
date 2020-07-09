/* this file contains basics and includes dependencies */
#define M_PI 3.141592653589793
#include <stdio.h>
#include <fcntl.h>
#include <sndfile.h>
#include <foreign/nayuki-fft/fft.c>
#include "../main/sph-sp.h"
#include <sph/spline-path.c>
#include <sph/helper.c>
#include <sph/memreg.c>
#include <sph/quicksort.c>
#include <sph/queue.c>
#include <sph/thread-pool.c>
#include <sph/futures.c>
#include <sph/set.c>
#define sp_status_declare status_declare_group(sp_s_group_sp)
#define sp_libc_s_id(id) \
  if (id < 0) { \
    status_set_goto(sp_s_group_libc, id); \
  }
#define sp_libc_s(expression) \
  status.id = expression; \
  if (status.id < 0) { \
    status.group = sp_s_group_libc; \
    goto exit; \
  } else { \
    status.id = 0; \
  }
/** define a deinterleave, interleave or similar routine.
     a: source
     b: target */
#define define_sp_interleave(name, type, body) \
  void name(type** a, type* b, sp_time_t a_size, sp_channels_t channel_count) { \
    sp_time_t b_size; \
    sp_channels_t channel; \
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
#define sp_memory_error status_set_goto(sp_s_group_sp, sp_s_id_memory)
define_sp_interleave(sp_interleave, sp_sample_t, (b[b_size] = (a[channel])[a_size]))
  define_sp_interleave(sp_deinterleave, sp_sample_t, ((a[channel])[a_size] = b[b_size]))
  /** get a string description for a status id in a status-t */
  uint8_t* sp_status_description(status_t a) {
  uint8_t* b;
  if (!strcmp(sp_s_group_sp, (a.group))) {
    if (sp_s_id_eof == a.id) {
      b = "end of file";
    } else if (sp_s_id_input_type == a.id) {
      b = "input argument is of wrong type";
    } else if (sp_s_id_not_implemented == a.id) {
      b = "not implemented";
    } else if (sp_s_id_memory == a.id) {
      b = "memory allocation error";
    } else if (sp_s_id_file_incompatible == a.id) {
      b = "file channel count or sample rate is different from what was requested";
    } else if (sp_s_id_file_incomplete == a.id) {
      b = "incomplete write";
    } else {
      b = "";
    };
  } else if (!strcmp(sp_s_group_sndfile, (a.group))) {
    b = ((uint8_t*)(sf_error_number((a.id))));
  } else if (!strcmp(sp_s_group_sph, (a.group))) {
    b = sph_helper_status_description(a);
  } else {
    b = "";
  };
  return (b);
}
/** get a one word identifier for a status id in a status-t */
uint8_t* sp_status_name(status_t a) {
  uint8_t* b;
  if (0 == strcmp(sp_s_group_sp, (a.group))) {
    if (sp_s_id_input_type == a.id) {
      b = "input-type";
    } else if (sp_s_id_not_implemented == a.id) {
      b = "not-implemented";
    } else if (sp_s_id_memory == a.id) {
      b = "memory";
    } else {
      b = "unknown";
    };
  } else if (0 == strcmp(sp_s_group_sndfile, (a.group))) {
    b = "sndfile";
  } else {
    b = "unknown";
  };
  return (b);
}
/** return a newly allocated array for channels with data arrays for each channel */
status_t sp_block_new(sp_channels_t channels, sp_time_t size, sp_block_t* out) {
  status_declare;
  memreg_init(channels);
  sp_sample_t* channel;
  sp_time_t i;
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
  status_return;
}
void sp_block_free(sp_block_t a) {
  sp_time_t i;
  if (a.size) {
    for (i = 0; (i < a.channels); i = (1 + i)) {
      free(((a.samples)[i]));
    };
  };
}
/** return a new block with offset added to all channel sample arrays */
sp_block_t sp_block_with_offset(sp_block_t a, sp_time_t offset) {
  sp_time_t i;
  for (i = 0; (i < a.channels); i = (1 + i)) {
    (a.samples)[i] = (offset + (a.samples)[i]);
  };
  return (a);
}
/** lower precision version of sin() that should be faster */
sp_sample_t sp_sin_lq(sp_float_t a) {
  sp_sample_t b;
  sp_sample_t c;
  b = (4 / M_PI);
  c = (-4 / (M_PI * M_PI));
  return ((((b * a) + (c * a * abs(a)))));
}
sp_time_t sp_phase(sp_time_t current, sp_time_t change, sp_time_t cycle) {
  sp_time_t a = (current + change);
  return (((a < cycle) ? a : (a % cycle)));
}
/** accumulate an integer phase with change given as a float value.
   change must be a positive value and is rounded to the next larger integer */
sp_time_t sp_phase_float(sp_time_t current, sp_sample_t change, sp_time_t cycle) {
  sp_time_t a = (current + sp_cheap_ceiling_positive(change));
  return (((a < cycle) ? a : (a % cycle)));
}
/** * sums into out
   * state.frq (frequency): array with hertz values
   * state.wvf (waveform): array with waveform samples
   * state.wvf-size: size of state.wvf
   * state.phs (phase): value per channel
   * state.amp (amplitude): array per channel */
void sp_wave(sp_time_t start, sp_time_t duration, sp_wave_state_t* state, sp_block_t out) {
  /* temp debug */
  sp_sample_t amp;
  sp_time_t channel_i;
  sp_time_t phs;
  sp_time_t i;
  for (channel_i = 0; (channel_i < out.channels); channel_i = (1 + channel_i)) {
    phs = (state->phs)[channel_i];
    for (i = 0; (i < duration); i = (1 + i)) {
      (out.samples)[channel_i][i] += ((state->amp)[channel_i][(start + i)] * (state->wvf)[phs]);
      phs += (state->frq)[(start + i)];
      if (phs >= state->wvf_size) {
        phs = (phs % state->wvf_size);
      };
    };
    (state->phs)[channel_i] = phs;
  };
}
#define sp_wave_state_set_channel(a, channel, amp_array, phs_value) \
  (a.amp)[channel] = amp_array; \
  (a.phs)[channel] = phs_value
/** setup a single channel wave config */
sp_wave_state_t sp_wave_state_1(sp_sample_t* wvf, sp_time_t wvf_size, sp_time_t size, sp_time_t* frq, sp_sample_t* amp, sp_time_t phs) {
  sp_wave_state_t a;
  a.channels = 1;
  a.size = size;
  a.frq = frq;
  a.wvf = wvf;
  a.wvf_size = wvf_size;
  sp_wave_state_set_channel(a, 0, amp, phs);
  return (a);
}
sp_wave_state_t sp_wave_state_2(sp_sample_t* wvf, sp_time_t wvf_size, sp_time_t size, sp_time_t* frq, sp_sample_t* amp1, sp_sample_t* amp2, sp_time_t phs1, sp_time_t phs2) {
  sp_wave_state_t a;
  a.channels = 2;
  a.size = size;
  a.frq = frq;
  a.wvf = wvf;
  a.wvf_size = wvf_size;
  sp_wave_state_set_channel(a, 0, amp1, phs1);
  sp_wave_state_set_channel(a, 1, amp2, phs2);
  return (a);
}
/** return a sample for a triangular wave with center offsets a left and b right.
   creates sawtooth waves if either a or b is 0 */
sp_sample_t sp_triangle(sp_time_t t, sp_time_t a, sp_time_t b) {
  sp_time_t remainder;
  remainder = (t % (a + b));
  return (((remainder < a) ? (remainder * (1 / ((sp_sample_t)(a)))) : ((((sp_sample_t)(b)) - (remainder - ((sp_sample_t)(a)))) * (1 / ((sp_sample_t)(b))))));
}
sp_sample_t sp_square(sp_time_t t, sp_time_t size) { return (((((2 * t) % (2 * size)) < size) ? -1 : 1)); }
/** writes one full period of a sine wave into out. can be used to create lookup tables */
void sp_sine_period(sp_time_t size, sp_sample_t* out) {
  sp_time_t i;
  for (i = 0; (i < size); i = (1 + i)) {
    out[i] = sin((i * (M_PI / (size / 2))));
  };
}
/** the normalised sinc function */
sp_float_t sp_sinc(sp_float_t a) { return (((0 == a) ? 1 : (sin((M_PI * a)) / (M_PI * a)))); }
/** all arrays should be input-len and are managed by the caller */
int sp_fft(sp_time_t input_len, double* input_or_output_real, double* input_or_output_imag) { return ((!Fft_transform(input_or_output_real, input_or_output_imag, input_len))); }
/** [[real, imaginary], ...]:complex-numbers -> real-numbers
   input-length > 0
   output-length = input-length
   output is allocated and owned by the caller */
int sp_ffti(sp_time_t input_len, double* input_or_output_real, double* input_or_output_imag) { return ((!(1 == Fft_inverseTransform(input_or_output_real, input_or_output_imag, input_len)))); }
/** modify an impulse response kernel for spectral inversion.
   a-len must be odd and "a" must have left-right symmetry.
   flips the frequency response top to bottom */
void sp_spectral_inversion_ir(sp_sample_t* a, sp_time_t a_len) {
  sp_time_t center;
  sp_time_t i;
  for (i = 0; (i < a_len); i = (1 + i)) {
    a[i] = (-1 * a[i]);
  };
  center = ((a_len - 1) / 2);
  a[center] = (1 + a[center]);
}
/** inverts the sign for samples at odd indexes.
   a-len must be odd and "a" must have left-right symmetry.
   flips the frequency response left to right */
void sp_spectral_reversal_ir(sp_sample_t* a, sp_time_t a_len) {
  while ((a_len > 1)) {
    a_len = (a_len - 2);
    a[a_len] = (-1 * a[a_len]);
  };
}
/** discrete linear convolution.
   result-samples must be all zeros, its length must be at least a-len + b-len - 1.
   result-samples is owned and allocated by the caller */
void sp_convolve_one(sp_sample_t* a, sp_time_t a_len, sp_sample_t* b, sp_time_t b_len, sp_sample_t* result_samples) {
  sp_time_t a_index;
  sp_time_t b_index;
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
}
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
void sp_convolve(sp_sample_t* a, sp_time_t a_len, sp_sample_t* b, sp_time_t b_len, sp_time_t carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples) {
  sp_time_t size;
  sp_time_t a_index;
  sp_time_t b_index;
  sp_time_t c_index;
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
}
sp_time_t sp_time_expt(sp_time_t base, sp_time_t exp) {
  sp_time_t a = 1;
  for (;;) {
    if (exp & 1) {
      a *= base;
    };
    exp = (exp >> 1);
    if (!exp) {
      break;
    };
    base *= base;
  };
  return (a);
}
sp_time_t sp_time_factorial(sp_time_t a) {
  sp_time_t result;
  result = 1;
  while ((a > 0)) {
    result = (result * a);
    a = (a - 1);
  };
  return (result);
}
/** calculate the maximum possible number of overlapping sequences */
sp_time_t sp_sequence_max(sp_time_t size, sp_time_t min_size) {
  sp_time_t i;
  if ((0 == size) || (min_size > size)) {
    return (0);
  } else if (min_size == size) {
    return (1);
  } else {
    sp_time_t result = 0;
    for (i = min_size; (i <= size); i += 1) {
      result += ((1 + size) - i);
    };
    return (result);
  };
}
/** calculate the maximum number of possible distinct selections from a set with length "set-size" */
sp_time_t sp_set_sequence_max(sp_time_t set_size, sp_time_t selection_size) { return (((0 == set_size) ? 0 : sp_time_expt(set_size, selection_size))); }
sp_time_t sp_permutations_max(sp_time_t set_size, sp_time_t selection_size) { return ((sp_time_factorial(set_size) / (set_size - selection_size))); }
sp_time_t sp_compositions_max(sp_time_t sum) { return ((sp_time_expt(2, (sum - 1)))); }
#include "../main/arrays.c"
void sp_block_zero(sp_block_t a) {
  sp_channels_t i;
  for (i = 0; (i < a.channels); i += 1) {
    sp_samples_zero(((a.samples)[i]), (a.size));
  };
}
/** out memory is allocated */
status_t sp_path_samples(sp_path_segment_t* segments, sp_time_t segments_size, sp_time_t size, sp_sample_t** out) {
  status_declare;
  sp_sample_t* result;
  status_require((sp_samples_new(size, (&result))));
  if (spline_path_new_get(segments_size, segments, 0, size, result)) {
    free(result);
    sp_memory_error;
  };
  *out = result;
exit:
  status_return;
}
/** create a new path from the given segments config.
   memory is allocated and ownership transferred to the caller */
status_t sp_path_times(sp_path_segment_t* segments, sp_time_t segments_size, sp_time_t size, sp_time_t** out) {
  status_declare;
  sp_time_t* result;
  sp_sample_t* temp;
  status_require((sp_path_samples(segments, segments_size, size, (&temp))));
  status_require((sp_times_new(size, (&result))));
  sp_samples_to_times(temp, size, result);
  *out = result;
exit:
  status_return;
}
status_t sp_path_times_1(sp_time_t** out, sp_time_t size, sp_path_segment_t s1) {
  sp_path_segment_t segments[1];
  segments[0] = s1;
  return ((sp_path_times(segments, 1, size, out)));
}
status_t sp_path_times_2(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2) {
  sp_path_segment_t segments[2];
  segments[0] = s1;
  segments[1] = s2;
  return ((sp_path_times(segments, 2, size, out)));
}
status_t sp_path_times_3(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3) {
  sp_path_segment_t segments[3];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  return ((sp_path_times(segments, 3, size, out)));
}
status_t sp_path_times_4(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4) {
  sp_path_segment_t segments[4];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  segments[3] = s4;
  return ((sp_path_times(segments, 4, size, out)));
}
status_t sp_path_samples_1(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1) {
  sp_path_segment_t segments[1];
  segments[0] = s1;
  return ((sp_path_samples(segments, 1, size, out)));
}
status_t sp_path_samples_2(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2) {
  sp_path_segment_t segments[2];
  segments[0] = s1;
  segments[1] = s2;
  return ((sp_path_samples(segments, 2, size, out)));
}
status_t sp_path_samples_3(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3) {
  sp_path_segment_t segments[3];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  return ((sp_path_samples(segments, 3, size, out)));
}
status_t sp_path_samples_4(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4) {
  sp_path_segment_t segments[4];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  segments[3] = s4;
  return ((sp_path_samples(segments, 4, size, out)));
}
#include "../main/io.c"
#include "../main/plot.c"
#include "../main/filter.c"
#include "../main/sequencer.c"
#include "../main/statistics.c"
/** render a single event to file. event can be a group */
status_t sp_render_file(sp_event_t event, sp_time_t start, sp_time_t end, sp_render_config_t config, uint8_t* path) {
  status_declare;
  sp_time_t block_end;
  sp_time_t remainder;
  sp_time_t i;
  sp_time_t written;
  sp_block_declare(block);
  sp_file_declare(file);
  status_require((sp_block_new((config.channels), (config.block_size), (&block))));
  status_require((sp_file_open(path, sp_file_mode_write, (config.channels), (config.rate), (&file))));
  remainder = ((end - start) % config.block_size);
  block_end = (config.block_size * ((end - start) / config.block_size));
  for (i = 0; (i < block_end); i += config.block_size) {
    sp_seq(i, (i + config.block_size), block, (&event), 1);
    /* (status-require (sp-file-write &file block.samples config.block-size &written)) */
    sp_block_zero(block);
  };
  if (remainder) {
    sp_seq(i, (i + remainder), block, (&event), 1);
    status_require((sp_file_write((&file), (block.samples), remainder, (&written))));
  };
exit:
  sp_block_free(block);
  sp_file_close(file);
  status_return;
}
/** render a single event to block arrays. event can be a group */
status_t sp_render_block(sp_event_t event, sp_time_t start, sp_time_t end, sp_render_config_t config, sp_block_t* out) {
  status_declare;
  sp_block_t block;
  sp_time_t i;
  status_require((sp_block_new((config.channels), (end - start), (&block))));
  sp_seq(start, end, block, (&event), 1);
  *out = block;
exit:
  status_return;
}
/** fills the sine wave lookup table */
status_t sp_initialise(uint16_t cpu_count, sp_time_t sine_table_size) {
  status_declare;
  if (cpu_count) {
    status.id = future_init(cpu_count);
    if (status.id) {
      status_return;
    };
  };
  sp_cpu_count = cpu_count;
  sp_sine_table_size = sine_table_size;
  sp_default_random_state = sp_random_state_new(1557083953);
  status_require((sp_samples_new(sine_table_size, (&sp_sine_table))));
  sp_sine_period(sine_table_size, sp_sine_table);
exit:
  status_return;
}
