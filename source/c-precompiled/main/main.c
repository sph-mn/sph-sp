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
#define sp_status_declare status_declare_group(sp_s_group_sp)
#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)
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
    sph_random_define_x256p(sp_random_samples, sp_sample_t, ((2 * sph_random_f64_from_u64(a)) - 1.0));
sph_random_define_x256ss(sp_random_times, sp_time_t, a);
/** display a sample array in one line */
void display_samples(sp_sample_t* a, sp_time_t len) {
  sp_time_t i;
  printf(("%.17g"), (a[0]));
  for (i = 1; (i < len); i = (1 + i)) {
    printf((" %.17g"), (a[i]));
  };
  printf("\n");
}
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
/** get a single word identifier for a status id in a status-t */
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
  for (i = 0; (i < a.channels); i = (1 + i)) {
    free(((a.samples)[i]));
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
#include "../main/arrays.c"
/** adjust amplitude of out to match the one of in */
void sp_set_unity_gain(sp_sample_t* in, sp_time_t in_size, sp_sample_t* out) {
  sp_time_t i;
  sp_sample_t in_max;
  sp_sample_t out_max;
  sp_sample_t difference;
  sp_sample_t correction;
  in_max = sp_sample_array_absolute_max(in, in_size);
  out_max = sp_sample_array_absolute_max(out, in_size);
  if ((0 == in_max) || (0 == out_max)) {
    return;
  };
  difference = (out_max / in_max);
  correction = (1 + ((1 - difference) / difference));
  for (i = 0; (i < in_size); i = (1 + i)) {
    out[i] = (correction * out[i]);
  };
}
void sp_block_zero(sp_block_t a) {
  sp_channels_t i;
  for (i = 0; (i < a.channels); i += 1) {
    sp_sample_array_zero(((a.samples)[i]), (a.size));
  };
}
status_t sp_path_samples(sp_path_segments_t segments, sp_time_t size, sp_samples_t* out) {
  status_declare;
  sp_samples_t result;
  array3_set_null(result);
  status_i_require((sp_samples_new(size, (&result))));
  if (spline_path_new_get((segments.size), (segments.data), 0, size, (result.data))) {
    array4_free(result);
    sp_memory_error;
  };
  *out = result;
exit:
  status_return;
}
/** create a new path from the given segments config.
  memory is allocated and ownership transferred to the caller */
status_t sp_path_times(sp_path_segments_t segments, sp_time_t size, sp_times_t* out) {
  status_declare;
  sp_times_t result;
  sp_samples_t temp;
  array3_set_null(temp);
  status_require((sp_path_samples(segments, size, (&temp))));
  status_i_require((sp_times_new(size, (&result))));
  sp_samples_to_times(temp, result);
  *out = result;
exit:
  status_return;
}
status_t sp_path_times_2(sp_times_t* out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2) {
  sp_path_segments_t segments;
  sp_path_segment_t segments_data[2];
  segments.size = 2;
  segments.data = segments_data;
  segments_data[0] = s1;
  segments_data[1] = s2;
  return ((sp_path_times(segments, size, out)));
}
status_t sp_path_times_3(sp_times_t* out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3) {
  sp_path_segments_t segments;
  sp_path_segment_t segments_data[3];
  segments.size = 3;
  segments.data = segments_data;
  segments_data[0] = s1;
  segments_data[1] = s2;
  segments_data[2] = s3;
  return ((sp_path_times(segments, size, out)));
}
status_t sp_path_times_4(sp_times_t* out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4) {
  sp_path_segments_t segments;
  sp_path_segment_t segments_data[4];
  segments.size = 4;
  segments.data = segments_data;
  segments_data[0] = s1;
  segments_data[1] = s2;
  segments_data[2] = s3;
  segments_data[3] = s4;
  return ((sp_path_times(segments, size, out)));
}
status_t sp_path_samples_2(sp_samples_t* out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2) {
  sp_path_segments_t segments;
  sp_path_segment_t segments_data[2];
  segments.size = 2;
  segments.data = segments_data;
  segments_data[0] = s1;
  segments_data[1] = s2;
  return ((sp_path_samples(segments, size, out)));
}
status_t sp_path_samples_3(sp_samples_t* out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3) {
  sp_path_segments_t segments;
  sp_path_segment_t segments_data[3];
  segments.size = 3;
  segments.data = segments_data;
  segments_data[0] = s1;
  segments_data[1] = s2;
  segments_data[2] = s3;
  return ((sp_path_samples(segments, size, out)));
}
status_t sp_path_samples_4(sp_samples_t* out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4) {
  sp_path_segments_t segments;
  sp_path_segment_t segments_data[4];
  segments.size = 4;
  segments.data = segments_data;
  segments_data[0] = s1;
  segments_data[1] = s2;
  segments_data[2] = s3;
  segments_data[3] = s4;
  return ((sp_path_samples(segments, size, out)));
}
#include "../main/io.c"
#include "../main/plot.c"
#include "../main/filter.c"
#include "../main/synthesiser.c"
#include "../main/sequencer.c"
/** render a single event to file. event can be a group */
status_t sp_render_file(sp_event_t event, sp_time_t start, sp_time_t duration, sp_render_config_t config, uint8_t* path) {
  status_declare;
  sp_block_t block;
  sp_time_t end;
  uint8_t file_is_open;
  sp_file_t file;
  sp_time_t i;
  sp_time_t written;
  file_is_open = 0;
  status_require((sp_block_new((config.channels), (config.block_size), (&block))));
  status_require((sp_file_open(path, sp_file_mode_write, (config.channels), (config.rate), (&file))));
  file_is_open = 1;
  end = (duration / config.block_size);
  end = ((1 > end) ? config.block_size : (end * config.block_size));
  for (i = 0; (i < end); i += config.block_size) {
    sp_seq(i, (i + config.block_size), block, (&event), 1);
    status_require((sp_file_write((&file), (block.samples), (config.block_size), (&written))));
    sp_block_zero(block);
  };
exit:
  if (file_is_open) {
    sp_file_close((&file));
  };
  status_return;
}
/** fills the sine wave lookup table */
status_t sp_initialise(uint16_t cpu_count) {
  status_declare;
  if (cpu_count) {
    status.id = future_init(cpu_count);
    if (status.id) {
      status_return;
    };
  };
  sp_cpu_count = cpu_count;
  sp_default_random_state = sp_random_state_new(1557083953);
  return ((sp_sine_table_new((&sp_sine_96_table), 96000)));
}
