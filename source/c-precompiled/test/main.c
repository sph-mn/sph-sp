#include "./helper.c"
#if (sp_sample_format_f64 == sp_sample_format)
#define sp_sample_nearly_equal f64_nearly_equal
#define sp_sample_array_nearly_equal f64_array_nearly_equal
#elif (sp_sample_format_f32 == sp_sample_format)
#define sp_sample_nearly_equal f32_nearly_equal
#define sp_sample_array_nearly_equal f32_array_nearly_equal
#endif
sp_sample_t error_margin = 0.1;
uint8_t* test_file_path = "/tmp/test-sph-sp-file";
status_t test_base() {
  status_declare;
  test_helper_assert(("input 0.5"), (sp_sample_nearly_equal((0.63662), (sp_sinc((0.5))), error_margin)));
  test_helper_assert("input 1", (sp_sample_nearly_equal((1.0), (sp_sinc(0)), error_margin)));
  test_helper_assert("window-blackman 0 51", (sp_sample_nearly_equal(0, (sp_window_blackman(0, 51)), error_margin)));
  test_helper_assert("window-blackman 25 51", (sp_sample_nearly_equal(1, (sp_window_blackman(25, 51)), error_margin)));
exit:
  return (status);
};
status_t test_spectral_inversion_ir() {
  status_declare;
  sp_sample_count_t a_len;
  sp_sample_t a[5] = { 0.1, -0.2, 0.3, -0.2, 0.1 };
  a_len = 5;
  sp_spectral_inversion_ir(a, a_len);
  test_helper_assert("result check", (sp_sample_nearly_equal((-0.1), (a[0]), error_margin) && sp_sample_nearly_equal((0.2), (a[1]), error_margin) && sp_sample_nearly_equal((0.7), (a[2]), error_margin) && sp_sample_nearly_equal((0.2), (a[3]), error_margin) && sp_sample_nearly_equal((-0.1), (a[4]), error_margin)));
exit:
  return (status);
};
status_t test_spectral_reversal_ir() {
  status_declare;
  sp_sample_count_t a_len;
  sp_sample_t a[5] = { 0.1, -0.2, 0.3, -0.2, 0.1 };
  a_len = 5;
  sp_spectral_reversal_ir(a, a_len);
  test_helper_assert("result check", (sp_sample_nearly_equal((0.1), (a[0]), error_margin) && sp_sample_nearly_equal((0.2), (a[1]), error_margin) && sp_sample_nearly_equal((0.3), (a[2]), error_margin) && sp_sample_nearly_equal((0.2), (a[3]), error_margin) && sp_sample_nearly_equal((0.1), (a[4]), error_margin)));
exit:
  return (status);
};
status_t test_convolve() {
  status_declare;
  sp_sample_t* a;
  sp_sample_count_t a_len;
  sp_sample_t* b;
  sp_sample_count_t b_len;
  sp_sample_t* carryover;
  sp_sample_count_t carryover_len;
  sp_sample_t* result;
  sp_sample_count_t result_len;
  sp_sample_count_t sample_count;
  sp_sample_t expected_result[5] = { 2, 7, 16, 22, 28 };
  sp_sample_t expected_carryover[3] = { 27, 18, 0 };
  memreg_init(4);
  sample_count = 5;
  b_len = 3;
  result_len = sample_count;
  a_len = sample_count;
  carryover_len = b_len;
  status_require((sph_helper_calloc((result_len * sizeof(sp_sample_t)), (&result))));
  memreg_add(result);
  status_require((sph_helper_calloc((a_len * sizeof(sp_sample_t)), (&a))));
  memreg_add(a);
  status_require((sph_helper_calloc((b_len * sizeof(sp_sample_t)), (&b))));
  memreg_add(b);
  status_require((sph_helper_calloc((carryover_len * sizeof(sp_sample_t)), (&carryover))));
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
  test_helper_assert("first result", (sp_sample_array_nearly_equal(result, result_len, expected_result, result_len, error_margin)));
  test_helper_assert("first result carryover", (sp_sample_array_nearly_equal(carryover, carryover_len, expected_carryover, carryover_len, error_margin)));
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
  test_helper_assert("second result", (sp_sample_array_nearly_equal(result, result_len, expected_result, result_len, error_margin)));
  test_helper_assert("second result carryover", (sp_sample_array_nearly_equal(carryover, carryover_len, expected_carryover, carryover_len, error_margin)));
exit:
  memreg_free;
  return (status);
};
status_t test_moving_average() {
  status_declare;
  sp_sample_count_t radius;
  sp_sample_t out[5] = { 0, 0, 0, 0, 0 };
  sp_sample_t in[5] = { 1, 3, 5, 7, 8 };
  sp_sample_t prev[5] = { 9, 10, 11 };
  sp_sample_t next[5] = { 12, 13, 14 };
  sp_sample_t* in_end;
  sp_sample_t* prev_end;
  sp_sample_t* next_end;
  sp_sample_t* in_window;
  sp_sample_t* in_window_end;
  prev_end = (prev + 2);
  next_end = (next + 2);
  in_end = (in + 4);
  in_window = (in + 1);
  in_window_end = (in + 3);
  radius = 3;
  status_require((sp_moving_average(in, in_end, in_window, in_window_end, prev, prev_end, next, next_end, radius, out)));
  /* first run with prev and next and only index 1 to 3 inclusively processed */
  test_helper_assert(("moving-average 1.1"), (sp_sample_nearly_equal((6.142857142857143), (out[0]), error_margin)));
  test_helper_assert(("moving-average 1.2"), (sp_sample_nearly_equal((6.571428571428571), (out[1]), error_margin)));
  test_helper_assert(("moving-average 1.2"), (sp_sample_nearly_equal(7, (out[2]), error_margin)));
  /* second run. result number series will be symmetric */
  out[0] = 0;
  out[1] = 0;
  out[2] = 0;
  out[3] = 0;
  out[4] = 0;
  in[0] = 2;
  in[1] = 2;
  in[2] = 2;
  in[3] = 2;
  in[4] = 2;
  status_require((sp_moving_average(in, in_end, in, in_end, 0, 0, 0, 0, 1, out)));
  test_helper_assert(("moving-average 2.1"), (sp_sample_nearly_equal((1.3), (out[0]), error_margin)));
  test_helper_assert(("moving-average 2.2"), (sp_sample_nearly_equal(2, (out[1]), error_margin)));
  test_helper_assert(("moving-average 2.3"), (sp_sample_nearly_equal(2, (out[2]), error_margin)));
  test_helper_assert(("moving-average 2.4"), (sp_sample_nearly_equal(2, (out[3]), error_margin)));
  test_helper_assert(("moving-average 2.5"), (sp_sample_nearly_equal((1.3), (out[4]), error_margin)));
exit:
  return (status);
};
status_t test_windowed_sinc() {
  status_declare;
  sp_float_t transition;
  sp_float_t cutoff;
  sp_sample_t* ir;
  sp_sample_count_t ir_len;
  sp_convolution_filter_state_t* state;
  sp_sample_t source[10] = { 3, 4, 5, 6, 7, 8, 9, 0, 1, 2 };
  sp_sample_t result[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  state = 0;
  cutoff = 0.1;
  transition = 0.08;
  /* ir functions */
  status_require((sp_windowed_sinc_lp_hp_ir(cutoff, transition, 0, (&ir), (&ir_len))));
  test_helper_assert("ir", (sp_sample_nearly_equal((0.0952), (ir[28]), error_margin)));
  status_require((sp_windowed_sinc_bp_br_ir((0.1), (0.4), (0.08), (0.08), 0, (&ir), (&ir_len))));
  status_require((sp_windowed_sinc_lp_hp_ir(cutoff, transition, 1, (&ir), (&ir_len))));
  status_require((sp_windowed_sinc_bp_br_ir(cutoff, cutoff, transition, transition, 1, (&ir), (&ir_len))));
  /* filter functions */
  status_require((sp_windowed_sinc_lp_hp(source, 10, (0.1), (0.08), 0, (&state), result)));
  sp_convolution_filter_state_free(state);
  state = 0;
  status_require((sp_windowed_sinc_lp_hp(source, 10, (0.1), (0.08), 1, (&state), result)));
  sp_convolution_filter_state_free(state);
  state = 0;
  status_require((sp_windowed_sinc_bp_br(source, 10, (0.1), (0.4), (0.08), (0.08), 0, (&state), result)));
  sp_convolution_filter_state_free(state);
  state = 0;
  status_require((sp_windowed_sinc_bp_br(source, 10, (0.1), (0.4), (0.08), (0.08), 1, (&state), result)));
  sp_convolution_filter_state_free(state);
exit:
  return (status);
};
status_t test_file() {
  status_declare;
  sp_sample_count_t channel;
  sp_channel_count_t channel_count;
  sp_sample_t** block;
  sp_sample_t** block_2;
  sp_sample_count_t len;
  sp_file_t file;
  sp_sample_count_t position;
  sp_sample_count_t sample_count;
  sp_sample_count_t result_sample_count;
  sp_sample_rate_t sample_rate;
  int8_t unequal;
  memreg_init(2);
  if (file_exists(test_file_path)) {
    unlink(test_file_path);
  };
  channel_count = 2;
  sample_rate = 8000;
  sample_count = 5;
  position = 0;
  channel = channel_count;
  status_require((sp_block_alloc(channel_count, sample_count, (&block))));
  memreg_add(block);
  status_require((sp_block_alloc(channel_count, sample_count, (&block_2))));
  memreg_add(block_2);
  while (channel) {
    channel = (channel - 1);
    len = sample_count;
    while (len) {
      len = (len - 1);
      (block[channel])[len] = len;
    };
  };
  goto exit;
  /* test create */
  status_require((sp_file_open(test_file_path, sp_file_mode_read_write, channel_count, sample_rate, (&file))));
  printf("  create\n");
  status_require((sp_file_position((&file), (&position))));
  status_require((sp_file_write((&file), block, sample_count, (&result_sample_count))));
  status_require((sp_file_position((&file), (&position))));
  test_helper_assert("sp-file-position file after write", (sample_count == position));
  status_require((sp_file_position_set((&file), 0)));
  status_require((sp_file_read((&file), sample_count, block_2, (&result_sample_count))));
  /* compare read result with output data */
  len = channel_count;
  unequal = 0;
  while ((len && !unequal)) {
    len = (len - 1);
    unequal = !sp_sample_array_nearly_equal((block[len]), sample_count, (block_2[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-file-read new file result", !unequal);
  status_require((sp_file_close((&file))));
  printf("  write\n");
  /* test open */
  status_require((sp_file_open(test_file_path, sp_file_mode_read_write, 2, 8000, (&file))));
  status_require((sp_file_position((&file), (&position))));
  test_helper_assert("sp-file-position existing file", (sample_count == position));
  status_require((sp_file_position_set((&file), 0)));
  sp_file_read((&file), sample_count, block_2, (&result_sample_count));
  /* compare read result with output data */
  unequal = 0;
  len = channel_count;
  while ((len && !unequal)) {
    len = (len - 1);
    unequal = !sp_sample_array_nearly_equal((block[len]), sample_count, (block_2[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-file-read existing result", !unequal);
  status_require((sp_file_close((&file))));
  printf("  open\n");
exit:
  memreg_free;
  return (status);
};
status_t test_fft() {
  status_declare;
  sp_sample_t a_real[6] = { -0.6, 0.1, 0.4, 0.8, 0, 0 };
  sp_sample_t a_imag[6] = { 0, 0, 0, 0, 0, 0 };
  sp_sample_count_t a_len;
  a_len = 6;
  status_require((sp_fft(a_len, a_real, a_imag)));
  status_require((sp_ffti(a_len, a_real, a_imag)));
exit:
  return (status);
};
status_t test_fm_synth() {
  status_declare;
  sp_sample_count_t* state;
  sp_sample_count_t config_len;
  sp_sample_t** out1;
  sp_sample_t** out2;
  sp_channel_count_t channels;
  sp_sample_count_t duration;
  sp_fm_synth_operator_t op1;
  sp_fm_synth_operator_t op2;
  sp_fm_synth_operator_t op3;
  sp_fm_synth_operator_t config[3];
  sp_sample_count_t wvl[4] = { 2, 2, 2, 2 };
  sp_sample_t amp[4] = { 0.1, 0.2, 0.3, 0.4 };
  state = 0;
  duration = 4;
  channels = 2;
  config_len = 3;
  op1.modifies = 0;
  op2.modifies = 1;
  op3.modifies = 0;
  (op1.amplitude)[0] = amp;
  (op1.amplitude)[1] = amp;
  (op1.wavelength)[0] = wvl;
  (op1.wavelength)[1] = wvl;
  (op1.phase_offset)[0] = 1;
  (op1.phase_offset)[2] = 2;
  (op2.amplitude)[0] = amp;
  (op2.amplitude)[1] = amp;
  (op2.wavelength)[0] = wvl;
  (op2.wavelength)[1] = wvl;
  (op2.phase_offset)[0] = 1;
  (op2.phase_offset)[2] = 2;
  (op3.amplitude)[0] = amp;
  (op3.amplitude)[1] = amp;
  (op3.wavelength)[0] = wvl;
  (op3.wavelength)[1] = wvl;
  (op3.phase_offset)[0] = 1;
  (op3.phase_offset)[2] = 2;
  config[0] = op1;
  config[1] = op2;
  config[2] = op3;
  status_require((sp_block_alloc(channels, duration, (&out1))));
  status_require((sp_block_alloc(channels, duration, (&out2))));
  status_require((sp_fm_synth(out1, channels, 0, duration, config_len, config, (&state))));
  status_require((sp_fm_synth(out2, channels, 0, duration, config_len, config, (&state))));
exit:
  return (status);
};
status_t test_sp_path() {
  status_declare;
  sp_path_t path;
  sp_path_segment_t segments[3];
  sp_path_segment_t s;
  sp_path_point_t p;
  sp_path_segment_count_t segments_len;
  sp_path_value_t out[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  p.x = 4;
  p.y = 8;
  *(s.points) = p;
  s.interpolator = sp_path_i_line;
  segments[0] = s;
  p.x = 8;
  p.y = 32;
  *(s.points) = p;
  s.interpolator = sp_path_i_line;
  segments[1] = s;
  segments_len = 2;
  status_require((sp_path_new(segments_len, segments, (&path))));
  sp_path_new(segments_len, segments, (&path));
  sp_path_get(path, 0, 9, out);
  printf("%f %f %f %f %f %f %f %f %f\n", (out[0]), (out[1]), (out[2]), (out[3]), (out[4]), (out[5]), (out[6]), (out[7]), (out[8]), (out[9]));
  sp_path_free(path);
exit:
  return (status);
};
int main() {
  status_declare;
  sp_initialise();
  test_helper_test_one(test_sp_path);
exit:
  test_helper_display_summary();
  return ((status.id));
};