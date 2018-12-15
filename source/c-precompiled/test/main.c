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
  sp_sample_count_t source_len;
  sp_sample_count_t result_len;
  sp_sample_count_t prev_len;
  sp_sample_count_t next_len;
  sp_sample_count_t radius;
  sp_sample_t* result;
  sp_sample_t* source;
  sp_sample_t* prev;
  sp_sample_t* next;
  memreg_init(4);
  source_len = 8;
  result_len = source_len;
  prev_len = 4;
  next_len = prev_len;
  radius = 4;
  /* allocate memory */
  status_require((sph_helper_calloc((result_len * sizeof(sp_sample_t)), (&result))));
  memreg_add(result);
  status_require((sph_helper_calloc((source_len * sizeof(sp_sample_t)), (&source))));
  memreg_add(source);
  status_require((sph_helper_calloc((prev_len * sizeof(sp_sample_t)), (&prev))));
  memreg_add(prev);
  status_require((sph_helper_calloc((next_len * sizeof(sp_sample_t)), (&next))));
  memreg_add(next);
  /* set values */
  source[0] = 1;
  source[1] = 4;
  source[2] = 8;
  source[3] = 12;
  source[4] = 3;
  source[5] = 32;
  source[6] = 2;
  prev[0] = 3;
  prev[1] = 2;
  prev[2] = 1;
  prev[3] = -12;
  next[0] = 83;
  next[1] = 12;
  next[2] = -32;
  next[3] = 2;
  /* with prev and next */
  status_require((sp_moving_average(source, source_len, prev, prev_len, next, next_len, 0, (source_len - 1), radius, result)));
  /* without prev and next */
  status_require((sp_moving_average(source, source_len, 0, 0, 0, 0, 0, (source_len - 1), (1 + (source_len / 2)), result)));
exit:
  memreg_free;
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
  status_require((sp_windowed_sinc_bp_br_ir((0.1), (0.4), (0.08), 0, (&ir), (&ir_len))));
  status_require((sp_windowed_sinc_lp_hp_ir(cutoff, transition, 1, (&ir), (&ir_len))));
  status_require((sp_windowed_sinc_bp_br_ir(cutoff, cutoff, transition, 1, (&ir), (&ir_len))));
  /* filter functions */
  status_require((sp_windowed_sinc_lp_hp(source, 10, (0.1), (0.08), 0, (&state), result)));
  status_require((sp_windowed_sinc_lp_hp(source, 10, (0.1), (0.08), 1, (&state), result)));
  status_require((sp_windowed_sinc_bp_br(source, 10, (0.1), (0.4), (0.08), 0, (&state), result)));
  status_require((sp_windowed_sinc_bp_br(source, 10, (0.1), (0.4), (0.08), 1, (&state), result)));
  sp_convolution_filter_state_free(state);
exit:
  return (status);
};
status_t test_port() {
  status_declare;
  sp_sample_count_t channel;
  sp_channel_count_t channel_count;
  sp_sample_t** channel_data;
  sp_sample_t** channel_data_2;
  sp_sample_count_t len;
  sp_port_t port;
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
  status_require((sp_alloc_channel_array(channel_count, sample_count, (&channel_data))));
  memreg_add(channel_data);
  status_require((sp_alloc_channel_array(channel_count, sample_count, (&channel_data_2))));
  memreg_add(channel_data_2);
  while (channel) {
    channel = (channel - 1);
    len = sample_count;
    while (len) {
      len = (len - 1);
      (channel_data[channel])[len] = len;
    };
  };
  goto exit;
  /* test create */
  status_require((sp_file_open(test_file_path, sp_port_mode_read_write, channel_count, sample_rate, (&port))));
  printf("  create\n");
  status_require((sp_port_position((&port), (&position))));
  status_require((sp_port_write((&port), channel_data, sample_count, (&result_sample_count))));
  status_require((sp_port_position((&port), (&position))));
  test_helper_assert("sp-port-position file after write", (sample_count == position));
  status_require((sp_port_position_set((&port), 0)));
  status_require((sp_port_read((&port), sample_count, channel_data_2, (&result_sample_count))));
  /* compare read result with output data */
  len = channel_count;
  unequal = 0;
  while ((len && !unequal)) {
    len = (len - 1);
    unequal = !sp_sample_array_nearly_equal((channel_data[len]), sample_count, (channel_data_2[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-port-read new file result", !unequal);
  status_require((sp_port_close((&port))));
  printf("  write\n");
  /* test open */
  status_require((sp_file_open(test_file_path, sp_port_mode_read_write, 2, 8000, (&port))));
  status_require((sp_port_position((&port), (&position))));
  test_helper_assert("sp-port-position existing file", (sample_count == position));
  status_require((sp_port_position_set((&port), 0)));
  sp_port_read((&port), sample_count, channel_data_2, (&result_sample_count));
  /* compare read result with output data */
  unequal = 0;
  len = channel_count;
  while ((len && !unequal)) {
    len = (len - 1);
    unequal = !sp_sample_array_nearly_equal((channel_data[len]), sample_count, (channel_data_2[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-port-read existing result", !unequal);
  status_require((sp_port_close((&port))));
  printf("  open\n");
exit:
  memreg_free;
  return (status);
};
status_t test_fftr() {
  status_declare;
  sp_sample_t a[6] = { 0, 0.1, 0.4, 0.8, 0, 0 };
  sp_sample_count_t a_len;
  sp_sample_t a_again[6] = { 0, 0, 0, 0, 0, 0 };
  sp_sample_count_t a_again_len;
  sp_sample_count_t b_len;
  sp_sample_t b[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  a_len = 6;
  b_len = sp_fftr_output_len(a_len);
  a_again_len = sp_fftri_output_len(b_len);
  status_require((sp_fftr(a, a_len, b)));
  test_helper_assert("result length", (4 == b_len));
  test_helper_assert("result first bin", (1 < b[0]));
  test_helper_assert("result second bin", (-0.5 > b[2]));
  test_helper_assert("result third bin", (0 < b[4]));
  test_helper_assert("result fourth bin", (0 > b[6]));
  status_require((sp_fftri(b, b_len, a_again)));
  test_helper_assert("result 2 length", (a_len == a_again_len));
exit:
  return (status);
};
int main() {
  status_declare;
  if (!((sp_sample_format_f64 == sp_sample_format) || (sp_sample_format_f32 == sp_sample_format))) {
    printf("error: the tests only support f64 or f32 sample type");
    exit(1);
  };
  test_helper_test_one(test_fftr);
  test_helper_test_one(test_spectral_inversion_ir);
  test_helper_test_one(test_base);
  test_helper_test_one(test_spectral_reversal_ir);
  test_helper_test_one(test_convolve);
  test_helper_test_one(test_port);
  test_helper_test_one(test_moving_average);
  test_helper_test_one(test_windowed_sinc);
exit:
  test_helper_display_summary();
  return ((status.id));
};