
#include <sph-sp/sph-sp.h>
#include <sph-sp/sph/filesystem.h>
#include <sph-sp/sph/test.h>
#include <math.h>

#define sp_test_helper_display_summary \
  if (status_is_success) { \
    printf(("--\ntests finished successfully.\n")); \
  } else { \
    printf(("\ntests failed. %d %s\n"), (status.id), (sp_status_description(status))); \
  }
#define feq(a, b) sp_sample_nearly_equal(a, b, (0.01))
#define _sp_rate 960
#define sp_seq_event_count 2
#define test_resonator_event_duration 100
#define test_stats_a_size 8
sp_sample_t error_margin = 0.1;
char* test_file_path = "/tmp/test-sph-sp-file";
status_t test_helper_event_generate(sp_time_t start, sp_time_t end, void* out, sp_event_t* event) {
  status_declare;
  sp_time_t sample_index;
  sp_channel_count_t channel_index;
  uint64_t raw_value;
  sp_sample_t base_value;
  sp_block_t* out_block;
  sp_time_t local_index;
  sp_sample_t sample_value;
  out_block = ((sp_block_t*)(out));
  raw_value = ((uint64_t)(event->config));
  base_value = ((sp_sample_t)(raw_value));
  sample_index = start;
  while ((sample_index < end)) {
    local_index = (sample_index - start);
    sample_value = (base_value * ((sp_sample_t)((sample_index + 1))));
    channel_index = 0;
    while ((channel_index < out_block->channel_count)) {
      (out_block->samples)[channel_index][local_index] += sample_value;
      channel_index = (channel_index + 1);
    };
    sample_index = (sample_index + 1);
  };
  status_return;
}
sp_event_t test_helper_event(sp_time_t start, sp_time_t end, sp_time_t number) {
  sp_declare_event(e);
  e.start = start;
  e.end = end;
  e.generate = test_helper_event_generate;
  e.config = ((void*)(((uint64_t)(number))));
  return (e);
}
status_t test_base(void) {
  status_declare;
  test_helper_assert(("input 0.5"), (sp_sample_nearly_equal((0.63662), (sp_sinc((0.5))), error_margin)));
  test_helper_assert("input 1", (sp_sample_nearly_equal((1.0), (sp_sinc(0)), error_margin)));
exit:
  status_return;
}
status_t test_spectral_inversion_ir(void) {
  status_declare;
  sp_time_t a_len;
  sp_sample_t a[5] = { 0.1, -0.2, 0.3, -0.2, 0.1 };
  a_len = 5;
  sp_spectral_inversion_ir(a, a_len);
  test_helper_assert("result check", (sp_sample_nearly_equal((-0.1), (a[0]), error_margin) && sp_sample_nearly_equal((0.2), (a[1]), error_margin) && sp_sample_nearly_equal((0.7), (a[2]), error_margin) && sp_sample_nearly_equal((0.2), (a[3]), error_margin) && sp_sample_nearly_equal((-0.1), (a[4]), error_margin)));
exit:
  status_return;
}
status_t test_spectral_reversal_ir(void) {
  status_declare;
  sp_time_t a_len;
  sp_sample_t a[5] = { 0.1, -0.2, 0.3, -0.2, 0.1 };
  a_len = 5;
  sp_spectral_reversal_ir(a, a_len);
  test_helper_assert("result check", (sp_sample_nearly_equal((0.1), (a[0]), error_margin) && sp_sample_nearly_equal((0.2), (a[1]), error_margin) && sp_sample_nearly_equal((0.3), (a[2]), error_margin) && sp_sample_nearly_equal((0.2), (a[3]), error_margin) && sp_sample_nearly_equal((0.1), (a[4]), error_margin)));
exit:
  status_return;
}

/** test sp-convolve with a kernel smaller than input */
status_t test_convolve_smaller(void) {
  status_declare;
  sp_sample_t* a;
  sp_time_t a_len;
  sp_sample_t* b;
  sp_time_t b_len;
  sp_sample_t* carryover;
  sp_time_t carryover_len;
  sp_sample_t* result;
  sp_time_t result_len;
  sp_time_t sample_count;
  sp_sample_t expected_result[5] = { 2, 7, 16, 22, 28 };
  sp_sample_t expected_carryover[3] = { 27, 18, 0 };
  memreg_init(4);
  sample_count = 5;
  b_len = 3;
  result_len = sample_count;
  a_len = sample_count;
  carryover_len = b_len;
  status_require((sph_calloc((result_len * sizeof(sp_sample_t)), (&result))));
  memreg_add(result);
  status_require((sph_calloc((a_len * sizeof(sp_sample_t)), (&a))));
  memreg_add(a);
  status_require((sph_calloc((b_len * sizeof(sp_sample_t)), (&b))));
  memreg_add(b);
  status_require((sph_calloc((carryover_len * sizeof(sp_sample_t)), (&carryover))));
  memreg_add(carryover);
  /* prepare input/output data arrays */
  a[0] = 2;
  a[1] = 3;
  a[2] = 4;
  a[3] = 5;
  a[4] = 6;
  b[0] = 1;
  b[1] = 2;
  b[2] = 3;
  carryover[0] = 0;
  carryover[1] = 0;
  carryover[2] = 0;
  /* test convolve first segment */
  sp_convolve(a, a_len, b, b_len, carryover_len, carryover, result);
  test_helper_assert("first result", (sp_samples_nearly_equal(result, result_len, expected_result, result_len, error_margin)));
  test_helper_assert("first result carryover", (sp_samples_nearly_equal(carryover, carryover_len, expected_carryover, carryover_len, error_margin)));
  /* test convolve second segment */
  a[0] = 8;
  a[1] = 9;
  a[2] = 10;
  a[3] = 11;
  a[4] = 12;
  expected_result[0] = 35;
  expected_result[1] = 43;
  expected_result[2] = 52;
  expected_result[3] = 58;
  expected_result[4] = 64;
  expected_carryover[0] = 57;
  expected_carryover[1] = 36;
  expected_carryover[2] = 0;
  sp_convolve(a, a_len, b, b_len, carryover_len, carryover, result);
  test_helper_assert("second result", (sp_samples_nearly_equal(result, result_len, expected_result, result_len, error_margin)));
  test_helper_assert("second result carryover", (sp_samples_nearly_equal(carryover, carryover_len, expected_carryover, carryover_len, error_margin)));
exit:
  memreg_free;
  status_return;
}

