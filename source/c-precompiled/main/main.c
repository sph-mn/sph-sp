#include <stdio.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include <sndfile.h>
#include "../main/sph-sp.h"
#include "../foreign/sph/helper.c"
#include "../foreign/sph/memreg.c"
#include "./kiss_fft.h"
#include "./tools/kiss_fftr.h"
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
#define sp_alsa_status_require(expression) \
  status.id = expression; \
  if (status_is_failure) { \
    status_set_group_goto(sp_status_group_alsa); \
  }
/** write samples for a sine wave into result-samples.
    sample-duration: seconds
    freq: radian frequency
    phase: phase offset
    amp: amplitude. 0..1
    used to define sp-sine, sp-sine-lq and similar */
#define define_sp_sine(id, sin) \
  void id(sp_sample_count_t len, sp_float_t sample_duration, sp_float_t freq, sp_float_t phase, sp_float_t amp, sp_sample_t* result_samples) { \
    sp_sample_count_t index = 0; \
    while ((index <= len)) { \
      result_samples[index] = (amp * sin((freq * phase * sample_duration))); \
      phase = (1 + phase); \
      index = (1 + index); \
    }; \
  }
/** set default if number is negative */
#define optional_set_number(a, default) \
  if (0 > a) { \
    a = default; \
  }
/** default if number is negative */
#define optional_number(a, default) ((0 > a) ? default : a)
/** define a deinterleave, interleave or similar routine.
    a: deinterleaved
    b: interleaved */
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
    } else if (sp_status_id_port_type == a.id) {
      b = "incompatible port type";
    } else {
      b = "";
    };
  } else if (!strcmp(sp_status_group_alsa, (a.group))) {
    b = ((char*)(snd_strerror((a.id))));
  } else if (!strcmp(sp_status_group_sndfile, (a.group))) {
    b = ((char*)(sf_error_number((a.id))));
  } else if (!strcmp(sp_status_group_sph, (a.group))) {
    b = sph_helper_status_description(a);
  } else {
    b = "";
  };
  return (b);
};
uint8_t* sp_status_name(status_t a) {
  char* b;
  if (!strcmp(sp_status_group_alsa, (a.group))) {
    if (sp_status_id_input_type == a.id) {
      b = "input-type";
    } else if (sp_status_id_not_implemented == a.id) {
      b = "not-implemented";
    } else if (sp_status_id_memory == a.id) {
      b = "memory";
    } else {
      b = "unknown";
    };
  } else if (!strcmp(sp_status_group_alsa, (a.group))) {
    b = "alsa";
  } else if (!strcmp(sp_status_group_sndfile, (a.group))) {
    b = "sndfile";
  } else {
    b = "unknown";
  };
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
/** lower precision version of sin() that could be faster */
sp_sample_t sp_sin_lq(sp_sample_t a) {
  sp_sample_t b;
  sp_sample_t c;
  b = (4 / M_PI);
  c = (-4 / (M_PI * M_PI));
  return ((((b * a) + (c * a * abs(a)))));
};
/** the normalised sinc function */
sp_sample_t sp_sinc(sp_sample_t a) { return (((0 == a) ? 1 : (sin((M_PI * a)) / (M_PI * a)))); };
/** result-samples is owned and allocated by the caller.
  fast fourier transform */
status_t sp_fft(sp_sample_count_t len, sp_sample_t* source, sp_sample_count_t source_len, sp_sample_t* result_samples) {
  sp_status_declare;
  kiss_fftr_cfg fftr_state;
  kiss_fft_cpx* out;
  memreg_init(2);
  fftr_state = kiss_fftr_alloc(len, 0, 0, 0);
  if (!fftr_state) {
    status_set_both_goto(sp_status_group_sp, sp_status_id_memory);
  };
  memreg_add(fftr_state);
  status_require((sph_helper_malloc((len * sizeof(kiss_fft_cpx)), (&out))));
  memreg_add(out);
  kiss_fftr(fftr_state, source, out);
  /* extract the real part */
  while (len) {
    len = (len - 1);
    result_samples[len] = (out[len]).r;
  };
exit:
  memreg_free;
  return (status);
};
status_t sp_ifft(sp_sample_count_t len, sp_sample_t* source, sp_sample_count_t source_len, sp_sample_t* result_samples) {
  sp_status_declare;
  kiss_fftr_cfg fftr_state;
  kiss_fft_cpx* in;
  memreg_init(2);
  fftr_state = kiss_fftr_alloc(source_len, 1, 0, 0);
  if (!fftr_state) {
    status_set_id_goto(sp_status_id_memory);
  };
  memreg_add(fftr_state);
  status_require((sph_helper_malloc((source_len * sizeof(kiss_fft_cpx)), (&in))));
  memreg_add(in);
  while (source_len) {
    source_len = (source_len - 1);
    (in[source_len]).r = source[(source_len * sizeof(sp_sample_t))];
  };
  kiss_fftri(fftr_state, in, result_samples);
exit:
  memreg_free;
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
  /* not all required samples in source array */
  if (!((start >= radius) && ((start + radius + 1) <= source_len))) {
    status_require((sph_helper_malloc((width * sizeof(sp_sample_t)), (&window))));
    memreg_add(window);
  };
  while ((start <= end)) {
    /* all required samples are in source array */
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
  result length must be at least a-len + b-len - 1.
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
/** discrete linear convolution for segments of a continuous stream. maps segments (a, a-len) to result-samples
  using (b, b-len) as the impulse response. b-len must be greater than zero.
  result-samples length is a-len, carryover length is b-len or carryover length from previous call.
  all heap memory is owned and allocated by the caller.
  algorithm: copy into result from carryover what overlaps from previous call, write to result,
  copy results that overlap with the next segment to carryover */
void sp_convolve(sp_sample_t* a, sp_sample_count_t a_len, sp_sample_t* b, sp_sample_count_t b_len, sp_sample_count_t carryover_len, sp_sample_t* result_carryover, sp_sample_t* result_samples) {
  sp_sample_count_t size;
  sp_sample_count_t a_index;
  sp_sample_count_t b_index;
  sp_sample_count_t c_index;
  memset(result_samples, 0, (a_len * sizeof(sp_sample_t)));
  memcpy(result_samples, result_carryover, (carryover_len * sizeof(sp_sample_t)));
  memset(result_carryover, 0, (b_len * sizeof(sp_sample_t)));
  /* result values.
restrict processed range to exclude input values that generate carryover */
  size = ((a_len < b_len) ? 0 : (a_len - (b_len - 1)));
  if (size) {
    sp_convolve_one(a, size, b, b_len, result_samples);
  };
  /* carryover values */
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
define_sp_interleave(sp_interleave, sp_sample_t, ({ b[b_size] = (a[channel])[a_size]; }));
define_sp_interleave(sp_deinterleave, sp_sample_t, ({ (a[channel])[a_size] = b[b_size]; }));
define_sp_sine(sp_sine, sin);
define_sp_sine(sp_sine_lq, sp_sin_lq);
#include "../main/windowed-sinc.c"
#include "../main/io.c"
