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
  status_return;
}
status_t test_moving_average() {
  status_declare;
  sp_time_t radius;
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
    unequal = !sp_sample_array_nearly_equal(((block.samples)[len]), sample_count, ((block_2.samples)[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-file-read new file result", !unequal);
  status_require((sp_file_close((&file))));
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
    unequal = !sp_sample_array_nearly_equal(((block.samples)[len]), sample_count, ((block_2.samples)[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-file-read existing result", !unequal);
  status_require((sp_file_close((&file))));
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
status_t test_synth() {
  status_declare;
  sp_time_t* state;
  sp_time_t config_len;
  sp_block_t out1;
  sp_block_t out2;
  sp_channels_t channels;
  sp_time_t duration;
  sp_synth_partial_t prt1;
  sp_synth_partial_t prt2;
  sp_synth_partial_t prt3;
  sp_synth_partial_t config[3];
  sp_time_t wvl[4] = { 2, 2, 2, 2 };
  sp_sample_t amp[4] = { 0.1, 0.2, 0.3, 0.4 };
  state = 0;
  duration = 4;
  channels = 2;
  config_len = 3;
  prt1.modifies = 0;
  prt2.modifies = 1;
  prt3.modifies = 0;
  prt1.start = 0;
  prt2.start = prt1.start;
  prt3.start = prt1.start;
  prt1.end = duration;
  prt2.end = prt1.end;
  prt3.end = prt1.end;
  (prt1.amp)[0] = amp;
  (prt1.amp)[1] = amp;
  (prt1.wvl)[0] = wvl;
  (prt1.wvl)[1] = wvl;
  (prt1.phs)[0] = 1;
  (prt1.phs)[2] = 2;
  (prt2.amp)[0] = amp;
  (prt2.amp)[1] = amp;
  (prt2.wvl)[0] = wvl;
  (prt2.wvl)[1] = wvl;
  (prt2.phs)[0] = 1;
  (prt2.phs)[2] = 2;
  (prt3.amp)[0] = amp;
  (prt3.amp)[1] = amp;
  (prt3.wvl)[0] = wvl;
  (prt3.wvl)[1] = wvl;
  (prt3.phs)[0] = 1;
  (prt3.phs)[2] = 2;
  config[0] = prt1;
  config[1] = prt2;
  config[2] = prt3;
  status_require((sp_block_new(channels, duration, (&out1))));
  status_require((sp_block_new(channels, duration, (&out2))));
  status_require((sp_synth_state_new(channels, config_len, config, (&state))));
  status_require((sp_synth(out1, 0, duration, config_len, config, state)));
  status_require((sp_synth(out2, 0, duration, config_len, config, state)));
  sp_block_free(out1);
  sp_block_free(out2);
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
  status_require((sph_helper_calloc((96000 * sizeof(sp_sample_t*)), (&out_t))));
  status_require((sph_helper_calloc((96000 * sizeof(sp_sample_t*)), (&out_s))));
  for (i = 0; (i < 96000); i = (1 + i)) {
    out_t[i] = sp_triangle_96(i);
    out_s[i] = sp_square_96(i);
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
  sp_random_samples((&s), 10, out);
  sp_random_samples((&s), 10, (10 + out));
  test_helper_assert("last value", (f64_nearly_equal((0.355602), (out[19]), error_margin)));
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
  sp_random_samples((&s), sp_noise_duration, in);
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
  status_require((sp_time_array_new(100, (&m1))));
  status_require((sp_time_array_new(100, (&m2))));
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
#define sp_synth_event_duration 100
#define sp_synth_event_half_duration (sp_synth_event_duration / 2)
/** sp synth values were taken from printing index and value of the result array.
   sp-plot-samples can plot the result */
status_t test_sp_synth_event() {
  status_declare;
  sp_event_t event;
  sp_block_t out;
  sp_time_t wvl1[sp_synth_event_duration];
  sp_time_t wvl2[sp_synth_event_duration];
  sp_sample_t amp1[sp_synth_event_duration];
  sp_sample_t amp2[sp_synth_event_duration];
  sp_synth_partial_t partials[2];
  sp_time_t i;
  sp_channels_t ci;
  for (i = 0; (i < sp_synth_event_duration); i += 1) {
    wvl1[i] = 5;
    wvl2[i] = 10;
    amp1[i] = 0.1;
    amp2[i] = 1;
  };
  partials[0] = sp_synth_partial_2(0, sp_synth_event_half_duration, 0, amp1, amp2, wvl1, wvl2, 0, 0);
  partials[1] = sp_synth_partial_2(sp_synth_event_half_duration, sp_synth_event_duration, 0, amp2, amp1, wvl2, wvl1, 0, 0);
  status_require((sp_synth_event(0, sp_synth_event_duration, 2, 2, partials, (&event))));
  status_require((sp_block_new(2, sp_synth_event_duration, (&out))));
  (event.f)(0, sp_synth_event_duration, out, (&event));
  test_helper_assert("values 1", (0.1 >= (out.samples)[0][0]));
  test_helper_assert("values 2", (0 == (out.samples)[0][49]));
  test_helper_assert("values 3", (0.1 > (out.samples)[0][44]));
  test_helper_assert("values 4", (0.1 <= (out.samples)[0][50]));
  test_helper_assert("values 5", (1 == (out.samples)[0][54]));
  test_helper_assert("values 6", (0.1 > (out.samples)[0][99]));
  sp_block_free(out);
  (event.free)((&event));
exit:
  status_return;
}
status_t test_path() {
  sp_samples_t samples;
  sp_times_t times;
  status_declare;
  status_require((sp_path_samples_2((&samples), 100, (sp_path_line(10, 1)), (sp_path_line(100, 0)))));
  status_require((sp_path_times_2((&times), 100, (sp_path_line(10, 1)), (sp_path_line(100, 0)))));
exit:
  status_return;
}
/** "goto exit" can skip events */
int main() {
  status_declare;
  sp_initialise(3);
  test_helper_test_one(test_path);
  test_helper_test_one(test_file);
  test_helper_test_one(test_sp_group);
  test_helper_test_one(test_sp_synth_event);
  test_helper_test_one(test_sp_seq);
  test_helper_test_one(test_sp_cheap_noise_event);
  test_helper_test_one(test_sp_cheap_filter);
  test_helper_test_one(test_sp_noise_event);
  test_helper_test_one(test_sp_random);
  test_helper_test_one(test_sp_triangle_square);
  test_helper_test_one(test_synth);
  test_helper_test_one(test_moving_average);
  test_helper_test_one(test_fft);
  test_helper_test_one(test_spectral_inversion_ir);
  test_helper_test_one(test_base);
  test_helper_test_one(test_spectral_reversal_ir);
  test_helper_test_one(test_convolve);
  test_helper_test_one(test_windowed_sinc);
exit:
  test_helper_display_summary();
  return ((status.id));
}
