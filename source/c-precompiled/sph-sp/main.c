
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif
#include <math.h>
#include <errno.h>
#include <arpa/inet.h>
#include <nayuki-fft/fft.c>
#include "../sph-sp/sph-sp.h"
#include <sph-sp/spline-path.c>
#include <sph-sp/quicksort.h>
#include <sph-sp/queue.h>
#include <sph-sp/random.c>
#include <sph-sp/float.c>
#include <sph-sp/thread-pool.h>
#include <sph-sp/thread-pool.c>
#include <sph-sp/futures.h>
#include <sph-sp/futures.c>
#include <sph-sp/helper.c>

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
  void name(type** a, type* b, sp_time_t a_size, sp_channel_count_t channel_count) { \
    sp_time_t b_size; \
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

/** fixed if array is zero, otherwise array value at index */
#define sp_modvalue(fixed, array, index) (array ? array[index] : fixed)
#include <sph-sp/arrays.c>
define_sp_interleave(sp_interleave, sp_sample_t, (b[b_size] = (a[channel])[a_size]))
  define_sp_interleave(sp_deinterleave, sp_sample_t, ((a[channel])[a_size] = b[b_size]))
    sp_sample_t sp_sample_max(sp_sample_t a, sp_sample_t b) { return (((a > b) ? a : b)); }
sp_time_t sp_time_max(sp_time_t a, sp_time_t b) { return (((a > b) ? a : b)); }
sp_sample_t sp_sample_min(sp_sample_t a, sp_sample_t b) { return (((a > b) ? b : a)); }
sp_time_t sp_time_min(sp_time_t a, sp_time_t b) { return (((a > b) ? b : a)); }

/** get a string description for a status id in a status_t */
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
    } else if (sp_s_id_file_write == a.id) {
      b = "invalid file write";
    } else if (sp_s_id_file_read == a.id) {
      b = "invalid file read";
    } else if (sp_s_id_file_not_implemented == a.id) {
      b = "unsupported format (only 32 bit float supported)";
    } else if (sp_s_id_file_eof == a.id) {
      b = "end of file";
    } else {
      b = "";
    };
  } else if (!strcmp(sp_s_group_sph, (a.group))) {
    b = sph_helper_status_description(a);
  } else {
    b = "";
  };
  return (b);
}

/** get a one word identifier for status id in status_t */
uint8_t* sp_status_name(status_t a) {
  uint8_t* b;
  if (0 == strcmp(sp_s_group_sp, (a.group))) {
    if (sp_s_id_input_type == a.id) {
      b = "input-type";
    } else if (sp_s_id_not_implemented == a.id) {
      b = "not-implemented";
    } else if (sp_s_id_memory == a.id) {
      b = "memory";
    } else if (sp_s_id_file_write == a.id) {
      b = "invalid-file-write";
    } else if (sp_s_id_file_read == a.id) {
      b = "invalid-file-read";
    } else if (sp_s_id_file_not_implemented == a.id) {
      b = "not-implemented";
    } else if (sp_s_id_file_eof == a.id) {
      b = "end-of-file";
    } else {
      b = "unknown";
    };
  } else {
    b = "unknown";
  };
  return (b);
}

/** return a newly allocated array for channel-count with data arrays for each channel */
status_t sp_block_new(sp_channel_count_t channel_count, sp_time_t size, sp_block_t* out) {
  status_declare;
  memreg_init(channel_count);
  sp_sample_t* channel;
  for (sp_size_t i = 0; (i < channel_count); i += 1) {
    status_require((sph_helper_calloc((size * sizeof(sp_sample_t)), (&channel))));
    memreg_add(channel);
    (out->samples)[i] = channel;
  };
  out->size = size;
  out->channel_count = channel_count;
exit:
  if (status_is_failure) {
    memreg_free;
  };
  status_return;
}
void sp_block_free(sp_block_t* a) {
  if (a->size) {
    for (sp_size_t i = 0; (i < a->channel_count); i += 1) {
      free(((a->samples)[i]));
    };
  };
}

/** return a new block with offset added to all channel sample arrays */
sp_block_t sp_block_with_offset(sp_block_t a, sp_time_t offset) {
  for (sp_size_t i = 0; (i < a.channel_count); i += 1) {
    (a.samples)[i] += offset;
  };
  return (a);
}
void sp_block_zero(sp_block_t a) {
  for (sp_size_t i = 0; (i < a.channel_count); i += 1) {
    sp_samples_zero(((a.samples)[i]), (a.size));
  };
}

