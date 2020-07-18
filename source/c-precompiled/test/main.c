#include "./helper.c"
#if (sp_sample_format_f64 == sp_sample_format)
#define sp_sample_nearly_equal f64_nearly_equal
#define sp_samples_nearly_equal f64_array_nearly_equal
#elif (sp_sample_format_f32 == sp_sample_format)
#define sp_sample_nearly_equal f32_nearly_equal
#define sp_samples_nearly_equal f32_array_nearly_equal
#endif
#define _rate 96000
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
status_t test_convolve() {
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
  /* sp-moving-average-centered */
  sp_moving_average_centered(in, size, radius, out);
  test_helper_assert("moving-average-centered", (sp_sample_nearly_equal((2.0), (out[1]), error_margin) && sp_sample_nearly_equal((5.5), (out[5]), error_margin) && sp_sample_nearly_equal((2.0), (out[9]), error_margin)));
exit:
  memreg_free;
  status_return;
}
status_t test_windowed_sinc() {
  status_declare;
  sp_float_t transition;
  sp_float_t cutoff;
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
  sp_channels_t channel_count;
  sp_time_t channel;
  sp_file_t file;
  sp_time_t len;
  sp_time_t position;
  sp_time_t result_sample_count;
  sp_time_t sample_count;
  sp_sample_rate_t sample_rate;
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
  sp_block_free(block);
  sp_block_free(block_2);
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
  status_require((sph_helper_calloc((_rate * sizeof(sp_sample_t*)), (&out_t))));
  status_require((sph_helper_calloc((_rate * sizeof(sp_sample_t*)), (&out_s))));
  for (i = 0; (i < _rate); i = (1 + i)) {
    out_t[i] = sp_triangle(i, (_rate / 2), (_rate / 2));
    out_s[i] = sp_square(i, _rate);
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
  test_helper_assert("last value", (f64_nearly_equal((0.6778), (out[19]), error_margin)));
exit:
  status_return;
}
#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)
#define sp_noise_duration 96
status_t test_sp_noise_event() {
  status_declare;
  sp_event_t events[1];
  sp_time_t events_size;
  sp_block_t out;
  sp_sample_t cut_l[sp_noise_duration];
  sp_sample_t cut_h[sp_noise_duration];
  sp_sample_t trn_l[sp_noise_duration];
  sp_sample_t trn_h[sp_noise_duration];
  sp_sample_t amp1[sp_noise_duration];
  sp_sample_t* amp[sp_channel_limit];
  sp_time_t i;
  status_require((sp_block_new(1, sp_noise_duration, (&out))));
  amp[0] = amp1;
  for (i = 0; (i < sp_noise_duration); i = (1 + i)) {
    cut_l[i] = ((i < (sp_noise_duration / 2)) ? 0.01 : 0.1);
    cut_h[i] = 0.11;
    trn_l[i] = 0.07;
    trn_h[i] = 0.07;
    amp1[i] = 1.0;
  };
  status_require((sp_noise_event(0, sp_noise_duration, amp, cut_l, cut_h, trn_l, trn_h, 0, 30, sp_default_random_state, events)));
  events_size = 1;
  sp_seq(0, sp_noise_duration, out, events, events_size);
  sp_events_array_free(events, events_size);
  sp_block_free(out);
exit:
  status_return;
}
status_t test_sp_cheap_filter() {
  status_declare;
  sp_cheap_filter_state_t state;
  sp_sample_t out[sp_noise_duration];
  sp_sample_t in[sp_noise_duration];
  sp_time_t i;
  sp_random_state_t s;
  s = sp_random_state_new(80);
  sp_samples_random((&s), sp_noise_duration, in);
  status_require((sp_cheap_filter_state_new(sp_noise_duration, sp_cheap_filter_passes_limit, (&state))));
  sp_cheap_filter_lp(in, sp_noise_duration, (0.2), 1, 0, 1, (&state), out);
  sp_cheap_filter_lp(in, sp_noise_duration, (0.2), sp_cheap_filter_passes_limit, 0, 1, (&state), out);
  sp_cheap_filter_lp(in, sp_noise_duration, (0.2), sp_cheap_filter_passes_limit, 0, 1, (&state), out);
  sp_cheap_filter_state_free((&state));
exit:
  status_return;
}
status_t test_sp_cheap_noise_event() {
  status_declare;
  sp_event_t events[1];
  sp_time_t events_size;
  sp_block_t out;
  sp_sample_t cut[sp_noise_duration];
  sp_sample_t amp1[sp_noise_duration];
  sp_sample_t* amp[sp_channel_limit];
  sp_sample_t q_factor;
  sp_time_t i;
  status_require((sp_block_new(1, sp_noise_duration, (&out))));
  amp[0] = amp1;
  q_factor = 0;
  for (i = 0; (i < sp_noise_duration); i = (1 + i)) {
    cut[i] = ((i < (sp_noise_duration / 2)) ? 0.01 : 0.1);
    cut[i] = 0.08;
    amp1[i] = 1.0;
  };
  status_require((sp_cheap_noise_event_lp(0, sp_noise_duration, amp, cut, 1, 0, 0, sp_default_random_state, events)));
  events_size = 1;
  sp_seq(0, sp_noise_duration, out, events, events_size);
  sp_events_array_free(events, events_size);
  sp_block_free(out);
exit:
  status_return;
}
#define sp_seq_event_count 2
status_t test_sp_seq() {
  status_declare;
  sp_event_t events[sp_seq_event_count];
  sp_block_t out;
  sp_time_t i;
  events[0] = test_helper_event(0, 40, 1);
  events[1] = test_helper_event(41, 100, 2);
  sp_seq_events_prepare(events, sp_seq_event_count);
  status_require((sp_block_new(2, 100, (&out))));
  sp_seq(0, 50, out, events, sp_seq_event_count);
  sp_seq(50, 100, (sp_block_with_offset(out, 50)), events, sp_seq_event_count);
  test_helper_assert("block contents 1 event 1", ((1 == (out.samples)[0][0]) && (1 == (out.samples)[0][39])));
  test_helper_assert("block contents 1 gap", (0 == (out.samples)[0][40]));
  test_helper_assert("block contents 1 event 2", ((2 == (out.samples)[0][41]) && (2 == (out.samples)[0][99])));
  /* sp-seq-parallel */
  status_require((sp_seq_parallel(0, 100, out, events, sp_seq_event_count)));
  sp_events_array_free(events, sp_seq_event_count);
  sp_block_free(out);
exit:
  status_return;
}
status_t test_sp_group() {
  status_declare;
  sp_event_t g;
  sp_event_t g1;
  sp_event_t e1;
  sp_event_t e2;
  sp_event_t e3;
  sp_block_t block;
  sp_time_t* m1;
  sp_time_t* m2;
  status_require((sp_times_new(100, (&m1))));
  status_require((sp_times_new(100, (&m2))));
  status_require((sp_group_new(0, 2, 2, (&g))));
  status_require((sp_group_new(10, 2, 0, (&g1))));
  status_require((sp_block_new(2, 100, (&block))));
  e1 = test_helper_event(0, 20, 1);
  e2 = test_helper_event(20, 40, 2);
  e3 = test_helper_event(50, 100, 3);
  sp_group_add(g1, e1);
  sp_group_add(g1, e2);
  sp_group_add(g, g1);
  sp_group_add(g, e3);
  sp_group_memory_add(g, m1);
  sp_group_memory_add(g, m2);
  sp_group_prepare(g);
  (g.f)(0, 50, block, (&g));
  (g.f)(50, 100, (sp_block_with_offset(block, 50)), (&g));
  (g.free)((&g));
  test_helper_assert("block contents event 1", ((1 == (block.samples)[0][10]) && (1 == (block.samples)[0][29])));
  test_helper_assert("block contents event 2", ((2 == (block.samples)[0][30]) && (2 == (block.samples)[0][39])));
  test_helper_assert("block contents gap", ((1 > (block.samples)[0][40]) && (1 > (block.samples)[0][49])));
  test_helper_assert("block contents event 3", ((3 == (block.samples)[0][50]) && (3 == (block.samples)[0][99])));
  sp_block_free(block);
exit:
  status_return;
}
#define test_wave_duration 4
#define test_wave_channels 2
status_t test_wave() {
  status_declare;
  sp_wave_state_t state;
  sp_block_t out1;
  sp_block_t out2;
  sp_time_t frq[4] = { 48000, 48000, 48000, 48000 };
  sp_sample_t amp[4] = { 0.1, 0.2, 0.3, 0.4 };
  status_require((sp_block_new(test_wave_channels, test_wave_duration, (&out1))));
  status_require((sp_block_new(test_wave_channels, test_wave_duration, (&out2))));
  state = sp_wave_state_2(sp_sine_table, _rate, test_wave_duration, frq, amp, amp, 0, 0);
  sp_wave(0, test_wave_duration, (&state), out1);
  sp_wave(0, test_wave_duration, (&state), out2);
  test_helper_assert("zeros", ((0 == (out1.samples)[0][0]) && ((out1.samples)[0][0] == (out1.samples)[0][2])));
  test_helper_assert("non-zeros", (!(0 == (out1.samples)[0][1]) && !(0 == (out1.samples)[0][3])));
  sp_block_free(out1);
  sp_block_free(out2);
exit:
  status_return;
}
#define sp_wave_event_duration 100
/** sp wave values were taken from printing index and value of the result array.
   sp-plot-samples can plot the result */
status_t test_wave_event() {
  status_declare;
  sp_event_t event;
  sp_block_t out;
  sp_time_t frq[sp_wave_event_duration];
  sp_sample_t amp1[sp_wave_event_duration];
  sp_sample_t amp2[sp_wave_event_duration];
  sp_time_t i;
  for (i = 0; (i < sp_wave_event_duration); i += 1) {
    frq[i] = 2000;
    amp1[i] = 1;
    amp2[i] = 0.5;
  };
  status_require((sp_wave_event(0, sp_wave_event_duration, (sp_wave_state_2(sp_sine_table, _rate, sp_wave_event_duration, frq, amp1, amp2, 0, 0)), (&event))));
  status_require((sp_block_new(2, sp_wave_event_duration, (&out))));
  (event.f)(0, 30, out, (&event));
  (event.f)(30, sp_wave_event_duration, (sp_block_with_offset(out, 30)), (&event));
  sp_block_free(out);
  (event.free)((&event));
exit:
  status_return;
}
status_t test_render_block() {
  status_declare;
  sp_event_t event;
  sp_block_t out;
  sp_time_t frq[sp_wave_event_duration];
  sp_sample_t amp[sp_wave_event_duration];
  sp_time_t i;
  sp_render_config_declare(rc);
  rc.block_size = 40;
  for (i = 0; (i < sp_wave_event_duration); i += 1) {
    frq[i] = 1500;
    amp[i] = 1;
  };
  status_require((sp_wave_event(0, sp_wave_event_duration, (sp_sine_state_2(sp_wave_event_duration, frq, amp, amp, 0, 0)), (&event))));
  status_require((sp_block_new(2, sp_wave_event_duration, (&out))));
  sp_render_file(event, 0, sp_wave_event_duration, rc, ("/tmp/test.wav"));
  /* (sp-render-block event 0 sp-wave-event-duration rc &out) */
  /* (sp-block-plot-1 out) */
  sp_block_free(out);
  (event.free)((&event));
exit:
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
#define sp_sample_nearly_equal f64_nearly_equal
#define sp_sample_array_nearly_equal f64_array_nearly_equal
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
  sp_times_extract_at_indices(a, indices, 3, a);
  test_helper_assert("extract-indices", ((4 == a[0]) && (7 == a[1]) && (13 == a[2])));
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
  sp_times_extract_random((&s), a, size, b, (&b_size));
exit:
  free(a_temp);
  status_return;
}
#define test_stats_a_size 8
#define test_stats_types_count (sp_stat_types_count - 3)
status_t test_stats() {
  status_declare;
  sp_time_t a[test_stats_a_size] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  sp_sample_t as[test_stats_a_size] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  sp_time_t inhar_1[test_stats_a_size] = { 2, 4, 6, 8, 10, 12, 14, 16 };
  sp_time_t inhar_2[test_stats_a_size] = { 2, 4, 6, 8, 10, 12, 13, 16 };
  sp_time_t inhar_3[test_stats_a_size] = { 2, 3, 6, 8, 10, 12, 13, 16 };
  sp_sample_t inhar_results[3];
  sp_stat_type_t stat_types[test_stats_types_count] = { sp_stat_center, sp_stat_complexity, sp_stat_deviation, sp_stat_inharmonicity, sp_stat_kurtosis, sp_stat_mean, sp_stat_median, sp_stat_range, sp_stat_skewness };
  sp_sample_t stats_a[sp_stat_types_count];
  sp_sample_t stats_as[sp_stat_types_count];
  status_require((sp_stat_times(a, test_stats_a_size, stat_types, test_stats_types_count, stats_a)));
  test_helper_assert("mean", (feq((4.5), (stats_a[sp_stat_mean]))));
  test_helper_assert("deviation", (feq((2.29), (stats_a[sp_stat_deviation]))));
  test_helper_assert("center", (feq((4.54), (stats_a[sp_stat_center]))));
  test_helper_assert("median", (feq((4.5), (stats_a[sp_stat_median]))));
  test_helper_assert("complexity", (feq((1.0), (stats_a[sp_stat_complexity_width]))));
  test_helper_assert("skewness", (feq((0.0), (stats_a[sp_stat_skewness]))));
  test_helper_assert("kurtosis", (feq((1.76), (stats_a[sp_stat_kurtosis]))));
  status_require((sp_stat_samples(as, test_stats_a_size, stat_types, test_stats_types_count, stats_as)));
  test_helper_assert("samples mean", (feq((4.5), (stats_as[sp_stat_mean]))));
  test_helper_assert("samples deviation", (feq((2.29), (stats_as[sp_stat_deviation]))));
  test_helper_assert("samples center", (feq((4.66), (stats_as[sp_stat_center]))));
  test_helper_assert("samples median", (feq((4.5), (stats_as[sp_stat_median]))));
  test_helper_assert("samples complexity", (feq((1.0), (stats_as[sp_stat_complexity_width]))));
  test_helper_assert("samples skewness", (feq((0.0), (stats_as[sp_stat_skewness]))));
  test_helper_assert("samples kurtosis", (feq((1.76), (stats_as[sp_stat_kurtosis]))));
  /* inharmonicity */
  stat_types[0] = sp_stat_inharmonicity;
  status_require((sp_stat_times(inhar_1, test_stats_a_size, stat_types, 1, stats_a)));
  inhar_results[0] = stats_a[sp_stat_inharmonicity];
  status_require((sp_stat_times(inhar_2, test_stats_a_size, stat_types, 1, stats_a)));
  inhar_results[1] = stats_a[sp_stat_inharmonicity];
  status_require((sp_stat_times(inhar_3, test_stats_a_size, stat_types, 1, stats_a)));
  inhar_results[2] = stats_a[sp_stat_inharmonicity];
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
status_t test_sp_seq_parallel() {
  sp_time_t i;
  sp_time_t step_size;
  sp_sample_t* amp;
  sp_time_t* frq;
  sp_time_t size;
  sp_block_t block;
  sp_event_t events[10];
  status_declare;
  size = (10 * _rate);
  status_require((sp_path_samples_2((&amp), size, (sp_path_move(0, (1.0))), (sp_path_constant()))));
  status_require((sp_path_times_2((&frq), size, (sp_path_move(0, 200)), (sp_path_constant()))));
  for (i = 0; (i < 10); i += 1) {
    status_require((sp_wave_event(0, size, (sp_sine_state_1(size, frq, amp, 1)), (events + i))));
  };
  status_require((sp_block_new(1, size, (&block))));
  step_size = _rate;
  for (i = 0; (i < size); i += step_size) {
    sp_seq_parallel(i, size, (sp_block_with_offset(block, i)), events, 10);
  };
  for (i = 0; (i < 10); i += 1) {
    ((events[i]).free)((events + i));
  };
  free(amp);
  free(frq);
  sp_block_free(block);
exit:
  status_return;
}
#define temp_size 100
status_t test_temp() {
  status_declare;
  sp_wave_state_t state;
  sp_block_t out;
  sp_time_t* frq;
  sp_sample_t* amp;
  status_require((sp_block_new(1, temp_size, (&out))));
  status_require((sp_path_times_2((&frq), temp_size, (sp_path_move(0, 2000)), (sp_path_constant()))));
  status_require((sp_path_samples_2((&amp), temp_size, (sp_path_move(0, (1.0))), (sp_path_constant()))));
  state = sp_sine_state_1(temp_size, frq, amp, 0);
  sp_wave(0, temp_size, (&state), out);
  sp_samples_display(((out.samples)[0]), temp_size);
  sp_block_free(out);
exit:
  status_return;
}
/** "goto exit" can skip events */
int main() {
  status_declare;
  rs = sp_random_state_new(3);
  sp_initialise(3, _rate);
  test_helper_test_one(test_moving_average);
  test_helper_test_one(test_stats);
  test_helper_test_one(test_render_block);
  test_helper_test_one(test_wave_event);
  test_helper_test_one(test_wave);
  test_helper_test_one(test_path);
  test_helper_test_one(test_file);
  test_helper_test_one(test_sp_group);
  test_helper_test_one(test_sp_seq);
  test_helper_test_one(test_sp_cheap_noise_event);
  test_helper_test_one(test_sp_cheap_filter);
  test_helper_test_one(test_sp_noise_event);
  test_helper_test_one(test_sp_random);
  test_helper_test_one(test_sp_triangle_square);
  test_helper_test_one(test_fft);
  test_helper_test_one(test_spectral_inversion_ir);
  test_helper_test_one(test_base);
  test_helper_test_one(test_spectral_reversal_ir);
  test_helper_test_one(test_convolve);
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