/** test sp-convolve with a kernel larger than input and process more blocks */
status_t test_convolve_larger(void) {
  status_declare;
  sp_sample_t* in_a;
  sp_sample_t* in_b;
  sp_sample_t* out;
  sp_sample_t* out_control;
  sp_sample_t* carryover;
  sp_time_t carryover_length;
  sp_time_t in_a_length;
  sp_time_t in_b_length;
  sp_time_t block_size;
  sp_time_t block_count;
  memreg_init(5);
  in_a = 0;
  in_b = 0;
  out = 0;
  carryover = 0;
  out_control = 0;
  block_size = 10;
  block_count = 10;
  in_a_length = 100;
  in_b_length = 15;
  carryover_length = in_b_length;
  status_require((sp_samples_new(in_a_length, (&in_a))));
  memreg_add(in_a);
  status_require((sp_samples_new(in_b_length, (&in_b))));
  memreg_add(in_b);
  status_require((sp_samples_new(in_a_length, (&out))));
  memreg_add(out);
  status_require((sp_samples_new(in_a_length, (&out_control))));
  memreg_add(out_control);
  status_require((sp_samples_new(in_b_length, (&carryover))));
  memreg_add(carryover);
  for (sp_size_t i = 0; (i < in_a_length); i += 1) {
    in_a[i] = i;
  };
  for (sp_size_t i = 0; (i < in_b_length); i += 1) {
    in_b[i] = (1 + i);
  };
  sp_convolve(in_a, in_a_length, in_b, in_b_length, carryover_length, carryover, out_control);
  sp_samples_zero(carryover, in_b_length);
  for (sp_size_t i = 0; (i < block_count); i += 1) {
    sp_convolve(((i * block_size) + in_a), block_size, in_b, in_b_length, carryover_length, carryover, ((i * block_size) + out));
    carryover_length = in_b_length;
  };
  test_helper_assert("equal to block processing result", (sp_samples_nearly_equal(out, in_a_length, out_control, in_a_length, (0.01))));
exit:
  memreg_free;
  status_return;
}
status_t test_sinc_make_minimum_phase(void) {
  status_declare;
  sp_time_t length;
  sp_time_t center_index;
  sp_time_t index;
  sp_sample_t cutoff_factor;
  sp_sample_t impulse_linear[63];
  sp_sample_t impulse_minimum[63];
  sp_sample_t real_linear[63];
  sp_sample_t imag_linear[63];
  sp_sample_t real_minimum[63];
  sp_sample_t imag_minimum[63];
  sp_sample_t energy_linear;
  sp_sample_t energy_minimum;
  sp_sample_t cumulative_energy_linear;
  sp_sample_t cumulative_energy_minimum;
  sp_time_t energy_index_linear;
  sp_time_t energy_index_minimum;
  sp_sample_t tolerance_energy;
  sp_sample_t tolerance_magnitude;
  sp_sample_t magnitude_linear;
  sp_sample_t magnitude_minimum;
  sp_sample_t delta_index;
  length = 63;
  center_index = ((length - 1) / 2);
  cutoff_factor = 0.1;
  tolerance_energy = 1.0e-6;
  tolerance_magnitude = 1.0e-4;
  index = 0;
  while ((index < length)) {
    delta_index = (((sp_sample_t)(index)) - ((sp_sample_t)(center_index)));
    impulse_linear[index] = (2.0 * cutoff_factor * sp_sinc((2.0 * cutoff_factor * delta_index)));
    impulse_minimum[index] = impulse_linear[index];
    index = (index + 1);
  };
  status_require((sp_sinc_make_minimum_phase(impulse_minimum, length)));
  index = 0;
  while ((index < length)) {
    real_linear[index] = impulse_linear[index];
    imag_linear[index] = 0.0;
    real_minimum[index] = impulse_minimum[index];
    imag_minimum[index] = 0.0;
    index = (index + 1);
  };
  status_i_require((sp_fft(length, real_linear, imag_linear)));
  status_i_require((sp_fft(length, real_minimum, imag_minimum)));
  energy_linear = 0.0;
  energy_minimum = 0.0;
  index = 0;
  while ((index < length)) {
    energy_linear = (energy_linear + (impulse_linear[index] * impulse_linear[index]));
    energy_minimum = (energy_minimum + (impulse_minimum[index] * impulse_minimum[index]));
    index = (index + 1);
  };
  test_helper_assert("sinc_minphase_energy_preserved", (fabs((energy_linear - energy_minimum)) <= tolerance_energy));
  index = 0;
  while ((index < length)) {
    magnitude_linear = sqrt(((real_linear[index] * real_linear[index]) + (imag_linear[index] * imag_linear[index])));
    magnitude_minimum = sqrt(((real_minimum[index] * real_minimum[index]) + (imag_minimum[index] * imag_minimum[index])));
    test_helper_assert("sinc_minphase_magnitude_equal", (fabs((magnitude_linear - magnitude_minimum)) <= tolerance_magnitude));
    index = (index + 1);
  };
  cumulative_energy_linear = 0.0;
  cumulative_energy_minimum = 0.0;
  energy_index_linear = (length - 1);
  energy_index_minimum = (length - 1);
  index = 0;
  while ((index < length)) {
    cumulative_energy_linear = (cumulative_energy_linear + (impulse_linear[index] * impulse_linear[index]));
    cumulative_energy_minimum = (cumulative_energy_minimum + (impulse_minimum[index] * impulse_minimum[index]));
    if ((cumulative_energy_linear >= (0.5 * energy_linear)) && (energy_index_linear == (length - 1))) {
      energy_index_linear = index;
    };
    if ((cumulative_energy_minimum >= (0.5 * energy_minimum)) && (energy_index_minimum == (length - 1))) {
      energy_index_minimum = index;
    };
    index = (index + 1);
  };
  test_helper_assert("sinc_minphase_energy_earlier", (energy_index_minimum <= energy_index_linear));
exit:
  status_return;
}
status_t test_resonator_filter_continuity(void) {
  status_declare;
  sp_sample_t* in;
  sp_sample_t* out;
  sp_sample_t* out_control;
  sp_convolution_filter_state_t* state;
  sp_convolution_filter_state_t* state_control;
  sp_time_t size;
  sp_time_t block_size;
  sp_time_t block_count;
  sp_sample_t cutoff_low;
  sp_sample_t cutoff_high;
  sp_sample_t transition;
  uint8_t arguments_buffer[(3 * sizeof(sp_sample_t))];
  uint8_t arguments_length;
  sp_size_t sample_index;
  sp_size_t block_index;
  sp_time_t offset;
  memreg_init(3);
  size = 100;
  block_size = 10;
  block_count = 10;
  state = 0;
  state_control = 0;
  cutoff_low = 0.1;
  cutoff_high = 0.3;
  transition = 0.1;
  status_require((sp_samples_new(size, (&in))));
  memreg_add(in);
  status_require((sp_samples_new(size, (&out))));
  memreg_add(out);
  status_require((sp_samples_new(size, (&out_control))));
  memreg_add(out_control);
  sample_index = 0;
  while ((sample_index < size)) {
    in[sample_index] = ((sp_sample_t)(sample_index));
    sample_index = (sample_index + 1);
  };
  arguments_length = (3 * sizeof(sp_sample_t));
  *((sp_sample_t*)(arguments_buffer)) = cutoff_low;
  *(((sp_sample_t*)(arguments_buffer)) + 1) = cutoff_high;
  *(((sp_sample_t*)(arguments_buffer)) + 2) = transition;
  status_require((sp_convolution_filter(in, size, sp_resonator_ir_f, arguments_buffer, arguments_length, (&state_control), out_control)));
  block_index = 0;
  while ((block_index < block_count)) {
    offset = (block_index * block_size);
    status_require((sp_convolution_filter((in + offset), block_size, sp_resonator_ir_f, arguments_buffer, arguments_length, (&state), (out + offset))));
    block_index = (block_index + 1);
  };
  test_helper_assert("resonator continuity", (sp_samples_nearly_equal(out, size, out_control, size, (0.01))));
  memreg_free;
  sp_convolution_filter_state_uninit(state);
  sp_convolution_filter_state_uninit(state_control);
exit:
  status_return;
}
status_t test_resonator_ir_and_filter(void) {
  status_declare;
  sp_sample_t cutoff_low;
  sp_sample_t cutoff_high;
  sp_sample_t transition;
  sp_sample_t* ir;
  sp_time_t ir_len;
  sp_convolution_filter_state_t* state;
  sp_sample_t source[10];
  sp_sample_t result[10];
  uint8_t arguments_buffer[(3 * sizeof(sp_sample_t))];
  uint8_t arguments_length;
  state = 0;
  source[0] = 3;
  source[1] = 4;
  source[2] = 5;
  source[3] = 6;
  source[4] = 7;
  source[5] = 8;
  source[6] = 9;
  source[7] = 0;
  source[8] = 1;
  source[9] = 2;
  result[0] = 0;
  result[1] = 0;
  result[2] = 0;
  result[3] = 0;
  result[4] = 0;
  result[5] = 0;
  result[6] = 0;
  result[7] = 0;
  result[8] = 0;
  result[9] = 0;
  /* direct IR construction: normal parameters */
  cutoff_low = 0.1;
  cutoff_high = 0.3;
  transition = 0.05;
  status_require((sp_resonator_ir(cutoff_low, cutoff_high, transition, (&ir), (&ir_len))));
  test_helper_assert("resonator ir length positive", (ir_len > 0));
  free(ir);
  /* direct IR construction: clamping branches (low < 0, high > 0.5, transition <= 0) */
  cutoff_low = -0.1;
  cutoff_high = 0.6;
  transition = 0.0;
  status_require((sp_resonator_ir(cutoff_low, cutoff_high, transition, (&ir), (&ir_len))));
  test_helper_assert("resonator ir clamped length positive", (ir_len > 0));
  free(ir);
  /* IR through ir_f adapter */
  cutoff_low = 0.05;
  cutoff_high = 0.2;
  transition = 0.03;
  arguments_length = (3 * sizeof(sp_sample_t));
  *((sp_sample_t*)(arguments_buffer)) = cutoff_low;
  *(((sp_sample_t*)(arguments_buffer)) + 1) = cutoff_high;
  *(((sp_sample_t*)(arguments_buffer)) + 2) = transition;
  status_require((sp_resonator_ir_f(arguments_buffer, (&ir), (&ir_len))));
  test_helper_assert("resonator ir_f length positive", (ir_len > 0));
  free(ir);
  /* filter through convolution: one call to create state and IR */
  status_require((sp_convolution_filter(source, 10, sp_resonator_ir_f, arguments_buffer, arguments_length, (&state), result)));
  /* second call with identical arguments: exercises state reuse path */
  status_require((sp_convolution_filter(source, 10, sp_resonator_ir_f, arguments_buffer, arguments_length, (&state), result)));
  if (state) {
    sp_convolution_filter_state_uninit(state);
  };
exit:
  status_return;
}
status_t test_moving_average(void) {
  status_declare;
  memreg_init(2);
  sp_sample_t* in;
  sp_sample_t* out;
  sp_time_t radius;
  sp_time_t size;
  sp_path_t path;
  sp_time_t t[2] = { 0, 10 };
  sp_sample_t y[2] = { 0, 5 };
  sp_time_t cursor;
  size = 11;
  radius = 4;
  path.t = t;
  (path.values)[0] = y;
  (path.values)[1] = 0;
  path.point_count = 2;
  cursor = 0;
  status_require((sp_samples_new(size, (&in))));
  memreg_add(in);
  sp_path_get((&path), 0, size, in, (&cursor));
  status_require((sp_samples_new(size, (&out))));
  memreg_add(out);
  /* without prev/next */
  sp_moving_average(in, size, 0, 0, radius, out);
  test_helper_assert("without prev/next", (sp_sample_nearly_equal((2.0), (out[1]), error_margin) && sp_sample_nearly_equal((5.5), (out[5]), error_margin) && sp_sample_nearly_equal((2.0), (out[9]), error_margin)));
  /* with prev/next */
  sp_samples_zero(out, size);
  sp_sample_t prev[11] = { 0, 0, 0, 0, 0, 0, 0, -8, -6, -4, -1 };
  sp_sample_t next[4] = { -1, -4, -6, -8 };
  sp_moving_average(in, size, prev, next, radius, out);
  test_helper_assert("with prev/next", (sp_sample_nearly_equal((2.11), (out[1]), error_margin) && sp_sample_nearly_equal((5.5), (out[5]), error_margin) && sp_sample_nearly_equal((2.11), (out[9]), error_margin)));
exit:
  memreg_free;
  status_return;
}
status_t test_file(void) {
  status_declare;
  sp_channel_count_t channel_count;
  sp_file_t file;
  sp_time_t sample_count;
  sp_time_t sample_rate;
  sp_block_declare(block_write);
  sp_block_declare(block_read);
  if (file_exists(test_file_path)) {
    unlink(test_file_path);
  };
  channel_count = 2;
  sample_rate = 8000;
  sample_count = 5;
  status_require((sp_block_init(channel_count, sample_count, (&block_write))));
  for (sp_channel_count_t j = 0; (j < channel_count); j += 1) {
    for (sp_time_t i = 0; (i < sample_count); i += 1) {
      ((block_write.samples)[j])[i] = (sample_count - i);
    };
  };
  /* test write */
  status_require((sp_file_open_write(test_file_path, channel_count, sample_rate, (&file))));
  status_require((sp_file_write((&file), (block_write.samples), sample_count)));
  sp_file_close_write((&file));
  /* test read */
  status_require((sp_block_init(channel_count, sample_count, (&block_read))));
  status_require((sp_file_open_read(test_file_path, (&file))));
  status_require((sp_file_read(file, sample_count, (block_read.samples))));
  for (sp_channel_count_t j = 0; (j < channel_count); j += 1) {
    test_helper_assert("sp-file-read new file result", (sp_samples_nearly_equal(((block_write.samples)[j]), sample_count, ((block_read.samples)[j]), sample_count, error_margin)));
  };
  sp_file_close_read(file);
exit:
  sp_block_uninit((&block_write));
  sp_block_uninit((&block_read));
  status_return;
}
status_t test_fft(void) {
  status_declare;
  sp_sample_t real_values[8];
  sp_sample_t imag_values[8];
  sp_sample_t real_values_original[8];
  sp_sample_t imag_values_original[8];
  sp_sample_t impulse_real[8];
  sp_sample_t impulse_imag[8];
  sp_time_t length;
  sp_time_t index;
  sp_sample_t tolerance;
  length = 8;
  tolerance = 1.0e-9;
  index = 0;
  while ((index < length)) {
    real_values[index] = ((((sp_sample_t)(index)) * 0.1) + 0.05);
    imag_values[index] = (((sp_sample_t)((length - index))) * 0.01);
    real_values_original[index] = real_values[index];
    imag_values_original[index] = imag_values[index];
    index = (index + 1);
  };
  status_i_require((sp_fft(length, real_values, imag_values)));
  status_i_require((sp_ffti(length, real_values, imag_values)));
  index = 0;
  while ((index < length)) {
    test_helper_assert("fft_roundtrip_real", (fabs(((real_values[index] / ((sp_sample_t)(length))) - real_values_original[index])) <= tolerance));
    test_helper_assert("fft_roundtrip_imag", (fabs(((imag_values[index] / ((sp_sample_t)(length))) - imag_values_original[index])) <= tolerance));
    index = (index + 1);
  };
  index = 0;
  while ((index < length)) {
    impulse_real[index] = 0.0;
    impulse_imag[index] = 0.0;
    index = (index + 1);
  };
  impulse_real[0] = 1.0;
  status_i_require((sp_fft(length, impulse_real, impulse_imag)));
  index = 0;
  while ((index < length)) {
    test_helper_assert("fft_impulse_real", (fabs((impulse_real[index] - 1.0)) <= tolerance));
    test_helper_assert("fft_impulse_imag", (fabs((impulse_imag[index])) <= tolerance));
    index = (index + 1);
  };
exit:
  status_return;
}