/** copies all channel-count and samples from $a to $b.
   $b channel count and size must be equal or greater than $a */
void sp_block_copy(sp_block_t a, sp_block_t b) {
  for (sp_size_t ci = 0; (ci < a.channel_count); ci += 1) {
    for (sp_size_t i = 0; (i < a.size); i += 1) {
      (b.samples)[ci][i] = (a.samples)[ci][i];
    };
  };
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

/** writes one full period of a sine wave into out. the sine has the frequency that makes it fit exactly into size.
   can be used to create lookup tables */
void sp_sine_period(sp_time_t size, sp_sample_t* out) {
  for (sp_size_t i = 0; (i < size); i += 1) {
    out[i] = sin((i * (M_PI / (size / 2))));
  };
}

/** sums to out */
void sp_wave(sp_time_t size, sp_sample_t* wvf, sp_time_t wvf_size, sp_sample_t amp, sp_sample_t* amod, sp_time_t frq, sp_time_t* fmod, sp_time_t* phs_state, sp_sample_t* out) {
  sp_time_t phs = (phs_state ? *phs_state : 0);
  for (sp_size_t i = 0; (i < size); i += 1) {
    out[i] += (amp * sp_optional_array_get(amod, amp, i) * wvf[phs]);
    phs += sp_optional_array_get(fmod, frq, i);
    if (phs >= wvf_size) {
      phs = (phs % wvf_size);
    };
  };
  if (phs_state) {
    *phs_state = phs;
  };
}
void sp_sine(sp_time_t size, sp_sample_t amp, sp_sample_t* amod, sp_time_t frq, sp_time_t* fmod, sp_time_t* phs_state, sp_sample_t* out) { sp_wave(size, sp_sine_table, sp_rate, amp, amod, frq, fmod, phs_state, out); }

/** supports frequencies below one hertz by using a larger lookup table.
   frq is (hertz / sp_sine_lfo_factor) */
void sp_sine_lfo(sp_time_t size, sp_sample_t amp, sp_sample_t* amod, sp_time_t frq, sp_time_t* fmod, sp_time_t* phs_state, sp_sample_t* out) { sp_wave(size, sp_sine_table_lfo, (sp_rate * sp_sine_lfo_factor), amp, amod, frq, fmod, phs_state, out); }

/** the normalised sinc function */
sp_sample_t sp_sinc(sp_sample_t a) { return (((0 == a) ? 1 : (sin((M_PI * a)) / (M_PI * a)))); }

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
  for (sp_size_t i = 0; (i < a_len); i += 1) {
    a[i] *= -1;
  };
  center = ((a_len - 1) / 2);
  a[center] += 1;
}

/** inverts the sign for samples at odd indexes.
   a-len must be odd and "a" must have left-right symmetry.
   flips the frequency response left to right */
void sp_spectral_reversal_ir(sp_sample_t* a, sp_time_t a_len) {
  while ((a_len > 1)) {
    a_len -= 2;
    a[a_len] *= -1;
  };
}

/** discrete linear convolution.
   out must be all zeros, its length must be at least a-len + b-len - 1.
   out is owned and allocated by the caller */
void sp_convolve_one(sp_sample_t* a, sp_time_t a_len, sp_sample_t* b, sp_time_t b_len, sp_sample_t* out) {
  sp_time_t a_index = 0;
  sp_time_t b_index = 0;
  while ((a_index < a_len)) {
    while ((b_index < b_len)) {
      out[(a_index + b_index)] += (a[a_index] * b[b_index]);
      b_index += 1;
    };
    b_index = 0;
    a_index += 1;
  };
}

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
#include <sph-sp/plot.c>
#include <sph-sp/filter.c>
#include <sph-sp/sequencer.c>
#include <sph-sp/statistics.c>
#include <sph-sp/file.c>
sp_render_config_t sp_render_config(sp_channel_count_t channel_count, sp_time_t rate, sp_time_t block_size, sp_bool_t display_progress) {
  sp_render_config_t a;
  a.channel_count = channel_count;
  a.rate = rate;
  a.block_size = block_size;
  a.display_progress = display_progress;
  return (a);
}

