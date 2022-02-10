
#include "./helper.c"
#if (sp_sample_format_f64 == sp_sample_format)
#define sp_sample_nearly_equal f64_nearly_equal
#define sp_samples_nearly_equal f64_array_nearly_equal
#elif (sp_sample_format_f32 == sp_sample_format)
#define sp_sample_nearly_equal f32_nearly_equal
#define sp_samples_nearly_equal f32_array_nearly_equal
#endif
#define _rate 960
sp_sample_t error_margin = 0.1;
uint8_t* test_file_path = "/tmp/test-sph-sp-file";
status_t test_base() {
  status_declare;
  test_helper_assert(("input 0.5"), (sp_sample_nearly_equal((0.63662), (sp_sinc((0.5))), error_margin)));
  test_helper_assert("input 1", (sp_sample_nearly_equal((1.0), (sp_sinc(0)), error_margin)));
  test_helper_assert("window-blackman 0 51", (sp_sample_nearly_equal(0, (sp_window_blackman(0, 51)), error_margin)));
  test_helper_assert("window-blackman 25 51", (sp_sample_nearly_equal(1, (sp_window_blackman(25, 51)), error_margin)));
exit:
  status_return;
}
status_t test_spectral_inversion_ir() {
  status_declare;
  sp_time_t a_len;
  sp_sample_t a[5] = { 0.1, -0.2, 0.3, -0.2, 0.1 };
  a_len = 5;
  sp_spectral_inversion_ir(a, a_len);
  test_helper_assert("result check", (sp_sample_nearly_equal((-0.1), (a[0]), error_margin) && sp_sample_nearly_equal((0.2), (a[1]), error_margin) && sp_sample_nearly_equal((0.7), (a[2]), error_margin) && sp_sample_nearly_equal((0.2), (a[3]), error_margin) && sp_sample_nearly_equal((-0.1), (a[4]), error_margin)));
exit:
  status_return;
}
status_t test_spectral_reversal_ir() {
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
status_t test_convolve_smaller() {
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
status_t test_convolve_larger() {
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
  memreg_init(4);
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
  for (sp_time_t i = 0; (i < in_a_length); i += 1) {
    in_a[i] = i;
  };
  for (sp_time_t i = 0; (i < in_b_length); i += 1) {
    in_b[i] = (1 + i);
  };
  sp_convolve(in_a, in_a_length, in_b, in_b_length, carryover_length, carryover, out_control);
  sp_samples_zero(carryover, in_b_length);
  for (sp_time_t i = 0; (i < block_count); i += 1) {
    sp_convolve(((i * block_size) + in_a), block_size, in_b, in_b_length, carryover_length, carryover, ((i * block_size) + out));
    carryover_length = in_b_length;
  };
  test_helper_assert("equal to block processing result", (sp_sample_array_nearly_equal(out, in_a_length, out_control, in_a_length, (0.01))));
exit:
  memreg_free;
  status_return;
}
status_t test_moving_average() {
  status_declare;
  sp_sample_t* in;
  sp_sample_t* out;
  sp_time_t radius;
  sp_time_t size;
  memreg_init(2);
  size = 11;
  radius = 4;
  status_require((sp_path_samples_2((&in), size, (sp_path_line(5, (10.0))), (sp_path_line((size - 1), 0)))));
  memreg_add(in);
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
status_t test_windowed_sinc_continuity() {
  status_declare;
  sp_sample_t* in;
  sp_sample_t* out;
  sp_sample_t* out_control;
  sp_convolution_filter_state_t* state;
  sp_convolution_filter_state_t* state_control;
  sp_random_state_t random_state;
  sp_time_t size;
  sp_time_t block_size;
  sp_time_t block_count;
  sp_sample_t cutl;
  sp_sample_t cuth;
  sp_sample_t trnl;
  sp_sample_t trnh;
  memreg_init(3);
  size = 100;
  block_size = 10;
  block_count = 10;
  state = 0;
  state_control = 0;
  cutl = 0.001;
  cuth = 0.03;
  trnl = 0.07;
  trnh = 0.07;
  status_require((sp_samples_new(size, (&in))));
  memreg_add(in);
  status_require((sp_samples_new(size, (&out))));
  memreg_add(out);
  status_require((sp_samples_new(size, (&out_control))));
  memreg_add(out_control);
  for (sp_time_t i = 0; (i < size); i += 1) {
    in[i] = i;
  };
  status_require((sp_windowed_sinc_bp_br(in, size, cutl, cuth, trnl, trnh, 0, (&state_control), out_control)));
  for (sp_time_t i = 0; (i < block_count); i += 1) {
    status_require((sp_windowed_sinc_bp_br(((i * block_size) + in), block_size, cutl, cuth, trnl, trnh, 0, (&state), ((i * block_size) + out))));
  };
  test_helper_assert("equal to block processing result", (sp_sample_array_nearly_equal(out, size, out_control, size, (0.01))));
exit:
  memreg_free;
  sp_convolution_filter_state_free(state);
  sp_convolution_filter_state_free(state_control);
  status_return;
}
status_t test_windowed_sinc() {
  status_declare;
  sp_sample_t transition;
  sp_sample_t cutoff;
  sp_sample_t* ir;
  sp_time_t ir_len;
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
  status_return;
}
status_t test_file() {
  status_declare;
  sp_block_t block_2;
  sp_block_t block;
  sp_channel_count_t channel_count;
  sp_time_t channel;
  sp_file_t file;
  sp_time_t len;
  sp_time_t position;
  sp_time_t result_sample_count;
  sp_time_t sample_count;
  sp_time_t sample_rate;
  int8_t unequal;
  if (file_exists(test_file_path)) {
    unlink(test_file_path);
  };
  channel_count = 2;
  sample_rate = 8000;
  sample_count = 5;
  position = 0;
  channel = channel_count;
  status_require((sp_block_new(channel_count, sample_count, (&block))));
  status_require((sp_block_new(channel_count, sample_count, (&block_2))));
  while (channel) {
    channel = (channel - 1);
    len = sample_count;
    while (len) {
      len = (len - 1);
      ((block.samples)[channel])[len] = len;
    };
  };
  /* test create */
  status_require((sp_file_open(test_file_path, sp_file_mode_read_write, channel_count, sample_rate, (&file))));
  printf("  create\n");
  status_require((sp_file_position((&file), (&position))));
  status_require((sp_file_write((&file), (block.samples), sample_count, (&result_sample_count))));
  status_require((sp_file_position((&file), (&position))));
  test_helper_assert("sp-file-position file after write", (sample_count == position));
  status_require((sp_file_position_set((&file), 0)));
  status_require((sp_file_read((&file), sample_count, (block_2.samples), (&result_sample_count))));
  /* compare read result with output data */
  len = channel_count;
  unequal = 0;
  while ((len && !unequal)) {
    len = (len - 1);
    unequal = !sp_samples_nearly_equal(((block.samples)[len]), sample_count, ((block_2.samples)[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-file-read new file result", !unequal);
  status_require((sp_file_close(file)));
  printf("  write\n");
  /* test open */
  status_require((sp_file_open(test_file_path, sp_file_mode_read_write, 2, 8000, (&file))));
  status_require((sp_file_position((&file), (&position))));
  test_helper_assert("sp-file-position existing file", (sample_count == position));
  status_require((sp_file_position_set((&file), 0)));
  sp_file_read((&file), sample_count, (block_2.samples), (&result_sample_count));
  /* compare read result with output data */
  unequal = 0;
  len = channel_count;
  while ((len && !unequal)) {
    len = (len - 1);
    unequal = !sp_samples_nearly_equal(((block.samples)[len]), sample_count, ((block_2.samples)[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-file-read existing result", !unequal);
  status_require((sp_file_close(file)));
  printf("  open\n");
exit:
  sp_block_free((&block));
  sp_block_free((&block_2));
  status_return;
}
status_t test_fft() {
  status_declare;
  sp_sample_t a_real[6] = { -0.6, 0.1, 0.4, 0.8, 0, 0 };
  sp_sample_t a_imag[6] = { 0, 0, 0, 0, 0, 0 };
  sp_time_t a_len;
  a_len = 6;
  status_i_require((sp_fft(a_len, a_real, a_imag)));
  status_i_require((sp_ffti(a_len, a_real, a_imag)));
exit:
  status_return;
}

/** better test separately as it opens gnuplot windows */
status_t test_sp_plot() {
  status_declare;
  sp_sample_t a[9] = { 0.1, -0.2, 0.1, -0.4, 0.3, -0.4, 0.2, -0.2, 0.1 };
  sp_plot_samples(a, 9);
  sp_plot_spectrum(a, 9);
exit:
  status_return;
}
status_t test_sp_triangle_square() {
  status_declare;
  sp_time_t i;
  sp_sample_t* out_t;
  sp_sample_t* out_s;
  sp_time_t size;
  size = 96000;
  status_require((sph_helper_calloc((size * sizeof(sp_sample_t*)), (&out_t))));
  status_require((sph_helper_calloc((size * sizeof(sp_sample_t*)), (&out_s))));
  for (i = 0; (i < size); i += 1) {
    out_t[i] = sp_triangle(i, (size / 2), (size / 2));
    out_s[i] = sp_square(i, size);
  };
  test_helper_assert("triangle 0", (0 == out_t[0]));
  test_helper_assert("triangle 1/2", (1 == out_t[48000]));
  test_helper_assert("triangle 1", (f64_nearly_equal(0, (out_t[95999]), error_margin)));
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
status_t test_sp_random() {
  status_declare;
  sp_random_state_t s;
  sp_sample_t out[20];
  s = sp_random_state_new(80);
  sp_samples_random((&s), 10, out);
  sp_samples_random((&s), 10, (10 + out));
  test_helper_assert("last value", (f64_nearly_equal((-0.553401), (out[19]), error_margin)));
exit:
  status_return;
}

#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)
#define test_noise_duration 960
status_t test_sp_noise_event() {
  status_declare;
  sp_block_t out;
  sp_sample_t cutl[test_noise_duration];
  sp_sample_t cuth[test_noise_duration];
  sp_sample_t trnl[test_noise_duration];
  sp_sample_t trnh[test_noise_duration];
  sp_sample_t amod[test_noise_duration];
  sp_time_t i;
  sp_noise_event_config_t* config;
  sp_declare_event(event);
  sp_declare_event_list(events);
  free_on_error_init(2);
  status_require((sp_noise_event_config_new((&config))));
  free_on_error1(config);
  status_require((sp_block_new(2, test_noise_duration, (&out))));
  free_on_error((&out), sp_block_free);
  for (sp_time_t i = 0; (i < test_noise_duration); i += 1) {
    cutl[i] = 0.01;
    cuth[i] = 0.3;
    amod[i] = 1.0;
  };
  (*config).cutl_mod = cutl;
  (*config).cuth_mod = cuth;
  (*config).amod = amod;
  (*config).amp = 1;
  (*config).channels = 2;
  (*config).trnh = 0.07;
  (*config).trnl = 0.07;
  event.start = 0;
  event.end = test_noise_duration;
  event.prepare = sp_noise_event_prepare;
  event.data = config;
  status_require((sp_event_list_add((&events), event)));
  status_require((sp_seq(0, test_noise_duration, out, (&events))));
  sp_sample_t sum;
  test_helper_assert(("in range -1..1"), (1.0 >= sp_samples_absolute_max(((out.samples)[0]), test_noise_duration)));
  sp_block_free((&out));
exit:
  if (status_is_failure) {
    free_on_error_free;
  };
  status_return;
}
status_t test_sp_cheap_filter() {
  status_declare;
  sp_cheap_filter_state_t state;
  sp_sample_t out[test_noise_duration];
  sp_sample_t in[test_noise_duration];
  sp_time_t i;
  sp_random_state_t s;
  s = sp_random_state_new(80);
  sp_samples_random((&s), test_noise_duration, in);
  status_require((sp_cheap_filter_state_new(test_noise_duration, sp_cheap_filter_passes_limit, (&state))));
  sp_cheap_filter_lp(in, test_noise_duration, (0.2), 1, 0, (&state), out);
  sp_cheap_filter_lp(in, test_noise_duration, (0.2), sp_cheap_filter_passes_limit, 0, (&state), out);
  sp_cheap_filter_lp(in, test_noise_duration, (0.2), sp_cheap_filter_passes_limit, 0, (&state), out);
  sp_cheap_filter_state_free((&state));
exit:
  status_return;
}
status_t test_sp_cheap_noise_event() {
  status_declare;
  sp_block_t out;
  sp_sample_t cut_mod[test_noise_duration];
  sp_sample_t amod[test_noise_duration];
  sp_time_t i;
  sp_cheap_noise_event_config_t* config;
  sp_declare_event(event);
  free_on_error_init(2);
  status_require((sp_cheap_noise_event_config_new((&config))));
  free_on_error1(config);
  status_require((sp_block_new(2, test_noise_duration, (&out))));
  free_on_error((&out), sp_block_free);
  for (i = 0; (i < test_noise_duration); i += 1) {
    cut_mod[i] = ((i < (test_noise_duration / 2)) ? 0.01 : 0.1);
    cut_mod[i] = 0.08;
    amod[i] = 1.0;
  };
  (*config).type = sp_state_variable_filter_lp;
  (*config).amp = 1;
  (*config).amod = amod;
  (*config).cut_mod = cut_mod;
  (*config).q_factor = 0.1;
  (*config).channels = 2;
  (*config).amp = 1;
  event.end = test_noise_duration;
  event.data = &config;
  event.prepare = sp_cheap_noise_event_prepare;
  status_require(((event.prepare)((&event))));
  status_require(((event.generate)(0, test_noise_duration, out, (&event))));
  test_helper_assert(("in range -1..1"), (1.0 >= sp_samples_absolute_max(((out.samples)[0]), test_noise_duration)));
  sp_block_free((&out));
exit:
  if (status_is_failure) {
    free_on_error_free;
  };
  status_return;
}
#define sp_seq_event_count 2
status_t test_sp_seq() {
  status_declare;
  sp_block_t out;
  sp_time_t i;
  sp_declare_event_list(events);
  status_require((sp_event_list_add((&events), (test_helper_event(0, 40, 1)))));
  status_require((sp_event_list_add((&events), (test_helper_event(41, 100, 2)))));
  status_require((sp_block_new(2, 100, (&out))));
  sp_seq_events_prepare((&events));
  sp_seq(0, 50, out, (&events));
  sp_seq(50, 100, (sp_block_with_offset(out, 50)), (&events));
  test_helper_assert("block contents 1 event 1", ((1 == (out.samples)[0][0]) && (1 == (out.samples)[0][39])));
  test_helper_assert("block contents 1 gap", (0 == (out.samples)[0][40]));
  test_helper_assert("block contents 1 event 2", ((2 == (out.samples)[0][41]) && (2 == (out.samples)[0][99])));
  sp_event_list_free((&events));
  sp_block_free((&out));
exit:
  status_return;
}
status_t test_sp_group() {
  status_declare;
  sp_block_t block;
  sp_time_t* m1;
  sp_time_t* m2;
  sp_declare_event_2(g, g1);
  sp_declare_event_3(e1, e2, e3);
  status_require((sp_times_new(100, (&m1))));
  status_require((sp_times_new(100, (&m2))));
  g.prepare = sp_group_prepare;
  g1.start = 10;
  g1.prepare = sp_group_prepare;
  status_require((sp_block_new(2, 100, (&block))));
  e1 = test_helper_event(0, 20, 1);
  e2 = test_helper_event(20, 40, 2);
  e3 = test_helper_event(50, 100, 3);
  status_require((sp_group_add((&g1), e1)));
  status_require((sp_group_add((&g1), e2)));
  status_require((sp_group_add((&g), g1)));
  status_require((sp_group_add((&g), e3)));
  status_require((sp_event_memory_init((&g), 2)));
  sp_event_memory_add((&g), m1);
  sp_event_memory_add((&g), m2);
  status_require(((g.prepare)((&g))));
  status_require(((g.generate)(0, 50, block, (&g))));
  status_require(((g.generate)(50, 100, (sp_block_with_offset(block, 50)), (&g))));
  (g.free)((&g));
  test_helper_assert("block contents event 1", ((1 == (block.samples)[0][10]) && (1 == (block.samples)[0][29])));
  test_helper_assert("block contents event 2", ((2 == (block.samples)[0][30]) && (2 == (block.samples)[0][39])));
  test_helper_assert("block contents gap", ((1 > (block.samples)[0][40]) && (1 > (block.samples)[0][49])));
  test_helper_assert("block contents event 3", ((3 == (block.samples)[0][50]) && (3 == (block.samples)[0][99])));
  sp_block_free((&block));
exit:
  status_return;
}
#define test_wave_event_duration 100

/** sp wave values were taken from printing index and value of the result array.
   sp-plot-samples can plot the result */
status_t test_sp_wave_event() {
  status_declare;
  sp_block_t out;
  sp_time_t fmod[test_wave_event_duration];
  sp_sample_t amod1[test_wave_event_duration];
  sp_sample_t amod2[test_wave_event_duration];
  sp_wave_event_config_t* config;
  sp_declare_event(event);
  free_on_error_init(2);
  status_require((sp_wave_event_config_new((&config))));
  free_on_error1(config);
  status_require((sp_block_new(2, test_wave_event_duration, (&out))));
  free_on_error((&out), sp_block_free);
  for (sp_time_t i = 0; (i < test_wave_event_duration); i += 1) {
    fmod[i] = 2000;
    amod1[i] = 1;
    amod2[i] = 0.5;
  };
  (*config).wvf = sp_sine_table;
  (*config).wvf_size = sp_rate;
  (*config).fmod = fmod;
  (*config).amp = 1;
  (*config).amod = amod1;
  (*config).channels = 2;
  (config->channel_config)[1] = sp_channel_config(0, 10, 10, 1, amod2);
  event.start = 0;
  event.end = test_wave_event_duration;
  event.data = config;
  event.prepare = sp_wave_event_prepare;
  status_require(((event.prepare)((&event))));
  status_require(((event.generate)(0, 30, out, (event.data))));
  status_require(((event.generate)(30, test_wave_event_duration, (sp_block_with_offset(out, 30)), (event.data))));
  /* (sp-plot-samples (array-get out.samples 0) test-wave-event-duration) */
  sp_block_free((&out));
exit:
  if (status_is_failure) {
    free_on_error_free;
  };
  status_return;
}
status_t test_render_block() {
  status_declare;
  sp_block_t out;
  sp_time_t frq[test_wave_event_duration];
  sp_sample_t amod[test_wave_event_duration];
  sp_time_t i;
  sp_render_config_t rc;
  sp_wave_event_config_t* config;
  sp_declare_event(event);
  free_on_error_init(2);
  rc = sp_render_config(sp_channels, sp_rate, sp_rate);
  rc.block_size = 40;
  for (i = 0; (i < test_wave_event_duration); i += 1) {
    frq[i] = 1500;
    amod[i] = 1;
  };
  status_require((sp_wave_event_config_new((&config))));
  free_on_error1(config);
  status_require((sp_block_new(1, test_wave_event_duration, (&out))));
  free_on_error((&out), sp_block_free);
  (*config).wvf = sp_sine_table;
  (*config).wvf_size = sp_rate;
  (*config).fmod = frq;
  (*config).amp = 1;
  (*config).amod = amod;
  (*config).channels = 1;
  event.start = 0;
  event.end = test_wave_event_duration;
  event.data = config;
  event.prepare = sp_wave_event_prepare;
  /* (sp-render-file event test-wave-event-duration rc /tmp/test.wav) */
  sp_render_block(event, 0, test_wave_event_duration, rc, (&out));
  /* (sp-block-plot-1 out) */
  sp_block_free((&out));
exit:
  if (status_is_failure) {
    free_on_error_free;
  };
  status_return;
}
status_t test_path() {
  sp_sample_t* samples;
  sp_time_t* times;
  status_declare;
  status_require((sp_path_samples_2((&samples), 100, (sp_path_line(10, 1)), (sp_path_line(100, 0)))));
  status_require((sp_path_times_2((&times), 100, (sp_path_line(10, 1)), (sp_path_line(100, 0)))));
exit:
  status_return;
}
#define feq(a, b) sp_sample_nearly_equal(a, b, (0.01))
sp_random_state_t rs;
uint8_t u64_from_array_test(sp_time_t size) {
  uint64_t bits_in;
  uint64_t bits_out;
  bits_in = 9838263505978427528u;
  bits_out = sp_u64_from_array(((uint8_t*)(&bits_in)), size);
  return ((0 == memcmp(((uint8_t*)(&bits_in)), ((uint8_t*)(&bits_out)), size)));
}
status_t test_times() {
  status_declare;
  sp_time_t size;
  sp_time_t* a_temp;
  sp_time_t a[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  sp_time_t b[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  sp_time_t b_size;
  sp_time_t* bits;
  sp_random_state_t s;
  a_temp = 0;
  size = 8;
  s = sp_random_state_new(123);
  sp_times_multiplications(1, 3, size, a);
  test_helper_assert("multiplications", (81 == a[4]));
  sp_times_additions(1, 3, size, a);
  test_helper_assert("additions", (13 == a[4]));
  sp_time_t indices[3] = { 1, 2, 4 };
  sp_times_select(a, indices, 3, a);
  test_helper_assert("select", ((4 == a[0]) && (7 == a[1]) && (13 == a[2])));
  sp_times_set_1(a, size, 1039, a);
  status_require((sp_times_new((8 * sizeof(sp_time_t)), (&bits))));
  sp_times_bits_to_times(a, (8 * sizeof(sp_time_t)), bits);
  test_helper_assert(("bits->times"), ((1 == bits[0]) && (1 == bits[3]) && (0 == bits[4]) && (1 == bits[10])));
  free(bits);
  sp_times_multiplications(1, 3, size, a);
  sp_times_shuffle((&s), a, size);
  s = sp_random_state_new(12);
  sp_times_random_binary((&s), size, a);
  sp_times_multiplications(1, 3, size, a);
  s = sp_random_state_new(113);
  sp_times_select_random((&s), a, size, b, (&b_size));
exit:
  free(a_temp);
  status_return;
}
#define test_stats_a_size 8
status_t test_statistics() {
  status_declare;
  sp_time_t size;
  sp_time_t a[test_stats_a_size] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  sp_sample_t as[test_stats_a_size] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  sp_time_t repetition_1[test_stats_a_size] = { 1, 1, 1, 1, 1, 1, 1, 1 };
  sp_time_t repetition_2[test_stats_a_size] = { 1, 2, 3, 4, 5, 6, 7, 8 };
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
  /* repetition-all */
  sp_stat_times_repetition_all(repetition_1, size, (&stat_out));
  test_helper_assert("repetition-1 all", (feq((28.0), stat_out)));
  test_helper_assert("repetition-1 all max", (feq((sp_stat_repetition_all_max(size)), stat_out)));
  sp_stat_times_repetition(repetition_1, size, 2, (&stat_out));
  test_helper_assert("repetition-1", (feq(6, stat_out)));
  test_helper_assert("repetition-1 max", (feq((sp_stat_repetition_max(size, 2)), stat_out)));
  sp_stat_times_repetition_all(repetition_2, size, (&stat_out));
  test_helper_assert("repetition-2 all", (feq((0.0), stat_out)));
  sp_stat_samples_repetition_all(as, size, (&stat_out));
  test_helper_assert("samples repetition-all", (feq((0.0), stat_out)));
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
status_t test_simple_mappings() {
  status_declare;
  sp_time_t size;
  sp_time_t a[4] = { 1, 1, 1, 1 };
  sp_time_t b[4] = { 2, 2, 2, 2 };
  sp_sample_t as[4] = { 1, 1, 1, 1 };
  sp_sample_t bs[4] = { 2, 2, 2, 2 };
  size = 4;
  /* times */
  sp_times_set_1(a, size, 0, a);
  sp_samples_set_1(as, size, 0, as);
  test_helper_assert("times set-1", (sp_times_equal_1(a, size, 0)));
  test_helper_assert("samples set-1", (sp_samples_equal_1(as, size, 0)));
  sp_times_add_1(a, size, 1, a);
  test_helper_assert("add-1", (sp_times_equal_1(a, size, 1)));
  sp_times_subtract_1(a, size, 10, a);
  test_helper_assert("subtract-1", (sp_times_equal_1(a, size, 0)));
  sp_times_add_1(a, size, 4, a);
  sp_times_multiply_1(a, size, 2, a);
  test_helper_assert("multiply-1", (sp_times_equal_1(a, size, 8)));
  sp_times_divide_1(a, size, 2, a);
  test_helper_assert("divide-1", (sp_times_equal_1(a, size, 4)));
  sp_times_set_1(a, size, 4, a);
  sp_times_add(a, size, b, a);
  test_helper_assert("add", (sp_times_equal_1(a, size, 6)));
  sp_times_set_1(a, size, 4, a);
  sp_times_subtract(a, size, b, a);
  test_helper_assert("subtract", (sp_times_equal_1(a, size, 2)));
  sp_times_set_1(a, size, 4, a);
  sp_times_multiply(a, size, b, a);
  test_helper_assert("multiply", (sp_times_equal_1(a, size, 8)));
  sp_times_set_1(a, size, 4, a);
  sp_times_divide(a, size, b, a);
  test_helper_assert("divide", (sp_times_equal_1(a, size, 2)));
  sp_times_set_1(a, size, 1, a);
exit:
  status_return;
}
status_t test_random_discrete() {
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
  sp_times_random_discrete((&rs), cudist, cudist_size, 8, a);
  for (i = 1; (i < size); i = (1 + i)) {
    test_helper_assert("random-discrete", ((1 == a[i]) || (3 == a[i])));
  };
exit:
  status_return;
}
status_t test_compositions() {
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
status_t test_permutations() {
  sp_time_t in[3] = { 1, 2, 3 };
  sp_time_t size;
  sp_time_t out_size;
  sp_time_t** out;
  sp_time_t i;
  sp_time_t* b;
  status_declare;
  size = 3;
  status_require((sp_times_permutations(size, in, size, (&out), (&out_size))));
  for (i = 0; (i < out_size); i += 1) {
    b = out[i];
    free(b);
  };
  free(out);
exit:
  status_return;
}

/** sum 10 wave events */
status_t test_sp_seq_parallel() {
  status_declare;
  sp_time_t i;
  sp_time_t step_size;
  sp_sample_t* amod;
  sp_time_t* fmod;
  sp_time_t size;
  sp_block_t block;
  sp_wave_event_config_t* config;
  sp_declare_event(event);
  sp_declare_event_list(events);
  size = 10000;
  free_on_error_init(4);
  status_require((sp_wave_event_config_new((&config))));
  free_on_error1(config);
  status_require((sp_path_samples_2((&amod), size, (sp_path_move(0, (1.0))), (sp_path_constant()))));
  free_on_error1(amod);
  status_require((sp_path_times_2((&fmod), size, (sp_path_move(0, 250)), (sp_path_constant()))));
  free_on_error1(fmod);
  (*config).wvf = sp_sine_table;
  (*config).wvf_size = sp_rate;
  (*config).fmod = fmod;
  (*config).amp = 1;
  (*config).amod = amod;
  (*config).channels = 2;
  for (i = 0; (i < 10); i += 1) {
    event.start = 0;
    event.end = size;
    event.data = config;
    event.prepare = sp_wave_event_prepare;
    status_require((sp_event_list_add((&events), event)));
  };
  status_require((sp_block_new(2, size, (&block))));
  sp_seq_events_prepare((&events));
  step_size = (size / 10);
  for (i = 0; (i < size); i += step_size) {
    sp_seq_parallel(i, (i + step_size), (sp_block_with_offset(block, i)), (&events));
  };
  test_helper_assert("first 1", (sp_sample_nearly_equal(0, ((block.samples)[0][0]), (0.001)) && sp_sample_nearly_equal(0, ((block.samples)[1][0]), (0.001))));
  test_helper_assert("last 1", (sp_sample_nearly_equal((8.314696), ((block.samples)[0][(step_size - 1)]), (0.001)) && sp_sample_nearly_equal((8.314696), ((block.samples)[1][(step_size - 1)]), (0.001))));
  test_helper_assert("first 2", (sp_sample_nearly_equal((5.0), ((block.samples)[0][step_size]), (0.001)) && sp_sample_nearly_equal((5.0), ((block.samples)[1][step_size]), (0.001))));
  test_helper_assert("last 2", (sp_sample_nearly_equal((-4.422887), ((block.samples)[0][((2 * step_size) - 1)]), (0.001)) && sp_sample_nearly_equal((-4.422887), ((block.samples)[1][((2 * step_size) - 1)]), (0.001))));
  sp_event_list_free((&events));
  free(amod);
  free(fmod);
  sp_block_free((&block));
exit:
  if (status_is_failure) {
    free_on_error_free;
  };
  status_return;
}
status_t test_sp_map_event_generate(sp_time_t start, sp_time_t end, sp_block_t in, sp_block_t out, void* state) {
  status_declare;
  sp_block_copy(in, out);
  status_return;
}
status_t test_sp_map_event() {
  status_declare;
  sp_time_t size;
  sp_block_t block;
  sp_sample_t* amod;
  sp_wave_event_config_t* config;
  sp_map_event_config_t* map_event_config;
  sp_declare_event_2(parent, child);
  free_on_error_init(2);
  status_require((sp_wave_event_config_new((&config))));
  free_on_error1(config);
  status_require((sp_map_event_config_new((&map_event_config))));
  free_on_error1(map_event_config);
  size = (10 * _rate);
  status_require((sp_path_samples_2((&amod), size, (sp_path_move(0, (1.0))), (sp_path_constant()))));
  (*config).wvf = sp_sine_table;
  (*config).wvf_size = sp_rate;
  (*config).frq = 300;
  (*config).fmod = 0;
  (*config).amp = 1;
  (*config).amod = amod;
  (*config).channels = 1;
  child.start = 0;
  child.end = size;
  child.data = config;
  child.prepare = sp_wave_event_prepare;
  status_require((sp_block_new(1, size, (&block))));
  (*map_event_config).event = child;
  (*map_event_config).map_generate = test_sp_map_event_generate;
  (*map_event_config).isolate = 1;
  parent.start = child.start;
  parent.end = child.end;
  parent.prepare = sp_map_event_prepare;
  parent.data = map_event_config;
  status_require(((parent.prepare)((&parent))));
  status_require(((parent.generate)(0, (size / 2), block, (&parent))));
  status_require(((parent.generate)((size / 2), size, block, (&parent))));
  (parent.free)((&parent));
  sp_block_free((&block));
  free(amod);
exit:
  if (status_is_failure) {
    free_on_error_free;
  };
  status_return;
}

/** "goto exit" can skip events */
int main() {
  status_declare;
  rs = sp_random_state_new(3);
  sp_initialize(3, 2, _rate);
  test_helper_test_one(test_windowed_sinc_continuity);
  test_helper_test_one(test_convolve_smaller);
  test_helper_test_one(test_convolve_larger);
  test_helper_test_one(test_sp_noise_event);
  test_helper_test_one(test_sp_wave_event);
  test_helper_test_one(test_sp_cheap_noise_event);
  test_helper_test_one(test_sp_map_event);
  test_helper_test_one(test_sp_group);
  test_helper_test_one(test_sp_seq);
  test_helper_test_one(test_render_block);
  test_helper_test_one(test_moving_average);
  test_helper_test_one(test_statistics);
  test_helper_test_one(test_path);
  test_helper_test_one(test_file);
  test_helper_test_one(test_sp_cheap_filter);
  test_helper_test_one(test_sp_random);
  test_helper_test_one(test_sp_triangle_square);
  test_helper_test_one(test_fft);
  test_helper_test_one(test_spectral_inversion_ir);
  test_helper_test_one(test_base);
  test_helper_test_one(test_spectral_reversal_ir);
  test_helper_test_one(test_windowed_sinc);
  test_helper_test_one(test_times);
  test_helper_test_one(test_permutations);
  test_helper_test_one(test_compositions);
  test_helper_test_one(test_simple_mappings);
  test_helper_test_one(test_random_discrete);
  test_helper_test_one(test_sp_seq_parallel);
exit:
  test_helper_display_summary();
  return ((status.id));
}