/** better test separately as it opens gnuplot windows */
status_t test_sp_plot(void) {
  status_declare;
  sp_sample_t a[9] = { 0.1, -0.2, 0.1, -0.4, 0.3, -0.4, 0.2, -0.2, 0.1 };
  sp_plot_samples(a, 9);
  sp_plot_spectrum(a, 9);
  status_return;
}
status_t test_sp_triangle_square(void) {
  status_declare;
  sp_sample_t* out_t;
  sp_sample_t* out_s;
  sp_time_t size;
  size = 96000;
  status_require((sph_calloc((size * sizeof(sp_sample_t*)), (&out_t))));
  status_require((sph_calloc((size * sizeof(sp_sample_t*)), (&out_s))));
  for (sp_size_t i = 0; (i < size); i += 1) {
    out_t[i] = sp_triangle(i, (size / 2), (size / 2));
    out_s[i] = sp_square(i, size);
  };
  test_helper_assert("triangle 0", (0 == out_t[0]));
  test_helper_assert("triangle 1/2", (1 == out_t[48000]));
  test_helper_assert("triangle 1", (sp_sample_nearly_equal(0, (out_t[95999]), error_margin)));
  test_helper_assert("square 0", (-1 == out_s[0]));
  test_helper_assert("square 1/4", (-1 == out_s[24000]));
  test_helper_assert("square 1/2 - 1", (-1 == out_s[47999]));
  test_helper_assert("square 1/2", (1 == out_s[48000]));
  test_helper_assert("square 3/4", (1 == out_s[72000]));
  test_helper_assert("square 1", (1 == out_s[95999]));
  free(out_t);
  free(out_s);
exit:
  status_return;
}
status_t test_sp_random(void) {
  status_declare;
  sp_random_state_t s;
  sp_sample_t out[20];
  s = sp_random_state_new(80);
  sp_samples_random_primitive((&s), 10, out);
  sp_samples_random_primitive((&s), 10, (10 + out));
  test_helper_assert("last value", (sp_sample_nearly_equal((-0.553401), (out[19]), error_margin)));
exit:
  status_return;
}
status_t test_sp_seq(void) {
  status_declare;
  sp_block_t out;
  sp_block_t shifted;
  sp_declare_event_list(events);
  status_require((sp_event_list_add((&events), (test_helper_event(0, 40, 1)))));
  status_require((sp_event_list_add((&events), (test_helper_event(41, 100, 2)))));
  status_require((sp_block_init(2, 100, (&out))));
  sp_seq_events_prepare((&events));
  sp_seq(0, 50, (&out), (&events));
  shifted = sp_block_with_offset(out, 50);
  sp_seq(50, 100, (&shifted), (&events));
  test_helper_assert("block contents 1 event 1", ((1 == (out.samples)[0][0]) && (40 == (out.samples)[0][39])));
  test_helper_assert("block contents 1 gap", (0 == (out.samples)[0][40]));
  test_helper_assert("block contents 1 event 2", ((2 == (out.samples)[0][41]) && (118 == (out.samples)[0][99])));
  sp_event_list_uninit((&events));
  sp_block_uninit((&out));
exit:
  status_return;
}
status_t test_sp_group(void) {
  status_declare;
  sp_block_t block;
  sp_block_t shifted;
  sp_time_t* m1;
  sp_time_t* m2;
  sp_declare_event(g);
  sp_declare_event(g1);
  sp_declare_event(e1);
  sp_declare_event(e2);
  sp_declare_event(e3);
  status_require((sp_times_new(100, (&m1))));
  status_require((sp_times_new(100, (&m2))));
  sp_group_event((&g));
  sp_group_event((&g1));
  g1.start = 10;
  status_require((sp_block_init(2, 100, (&block))));
  e1 = test_helper_event(0, 20, 1);
  e2 = test_helper_event(20, 40, 2);
  e3 = test_helper_event(50, 100, 3);
  status_require((sp_group_add((&g1), e1)));
  status_require((sp_group_add((&g1), e2)));
  status_require((sp_group_add((&g), g1)));
  status_require((sp_group_add((&g), e3)));
  status_require((sp_event_memory_ensure((&g), 2)));
  status_require((sp_event_memory_add((&g), m1)));
  status_require((sp_event_memory_add((&g), m2)));
  status_require(((g.prepare)((&g))));
  status_require(((g.generate)(0, 50, (&block), (&g))));
  shifted = sp_block_with_offset(block, 50);
  status_require(((g.generate)(50, 100, (&shifted), (&g))));
  (g.free)((&g));
  test_helper_assert("block contents event 1", ((1 == (block.samples)[0][10]) && (20 == (block.samples)[0][29])));
  test_helper_assert("block contents event 2", ((2 == (block.samples)[0][30]) && (20 == (block.samples)[0][39])));
  test_helper_assert("block contents gap", ((0 == (block.samples)[0][40]) && (0 == (block.samples)[0][49])));
  test_helper_assert("block contents event 3", ((3 == (block.samples)[0][50]) && (150 == (block.samples)[0][99])));
  sp_block_uninit((&block));
exit:
  status_return;
}
uint8_t u64_from_array_test(sp_time_t size) {
  uint64_t bits_in;
  uint64_t bits_out;
  bits_in = 9838263505978427528u;
  bits_out = sp_u64_from_array(((uint8_t*)(&bits_in)), size);
  return ((0 == memcmp(((uint8_t*)(&bits_in)), ((uint8_t*)(&bits_out)), size)));
}
status_t test_times(void) {
  status_declare;
  sp_time_t size;
  sp_time_t* a_temp;
  sp_time_t a[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  sp_time_t b[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  sp_time_t b_size;
  sp_time_t* bits;
  a_temp = 0;
  size = 8;
  sp_times_geometric(1, 3, size, a);
  test_helper_assert("multiplications", (81 == a[4]));
  sp_times_additions(1, 3, size, a);
  test_helper_assert("additions", (13 == a[4]));
  sp_time_t indices[3] = { 1, 2, 4 };
  sp_times_select(a, indices, 3, a);
  test_helper_assert("select", ((4 == a[0]) && (7 == a[1]) && (13 == a[2])));
  sp_times_set(a, size, 1039);
  status_require((sp_times_new((8 * sizeof(sp_time_t)), (&bits))));
  sp_times_bits_to_times(a, (8 * sizeof(sp_time_t)), bits);
  test_helper_assert(("bits->times"), ((1 == bits[0]) && (1 == bits[3]) && (0 == bits[4]) && (1 == bits[10])));
  free(bits);
  sp_times_geometric(1, 3, size, a);
  sp_times_shuffle(a, size);
  sp_times_random_binary(size, a);
  sp_times_geometric(1, 3, size, a);
  sp_times_select_random(a, size, b, (&b_size));
exit:
  free(a_temp);
  status_return;
}
status_t test_statistics(void) {
  status_declare;
  sp_time_t size;
  sp_time_t a[test_stats_a_size] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  sp_sample_t as[test_stats_a_size] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  sp_time_t inhar_1[test_stats_a_size] = { 2, 4, 6, 8, 10, 12, 14, 16 };
  sp_time_t inhar_2[test_stats_a_size] = { 2, 4, 6, 8, 10, 12, 13, 16 };
  sp_time_t inhar_3[test_stats_a_size] = { 2, 3, 6, 8, 10, 12, 13, 16 };
  sp_sample_t inhar_results[3];
  sp_sample_t stat_out;
  size = test_stats_a_size;
  sp_stat_times_mean(a, size, (&stat_out));
  test_helper_assert("mean", (feq((4.5), stat_out)));
  sp_stat_times_deviation(a, size, (&stat_out));
  test_helper_assert("deviation", (feq((2.29), stat_out)));
  sp_stat_times_center(a, size, (&stat_out));
  test_helper_assert("center", (feq((4.54), stat_out)));
  sp_stat_times_median(a, size, (&stat_out));
  test_helper_assert("median", (feq((4.5), stat_out)));
  sp_stat_times_skewness(a, size, (&stat_out));
  test_helper_assert("skewness", (feq((0.0), stat_out)));
  sp_stat_times_kurtosis(a, size, (&stat_out));
  test_helper_assert("kurtosis", (feq((1.76), stat_out)));
  sp_stat_samples_mean(as, size, (&stat_out));
  test_helper_assert("samples mean", (feq((4.5), stat_out)));
  sp_stat_samples_deviation(as, size, (&stat_out));
  test_helper_assert("samples deviation", (feq((2.29), stat_out)));
  sp_stat_samples_center(as, size, (&stat_out));
  test_helper_assert("samples center", (feq((4.66), stat_out)));
  sp_stat_samples_median(as, size, (&stat_out));
  test_helper_assert("samples median", (feq((4.5), stat_out)));
  sp_stat_samples_skewness(as, size, (&stat_out));
  test_helper_assert("samples skewness", (feq((0.0), stat_out)));
  sp_stat_samples_kurtosis(as, size, (&stat_out));
  test_helper_assert("samples kurtosis", (feq((1.76), stat_out)));
  /* inharmonicity */
  sp_stat_times_inharmonicity(inhar_1, size, (inhar_results + 0));
  sp_stat_times_inharmonicity(inhar_2, size, (inhar_results + 1));
  sp_stat_times_inharmonicity(inhar_3, size, (inhar_results + 2));
  test_helper_assert("inharmonicity relations", ((inhar_results[0] < inhar_results[1]) && (inhar_results[1] < inhar_results[2])));
  test_helper_assert("inharmonicity 1", (feq((0.0), (inhar_results[0]))));
  test_helper_assert("inharmonicity 2", (feq((0.0625), (inhar_results[1]))));
  test_helper_assert("inharmonicity 3", (feq((0.125), (inhar_results[2]))));
exit:
  status_return;
}
status_t test_render_range_block(void) {
  status_declare;
  sp_block_t out;
  sp_render_config_t render_config;
  sp_declare_event(event);
  error_memory_init(1);
  render_config = sp_render_config(sp_channel_count, sp_rate, sp_rate, 0);
  render_config.block_size = 40;
  status_require((sp_block_init(1, test_resonator_event_duration, (&out))));
  error_memory_add2((&out), sp_block_uninit);
  event.start = 0;
  event.end = test_resonator_event_duration;
  event.config = ((void*)(1));
  event.prepare = 0;
  event.generate = test_helper_event_generate;
  event.free = 0;
  sp_render_range_block(event, 0, test_resonator_event_duration, render_config, (&out));
  test_helper_assert("render range block nonzero", (sp_samples_max(((out.samples)[0]), test_resonator_event_duration) > 0.0));
  sp_block_uninit((&out));
  if (status_is_failure) {
    error_memory_free;
  };
exit:
  status_return;
}
status_t test_simple_mappings(void) {
  status_declare;
  sp_time_t size;
  sp_time_t a[4] = { 1, 1, 1, 1 };
  sp_time_t b[4] = { 2, 2, 2, 2 };
  sp_sample_t as[4] = { 1, 1, 1, 1 };
  size = 4;
  /* times */
  sp_times_set(a, size, 0);
  sp_samples_set(as, size, 0);
  test_helper_assert("times set", (sp_times_equal(a, size, 0)));
  test_helper_assert("samples set", (sp_samples_equal(as, size, 0)));
  sp_times_add(a, size, 1);
  test_helper_assert("add", (sp_times_equal(a, size, 1)));
  sp_times_subtract(a, size, 10);
  test_helper_assert("subtract", (sp_times_equal(a, size, 0)));
  sp_times_add(a, size, 4);
  sp_times_multiply(a, size, 2);
  test_helper_assert("multiply", (sp_times_equal(a, size, 8)));
  sp_times_divide(a, size, 2);
  test_helper_assert("divide", (sp_times_equal(a, size, 4)));
  sp_times_set(a, size, 4);
  sp_times_add_times(a, size, b);
  test_helper_assert("add-times", (sp_times_equal(a, size, 6)));
  sp_times_set(a, size, 4);
  sp_times_subtract_times(a, size, b);
  test_helper_assert("subtract", (sp_times_equal(a, size, 2)));
  sp_times_set(a, size, 4);
  sp_times_multiply_times(a, size, b);
  test_helper_assert("multiply", (sp_times_equal(a, size, 8)));
  sp_times_set(a, size, 4);
  sp_times_divide_times(a, size, b);
  test_helper_assert("divide", (sp_times_equal(a, size, 2)));
  sp_times_set(a, size, 1);
exit:
  status_return;
}
status_t test_random_discrete(void) {
  status_declare;
  sp_time_t i;
  sp_time_t size;
  sp_time_t cudist_size;
  sp_time_t prob[4] = { 0, 3, 0, 3 };
  sp_time_t cudist[4];
  sp_time_t a[8];
  cudist_size = 4;
  size = 8;
  sp_times_cusum(prob, size, cudist);
  sp_times_random_discrete(cudist, cudist_size, 8, a);
  for (i = 1; (i < size); i = (1 + i)) {
    test_helper_assert("random-discrete", ((1 == a[i]) || (3 == a[i])));
  };
exit:
  status_return;
}
status_t test_compositions(void) {
  sp_time_t** out;
  sp_time_t out_size;
  sp_time_t* out_sizes;
  sp_time_t i;
  sp_time_t* b;
  status_declare;
  status_require((sp_times_compositions(5, (&out), (&out_size), (&out_sizes))));
  for (i = 0; (i < out_size); i += 1) {
    b = out[i];
    free(b);
  };
  free(out);
exit:
  status_return;
}
status_t test_permutations(void) {
  status_declare;
  sp_time_t in[3] = { 1, 2, 3 };
  sp_size_t out_size;
  sp_time_t** out;
  sp_size_t size = 3;
  status_require((sp_times_permutations(size, in, size, (&out), (&out_size))));
  for (sp_size_t i = 0; (i < out_size); i += 1) {
    free((out[i]));
  };
  free(out);
exit:
  status_return;
}
status_t test_sp_resonator_event(void) {
  status_declare;
  sp_block_t out;
  sp_sample_t amod1[test_resonator_event_duration];
  sp_sample_t amod2[test_resonator_event_duration];
  sp_resonator_event_config_t* config;
  sp_size_t sample_index;
  sp_resonator_event_channel_config_t* channel_config;
  sp_declare_event(event);
  error_memory_init(2);
  status_require((sp_resonator_event_config_init((&config))));
  error_memory_add(config);
  status_require((sp_block_init(2, test_resonator_event_duration, (&out))));
  error_memory_add2((&out), sp_block_uninit);
  sample_index = 0;
  while ((sample_index < test_resonator_event_duration)) {
    amod1[sample_index] = 0.9;
    amod2[sample_index] = 0.8;
    sample_index += 1;
  };
  config->channel_count = 2;
  config->bandwidth_threshold = 50.0;
  channel_config = (config->channel_config + 0);
  channel_config->use = 1;
  channel_config->amp = 1.0;
  channel_config->amod = amod1;
  channel_config->frq = 2000.0;
  channel_config->wdt = 10.0;
  channel_config = (config->channel_config + 1);
  channel_config->use = 1;
  channel_config->amp = 0.5;
  channel_config->amod = amod2;
  channel_config->frq = 2000.0;
  channel_config->wdt = 200.0;
  event.start = 0;
  event.end = test_resonator_event_duration;
  sp_resonator_event((&event), config);
  status_require(((event.prepare)((&event))));
  status_require(((event.generate)(0, 30, (&out), (&event))));
  status_require(((event.generate)(30, test_resonator_event_duration, (&out), (&event))));
  test_helper_assert("resonator amod applied channel 0", (feq((0.9), (sp_samples_max(((out.samples)[0]), test_resonator_event_duration)))));
  test_helper_assert("resonator channel 1 in range", (1.0 >= sp_samples_absolute_max(((out.samples)[1]), test_resonator_event_duration)));
  test_helper_assert("resonator channel 1 nonzero", (sp_samples_absolute_max(((out.samples)[1]), test_resonator_event_duration) > 0.0));
  sp_block_uninit((&out));
exit:
  if (status_is_failure) {
    error_memory_free;
  };
  status_return;
}
status_t test_sp_map_event_generate(sp_time_t start, sp_time_t end, void* in, void* out, void* state) {
  status_declare;
  sp_block_copy((*((sp_block_t*)(in))), (*((sp_block_t*)(out))));
  status_return;
}
status_t test_sp_map_event(void) {
  status_declare;
  sp_time_t size;
  sp_block_t block;
  sp_map_event_config_t* map_event_config;
  sp_declare_event(parent);
  sp_declare_event(child);
  error_memory_init(1);
  status_require((sp_map_event_config_init((&map_event_config))));
  error_memory_add(map_event_config);
  size = (10 * _sp_rate);
  child = test_helper_event(0, size, 3);
  status_require((sp_block_init(1, size, (&block))));
  map_event_config->event = child;
  map_event_config->map_generate = test_sp_map_event_generate;
  map_event_config->isolate = 1;
  parent.start = child.start;
  parent.end = child.end;
  sp_map_event((&parent), map_event_config);
  status_require(((parent.prepare)((&parent))));
  status_require(((parent.generate)(0, (size / 2), (&block), (&parent))));
  status_require(((parent.generate)((size / 2), size, (&block), (&parent))));
  (parent.free)((&parent));
  sp_block_uninit((&block));
  if (status_is_failure) {
    error_memory_free;
  };
exit:
  status_return;
}
status_t test_sp_seq_parallel_block(void) {
  status_declare;
  sp_time_t size;
  sp_time_t step_size;
  sp_time_t event_count;
  sp_time_t event_index;
  sp_time_t position;
  sp_block_t block;
  sp_block_t shifted;
  sp_sample_t event_sum;
  sp_time_t sample_index;
  sp_sample_t expected_value;
  sp_sample_t sample_value_ch0;
  sp_sample_t sample_value_ch1;
  sp_declare_event(event);
  sp_declare_event_list(events);
  size = 1000;
  event_count = 10;
  step_size = (size / 10);
  status_require((sp_block_init(2, size, (&block))));
  event.start = 0;
  event.end = size;
  event.prepare = 0;
  event.generate = test_helper_event_generate;
  event.free = 0;
  event_index = 0;
  while ((event_index < event_count)) {
    event.config = ((void*)(((uintptr_t)((event_index + 1)))));
    status_require((sp_event_list_add((&events), event)));
    event_index += 1;
  };
  sp_seq_events_prepare((&events));
  position = 0;
  while ((position < size)) {
    shifted = sp_block_with_offset(block, position);
    sp_seq_parallel_block(position, (position + step_size), shifted, (&events));
    position = (position + step_size);
  };
  event_sum = ((sp_sample_t)(((event_count * (event_count + 1)) / 2)));
  sample_index = 0;
  while ((sample_index < size)) {
    expected_value = (event_sum * ((sp_sample_t)((sample_index + 1))));
    sample_value_ch0 = (block.samples)[0][sample_index];
    sample_value_ch1 = (block.samples)[1][sample_index];
    test_helper_assert("seq_parallel_block ch0", (sp_sample_nearly_equal(expected_value, sample_value_ch0, (0.001))));
    test_helper_assert("seq_parallel_block ch1", (sp_sample_nearly_equal(expected_value, sample_value_ch1, (0.001))));
    sample_index = (sample_index + 1);
  };
  sp_event_list_uninit((&events));
  sp_block_uninit((&block));
exit:
  status_return;
}
status_t test_sp_pan_to_amp(void) {
  status_declare;
  test_helper_assert("value 0, channel 0", (sp_sample_nearly_equal(1, (sp_pan_to_amp(0, 0)), error_margin)));
  test_helper_assert("value 0, channel 1", (sp_sample_nearly_equal(0, (sp_pan_to_amp(0, 1)), error_margin)));
  test_helper_assert("value 1, channel 0", (sp_sample_nearly_equal(0, (sp_pan_to_amp(1, 0)), error_margin)));
  test_helper_assert("value 1, channel 1", (sp_sample_nearly_equal(1, (sp_pan_to_amp(1, 1)), error_margin)));
  test_helper_assert(("value 0.5, channel 0"), (sp_sample_nearly_equal(1, (sp_pan_to_amp((0.5), 0)), error_margin)));
  test_helper_assert(("value 0.5, channel 0"), (sp_sample_nearly_equal(1, (sp_pan_to_amp((0.5), 1)), error_margin)));
  test_helper_assert(("value 0.25, channel 0"), (sp_sample_nearly_equal(1, (sp_pan_to_amp((0.25), 0)), error_margin)));
  test_helper_assert(("value 0.25, channel 1"), (sp_sample_nearly_equal((0.5), (sp_pan_to_amp((0.25), 1)), error_margin)));
exit:
  status_return;
}
status_t test_resonator_continuity_2(void) {
  status_declare;
  sp_sample_t* in;
  sp_sample_t* out;
  sp_sample_t* out_control;
  sp_convolution_filter_state_t* state;
  sp_convolution_filter_state_t* state_control;
  sp_time_t size;
  sp_time_t duration_a;
  sp_time_t duration_b;
  sp_sample_t cutoff_low;
  sp_sample_t cutoff_high;
  sp_sample_t transition;
  uint8_t arguments_buffer[(3 * sizeof(sp_sample_t))];
  uint8_t arguments_length;
  size = 100;
  duration_a = 30;
  duration_b = 70;
  state = 0;
  state_control = 0;
  cutoff_low = 0.1;
  cutoff_high = 0.3;
  transition = 0.1;
  status_require((sp_samples_new(size, (&in))));
  status_require((sp_samples_new(size, (&out))));
  status_require((sp_samples_new(size, (&out_control))));
  for (sp_size_t i = 0; (i < size); i += 1) {
    in[i] = ((sp_sample_t)(i));
  };
  arguments_length = (3 * sizeof(sp_sample_t));
  ((sp_sample_t*)(arguments_buffer))[0] = cutoff_low;
  ((sp_sample_t*)(arguments_buffer))[1] = cutoff_high;
  ((sp_sample_t*)(arguments_buffer))[2] = transition;
  status_require((sp_convolution_filter(in, size, sp_resonator_ir_f, arguments_buffer, arguments_length, (&state_control), out_control)));
  status_require((sp_convolution_filter(in, duration_a, sp_resonator_ir_f, arguments_buffer, arguments_length, (&state), out)));
  status_require((sp_convolution_filter((in + duration_a), duration_b, sp_resonator_ir_f, arguments_buffer, arguments_length, (&state), (out + duration_a))));
  test_helper_assert("resonator two-segment continuity", (sp_samples_nearly_equal(out, size, out_control, size, (0.01))));
  sp_convolution_filter_state_uninit(state);
  sp_convolution_filter_state_uninit(state_control);
  free(in);
  free(out);
  free(out_control);
exit:
  status_return;
}

/** "goto exit" can skip events */
int main(void) {
  status_declare;
  sp_initialize(3, 2, _sp_rate);
  test_helper_test_one(test_fft);
  test_helper_test_one(test_sinc_make_minimum_phase);
  test_helper_test_one(test_sp_resonator_event);
  test_helper_test_one(test_statistics);
  test_helper_test_one(test_sp_pan_to_amp);
  test_helper_test_one(test_base);
  test_helper_test_one(test_sp_random);
  test_helper_test_one(test_sp_triangle_square);
  test_helper_test_one(test_spectral_inversion_ir);
  test_helper_test_one(test_spectral_reversal_ir);
  test_helper_test_one(test_compositions);
  test_helper_test_one(test_times);
  test_helper_test_one(test_simple_mappings);
  test_helper_test_one(test_random_discrete);
  test_helper_test_one(test_file);
  test_helper_test_one(test_permutations);
  test_helper_test_one(test_render_range_block);
  /* resonator */
  test_helper_test_one(test_convolve_smaller);
  test_helper_test_one(test_convolve_larger);
  test_helper_test_one(test_resonator_ir_and_filter);
  test_helper_test_one(test_resonator_filter_continuity);
  test_helper_test_one(test_resonator_continuity_2);
  /* sequencer */
  test_helper_test_one(test_sp_group);
  test_helper_test_one(test_sp_seq);
  test_helper_test_one(test_sp_seq_parallel_block);
  test_helper_test_one(test_sp_map_event);
exit:
  sp_deinitialize();
  sp_test_helper_display_summary;
  return ((status.id));
}