/** render an event with sp_seq to a file. the file is created or overwritten */
status_t sp_render_range_file(sp_event_t event, sp_time_t start, sp_time_t end, sp_render_config_t config, uint8_t* path) {
  status_declare;
  sp_time_t block_end;
  sp_time_t remainder;
  sp_time_t i;
  sp_file_t file;
  sp_block_declare(block);
  sp_declare_event_list(events);
  status_require((sp_event_list_add((&events), event)));
  status_require((sp_block_new((config.channel_count), (config.block_size), (&block))));
  status_require((sp_file_open_write(path, (config.channel_count), (config.rate), (&file))));
  remainder = ((end - start) % config.block_size);
  block_end = (config.block_size * ((end - start) / config.block_size));
  for (i = 0; (i < block_end); i += config.block_size) {
    if (config.display_progress) {
      printf(("%.1f%\n"), (100 * (i / ((sp_sample_t)(block_end)))));
    };
    status_require((sp_seq(i, (i + config.block_size), block, (&events))));
    status_require((sp_file_write((&file), (block.samples), (config.block_size))));
    sp_block_zero(block);
  };
  if (remainder) {
    status_require((sp_seq(i, (i + remainder), block, (&events))));
    status_require((sp_file_write((&file), (block.samples), remainder)));
  };
  sp_event_list_free((&events));
exit:
  sp_block_free((&block));
  sp_file_close_write((&file));
  status_return;
}

/** render a single event with sp_seq to sample arrays in sp_block_t.
   events should have been prepared with sp-seq-events-prepare.
   block will be allocated */
status_t sp_render_range_block(sp_event_t event, sp_time_t start, sp_time_t end, sp_render_config_t config, sp_block_t* out) {
  status_declare;
  sp_block_t block;
  sp_declare_event_list(events);
  status_require((sp_event_list_add((&events), event)));
  status_require((sp_block_new((config.channel_count), (end - start), (&block))));
  status_require((sp_seq(start, end, block, (&events))));
  sp_event_list_free((&events));
  *out = block;
exit:
  status_return;
}

/** render the full duration of event to file at path and write information to standard output.
   uses channel count from global variable sp_channel_count and block size sp_rate */
status_t sp_render_file(sp_event_t event, uint8_t* path) {
  status_declare;
  if (!event.end) {
    sp_event_prepare_optional_srq(event);
  };
  printf("rendering %lu seconds to file %s\n", (event.end / sp_rate), path);
  status_require((sp_render_range_file(event, 0, (event.end), (sp_render_config(sp_channel_count, sp_rate, (sp_render_block_seconds * sp_rate), 1)), path)));
exit:
  status_return;
}

/** render the full duration of event to file at path and write information to standard output.
   uses channel count from global variable sp_channel_count and block size sp_rate */
status_t sp_render_plot(sp_event_t event) {
  status_declare;
  sp_block_t block;
  if (!event.end) {
    sp_event_prepare_optional_srq(event);
  };
  if (event.end && (event.end < sp_rate)) {
    printf("rendering %lu milliseconds to plot\n", (event.end / sp_rate / 1000));
  } else {
    printf("rendering %lu seconds to plot\n", (event.end / sp_rate));
  };
  status_require((sp_render_range_block(event, 0, (event.end), (sp_render_config(sp_channel_count, sp_rate, (sp_render_block_seconds * sp_rate), 1)), (&block))));
  sp_plot_samples(((block.samples)[0]), (event.end));
exit:
  status_return;
}
sp_random_state_t sp_random_state_new(sp_time_t seed) {
  sp_random_state_t result = sph_random_state_new(seed);
  /* random state needs warm-up for some reason */
  sp_time_random_primitive((&result));
  sp_time_random_primitive((&result));
  return (result);
}

/** fills the sine wave lookup table.
   rate and channel-count are used to set sp_rate and sp_channel-count,
   which are used as defaults in a few cases */
status_t sp_initialize(uint16_t cpu_count, sp_channel_count_t channel_count, sp_time_t rate) {
  status_declare;
  if (cpu_count) {
    status.id = sph_future_init(cpu_count);
    if (status.id) {
      status_return;
    };
  };
  sp_sine_table = 0;
  sp_sine_table_lfo = 0;
  sp_cpu_count = cpu_count;
  sp_rate = rate;
  sp_channel_count = channel_count;
  sp_random_state = sp_random_state_new(sp_random_seed);
  sp_sine_lfo_factor = 100;
  status_require((sp_samples_new(sp_rate, (&sp_sine_table))));
  status_require((sp_samples_new((sp_rate * sp_sine_lfo_factor), (&sp_sine_table_lfo))));
  sp_sine_period(sp_rate, sp_sine_table);
  sp_sine_period((sp_rate * sp_sine_lfo_factor), sp_sine_table_lfo);
exit:
  if (status_is_failure) {
    sp_deinitialize();
  };
  status_return;
}
void sp_deinitialize() {
  if (sp_cpu_count) {
    sph_future_deinit();
  };
  free(sp_sine_table);
  free(sp_sine_table_lfo);
}
#include <sph-sp/path.c>

/* extra */

/** return a sample for a triangular wave with center offsets a left and b right.
   creates sawtooth waves if either a or b is 0 */
sp_sample_t sp_triangle(sp_time_t t, sp_time_t a, sp_time_t b) {
  sp_time_t remainder;
  remainder = (t % (a + b));
  return (((remainder < a) ? (remainder * (1 / ((sp_sample_t)(a)))) : ((((sp_sample_t)(b)) - (remainder - ((sp_sample_t)(a)))) * (1 / ((sp_sample_t)(b))))));
}
sp_sample_t sp_square(sp_time_t t, sp_time_t size) { return (((((2 * t) % (2 * size)) < size) ? -1 : 1)); }

/** convert a pan value between 0..channel_count to a volume factor.
   values are interpreted as split between even and odd numbers, from channel..(channel + 1),
   0: second channel muted
   0.5: no channel muted
   1: first channel muted
   0.75: first channel 50%
   0.25: second channel 50% */
sp_sample_t sp_pan_to_amp(sp_sample_t value, sp_channel_count_t channel) { return (((1 & channel) ? (sp_inline_limit(value, (channel - 1), (channel - 0.5)) / 0.5) : (1 - ((sp_inline_limit(value, (channel + 0.5), (channel + 1)) - 0.5) / 0.5)))); }

/** untested. return normally distributed numbers in range */
sp_time_t sp_normal_random(sp_time_t min, sp_time_t max) {
  sp_time_t samples[32];
  sp_sample_t result;
  sp_times_random_bounded((max - min), 32, samples);
  sp_times_add(samples, 32, min);
  sp_stat_times_mean(samples, 32, (&result));
  return (((sp_time_t)(result)));
}

/** untested. round value amount distance to nearest multiple of base.
   for example, amount 1.0 rounds fully, amount 0.0 does not round at all, amount 0.5 rounds half-way */
sp_time_t sp_time_harmonize(sp_time_t a, sp_time_t base, sp_sample_t amount) {
  sp_time_t nearest;
  nearest = ((((a + base) - 1) / base) * base);
  return (((a > nearest) ? (a - (amount * (a - nearest))) : (a + (amount * (nearest - a)))));
}

/** untested. the nearer values are to the multiple, the further move them randomly up to half base away */
sp_time_t sp_time_deharmonize(sp_time_t a, sp_time_t base, sp_sample_t amount) {
  sp_time_t nearest;
  sp_sample_t distance_ratio;
  nearest = ((((a + base) - 1) / base) * base);
  distance_ratio = ((base - sp_inline_absolute_difference(a, nearest)) / ((sp_sample_t)(base)));
  amount = (amount * distance_ratio * (1 + sp_time_random_bounded((base / 2))));
  if ((a > nearest) || (a < amount)) {
    return ((a + amount));
  } else {
    if (a < nearest) {
      return ((a - amount));
    } else {
      if (1 & sp_time_random()) {
        return ((a - amount));
      } else {
        return ((a + amount));
      };
    };
  };
}

/** return the index in divisors where partial_number modulo divisor is zero.
   returns the last divisors index if none were matched.
   for example, if divisors are 3 and 2 and index starts with 1 then every third partial will map to 0 and every second partial to 1.
   for selecting ever nth index */
size_t sp_modulo_match(size_t index, size_t* divisors, size_t divisor_count) {
  for (size_t i = 0; (i < divisor_count); i += 1) {
    if (!(index % divisors[i])) {
      return (i);
    };
  };
  return ((divisor_count - 1));
}
sp_time_t sp_time_expt(sp_time_t base, sp_time_t exp) {
  sp_time_t a = 1;
  while (1) {
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
sp_time_t sp_time_factorial(sp_time_t n) {
  sp_time_t result = 1;
  while ((n > 0)) {
    result *= n;
    n -= 1;
  };
  return (result);
}

/** return the maximum number of possible distinct selections from a set of length "set-size" */
sp_size_t sp_set_sequence_max(sp_size_t set_size, sp_size_t selection_size) { return (((0 == set_size) ? 0 : sp_time_expt(set_size, selection_size))); }
sp_size_t sp_permutations_max(sp_size_t set_size, sp_size_t selection_size) { return ((sp_time_factorial(set_size) / (set_size - selection_size))); }
sp_size_t sp_compositions_max(sp_size_t sum) { return ((sp_time_expt(2, (sum - 1)))); }
