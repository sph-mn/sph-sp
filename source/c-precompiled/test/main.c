
#include <sph-sp/sph-sp.h>
#include <sph-sp/string.h>
#include <sph-sp/filesystem.h>

#define test_helper_test_one(func) \
  printf("%s\n", #func); \
  status_require((func()))
#define test_helper_assert(description, expression) \
  if (!expression) { \
    printf("%s failed\n", description); \
    status_set_goto("sph-sp", 1); \
  }
#define test_helper_display_summary() \
  if (status_is_success) { \
    printf(("--\ntests finished successfully.\n")); \
  } else { \
    printf(("\ntests failed. %d %s\n"), (status.id), (sp_status_description(status))); \
  }
#define _sp_rate 960
#define test_noise_duration _sp_rate
status_t test_helper_event_generate(sp_time_t start, sp_time_t end, sp_block_t out, sp_event_t* event) {
  status_declare;
  sp_time_t i;
  sp_channel_count_t ci;
  uint64_t value;
  value = ((sp_time_t)(((uint64_t)(event->data))));
  for (i = start; (i < end); i += 1) {
    for (ci = 0; (ci < out.channel_count); ci += 1) {
      (out.samples)[ci][(i - start)] = value;
    };
  };
  status_return;
}
sp_event_t test_helper_event(sp_time_t start, sp_time_t end, sp_time_t number) {
  sp_declare_event(e);
  e.start = start;
  e.end = end;
  e.generate = test_helper_event_generate;
  e.data = ((void*)(((uint64_t)(number))));
  return (e);
}
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
  for (sp_size_t i = 0; (i < size); i += 1) {
    in[i] = i;
  };
  status_require((sp_windowed_sinc_bp_br(in, size, cutl, cuth, trnl, trnh, 0, (&state_control), out_control)));
  for (sp_size_t i = 0; (i < block_count); i += 1) {
    status_require((sp_windowed_sinc_bp_br(((i * block_size) + in), block_size, cutl, cuth, trnl, trnh, 0, (&state), ((i * block_size) + out))));
  };
  test_helper_assert("equal to block processing result", (sp_samples_nearly_equal(out, size, out_control, size, (0.01))));
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
  status_require((sp_block_new(channel_count, sample_count, (&block_write))));
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
  status_require((sp_block_new(channel_count, sample_count, (&block_read))));
  status_require((sp_file_open_read(test_file_path, (&file))));
  status_require((sp_file_read(file, sample_count, (block_read.samples))));
  for (sp_channel_count_t j = 0; (j < channel_count); j += 1) {
    test_helper_assert("sp-file-read new file result", (sp_samples_nearly_equal(((block_write.samples)[j]), sample_count, ((block_read.samples)[j]), sample_count, error_margin)));
  };
  sp_file_close_read(file);
exit:
  sp_block_free((&block_write));
  sp_block_free((&block_read));
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
  status_return;
}
status_t test_sp_triangle_square() {
  status_declare;
  sp_sample_t* out_t;
  sp_sample_t* out_s;
  sp_time_t size;
  size = 96000;
  status_require((sph_helper_calloc((size * sizeof(sp_sample_t*)), (&out_t))));
  status_require((sph_helper_calloc((size * sizeof(sp_sample_t*)), (&out_s))));
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
status_t test_sp_random() {
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
status_t test_sp_noise_event() {
  status_declare;
  sp_block_t out;
  sp_sample_t cutl[test_noise_duration];
  sp_sample_t cuth[test_noise_duration];
  sp_sample_t amod[test_noise_duration];
  sp_noise_event_config_t* config;
  sp_declare_event(event);
  sp_declare_event_list(events);
  error_memory_init(2);
  status_require((sp_noise_event_config_new((&config))));
  error_memory_add(config);
  status_require((sp_block_new(2, test_noise_duration, (&out))));
  error_memory_add2((&out), sp_block_free);
  for (sp_size_t i = 0; (i < test_noise_duration); i += 1) {
    cutl[i] = 0.01;
    cuth[i] = 0.3;
    amod[i] = 1.0;
  };
  (*config).cutl_mod = cutl;
  (*config).cuth_mod = cuth;
  (*config).amod = amod;
  (*config).amp = 1;
  (*config).channel_count = 2;
  event.start = 0;
  event.end = test_noise_duration;
  event.prepare = sp_noise_event_prepare;
  event.config = config;
  status_require((sp_event_list_add((&events), event)));
  status_require((sp_seq(0, test_noise_duration, out, (&events))));
  test_helper_assert(("in range -1..1"), (1.0 >= sp_samples_absolute_max(((out.samples)[0]), test_noise_duration)));
  sp_block_free((&out));
exit:
  if (status_is_failure) {
    error_memory_free;
  };
  status_return;
}
status_t test_sp_cheap_filter() {
  status_declare;
  sp_cheap_filter_state_t state;
  sp_sample_t out[test_noise_duration];
  sp_sample_t in[test_noise_duration];
  sp_random_state_t s;
  s = sp_random_state_new(80);
  sp_samples_random_primitive((&s), test_noise_duration, in);
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
  error_memory_init(2);
  status_require((sp_cheap_noise_event_config_new((&config))));
  error_memory_add(config);
  status_require((sp_block_new(2, test_noise_duration, (&out))));
  error_memory_add2((&out), sp_block_free);
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
  (*config).channel_count = 2;
  (*config).amp = 1;
  event.end = test_noise_duration;
  event.config = &config;
  event.prepare = sp_cheap_noise_event_prepare;
  status_require(((event.prepare)((&event))));
  status_require(((event.generate)(0, test_noise_duration, out, (&event))));
  test_helper_assert(("in range -1..1"), (1.0 >= sp_samples_absolute_max(((out.samples)[0]), test_noise_duration)));
  sp_block_free((&out));
exit:
  if (status_is_failure) {
    error_memory_free;
  };
  status_return;
}
status_t test_sp_sound_event() {
  status_declare;
  sp_block_t out;
  sp_time_t fmod[test_noise_duration];
  sp_time_t wmod[test_noise_duration];
  sp_sample_t amod[test_noise_duration];
  sp_time_t i;
  sp_sound_event_config_t config;
  sp_declare_event(event);
  status_require((sp_block_new(2, test_noise_duration, (&out))));
  for (i = 0; (i < test_noise_duration); i += 1) {
    fmod[i] = 30;
    wmod[i] = 200;
    amod[i] = 1.0;
  };
  config.amp = 1;
  config.amod = amod;
  config.noise = 0;
  config.frq = 200;
  config.fmod = 0;
  config.wmod = 0;
  config.wdt = 200;
  event.end = test_noise_duration;
  event.config = &config;
  event.prepare = sp_sound_event_prepare;
  status_require(((event.prepare)((&event))));
  status_require(((event.generate)(0, test_noise_duration, out, (&event))));
  config.noise = 1;
  status_require(((event.prepare)((&event))));
  status_require(((event.generate)(0, test_noise_duration, out, (&event))));
  config.noise = 2;
  status_require(((event.prepare)((&event))));
  status_require(((event.generate)(0, test_noise_duration, out, (&event))));
exit:
  status_return;
}
#define sp_seq_event_count 2
status_t test_sp_seq() {
  status_declare;
  sp_block_t out;
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
  status_require((sp_event_memory_ensure((&g), 2)));
  sp_event_memory_fixed_add((&g), m1);
  sp_event_memory_fixed_add((&g), m2);
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
  error_memory_init(2);
  status_require((sp_wave_event_config_new((&config))));
  error_memory_add(config);
  status_require((sp_block_new(2, test_wave_event_duration, (&out))));
  error_memory_add2((&out), sp_block_free);
  for (sp_size_t i = 0; (i < test_wave_event_duration); i += 1) {
    fmod[i] = 2000;
    amod1[i] = 1;
    amod2[i] = 0.5;
  };
  (*config).wvf = sp_sine_table;
  (*config).wvf_size = sp_rate;
  (*config).fmod = fmod;
  (*config).amp = 1;
  (*config).amod = amod1;
  (*config).channel_count = 2;
  (config->channel_config)[1] = sp_channel_config(0, 10, 10, 1, amod2);
  event.start = 0;
  event.end = test_wave_event_duration;
  event.config = config;
  event.prepare = sp_wave_event_prepare;
  status_require(((event.prepare)((&event))));
  status_require(((event.generate)(0, 30, out, (&event))));
  status_require(((event.generate)(30, test_wave_event_duration, (sp_block_with_offset(out, 30)), (&event))));
  /* (sp-plot-samples (array-get out.samples 0) test-wave-event-duration) */
  sp_block_free((&out));
exit:
  if (status_is_failure) {
    error_memory_free;
  };
  status_return;
}
status_t test_render_range_block() {
  status_declare;
  sp_block_t out;
  sp_time_t frq[test_wave_event_duration];
  sp_sample_t amod[test_wave_event_duration];
  sp_time_t i;
  sp_render_config_t rc;
  sp_wave_event_config_t* config;
  sp_declare_event(event);
  error_memory_init(2);
  rc = sp_render_config(sp_channel_count, sp_rate, sp_rate);
  rc.block_size = 40;
  for (i = 0; (i < test_wave_event_duration); i += 1) {
    frq[i] = 1500;
    amod[i] = 1;
  };
  status_require((sp_wave_event_config_new((&config))));
  error_memory_add(config);
  status_require((sp_block_new(1, test_wave_event_duration, (&out))));
  error_memory_add2((&out), sp_block_free);
  (*config).wvf = sp_sine_table;
  (*config).wvf_size = sp_rate;
  (*config).fmod = frq;
  (*config).amp = 1;
  (*config).amod = amod;
  (*config).channel_count = 1;
  event.start = 0;
  event.end = test_wave_event_duration;
  event.config = config;
  event.prepare = sp_wave_event_prepare;
  /* (sp-render-range-file event test-wave-event-duration rc /tmp/sp-test.wav) */
  sp_render_range_block(event, 0, test_wave_event_duration, rc, (&out));
  /* (sp-block-plot-1 out) */
  sp_block_free((&out));
exit:
  if (status_is_failure) {
    error_memory_free;
  };
  status_return;
}
status_t test_path() {
  sp_sample_t* samples;
  sp_time_t* times;
  sp_path_curves_config_t curves_config;
  sp_sample_t* out;
  status_declare;
  status_require((sp_path_samples_2((&samples), 100, (sp_path_line(10, 1)), (sp_path_line(100, 0)))));
  status_require((sp_path_times_2((&times), 100, (sp_path_line(10, 1)), (sp_path_line(100, 0)))));
  srq((sp_path_curves_config_new(5, (&curves_config))));
  (curves_config.x)[0] = 20;
  (curves_config.x)[1] = 40;
  (curves_config.x)[2] = 50;
  (curves_config.x)[3] = 55;
  (curves_config.x)[4] = 100;
  (curves_config.y)[0] = 10;
  (curves_config.y)[1] = 20;
  (curves_config.y)[2] = 30;
  (curves_config.y)[3] = 40;
  (curves_config.y)[4] = 70;
  (curves_config.c)[0] = -1;
  (curves_config.c)[1] = 1;
  (curves_config.c)[2] = 0.1;
  (curves_config.c)[3] = 0.7;
  (curves_config.c)[4] = -0.1;
  sp_path_curves_samples_new(curves_config, 100, (&out));
exit:
  status_return;
}
#define feq(a, b) sp_sample_nearly_equal(a, b, (0.01))
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
  sp_times_set(a, size, 1039);
  status_require((sp_times_new((8 * sizeof(sp_time_t)), (&bits))));
  sp_times_bits_to_times(a, (8 * sizeof(sp_time_t)), bits);
  test_helper_assert(("bits->times"), ((1 == bits[0]) && (1 == bits[3]) && (0 == bits[4]) && (1 == bits[10])));
  free(bits);
  sp_times_multiplications(1, 3, size, a);
  sp_times_shuffle(a, size);
  s = sp_random_state_new(12);
  sp_times_random_binary(size, a);
  sp_times_multiplications(1, 3, size, a);
  s = sp_random_state_new(113);
  sp_times_select_random(a, size, b, (&b_size));
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
status_t test_simple_mappings() {
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
  sp_times_random_discrete(cudist, cudist_size, 8, a);
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
  error_memory_init(4);
  status_require((sp_wave_event_config_new((&config))));
  error_memory_add(config);
  status_require((sp_path_samples_2((&amod), size, (sp_path_move(0, (1.0))), (sp_path_constant()))));
  error_memory_add(amod);
  status_require((sp_path_times_2((&fmod), size, (sp_path_move(0, 250)), (sp_path_constant()))));
  error_memory_add(fmod);
  (*config).wvf = sp_sine_table;
  (*config).wvf_size = sp_rate;
  (*config).fmod = fmod;
  (*config).amp = 1;
  (*config).amod = amod;
  (*config).channel_count = 2;
  for (i = 0; (i < 10); i += 1) {
    event.start = 0;
    event.end = size;
    event.config = config;
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
    error_memory_free;
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
  error_memory_init(2);
  status_require((sp_wave_event_config_new((&config))));
  error_memory_add(config);
  status_require((sp_map_event_config_new((&map_event_config))));
  error_memory_add(map_event_config);
  size = (10 * _sp_rate);
  status_require((sp_path_samples_2((&amod), size, (sp_path_move(0, (1.0))), (sp_path_constant()))));
  (*config).wvf = sp_sine_table;
  (*config).wvf_size = sp_rate;
  (*config).frq = 300;
  (*config).fmod = 0;
  (*config).amp = 1;
  (*config).amod = amod;
  (*config).channel_count = 1;
  child.start = 0;
  child.end = size;
  child.config = config;
  child.prepare = sp_wave_event_prepare;
  status_require((sp_block_new(1, size, (&block))));
  (*map_event_config).event = child;
  (*map_event_config).map_generate = test_sp_map_event_generate;
  (*map_event_config).isolate = 1;
  parent.start = child.start;
  parent.end = child.end;
  parent.prepare = sp_map_event_prepare;
  parent.config = map_event_config;
  status_require(((parent.prepare)((&parent))));
  status_require(((parent.generate)(0, (size / 2), block, (&parent))));
  status_require(((parent.generate)((size / 2), size, block, (&parent))));
  (parent.free)((&parent));
  sp_block_free((&block));
  free(amod);
exit:
  if (status_is_failure) {
    error_memory_free;
  };
  status_return;
}
status_t test_sp_pan_to_amp() {
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

/** "goto exit" can skip events */
int main() {
  status_declare;
  sp_initialize(3, 2, _sp_rate);
  test_helper_test_one(test_base);
  test_helper_test_one(test_path);
  test_helper_test_one(test_sp_sound_event);
  test_helper_test_one(test_sp_pan_to_amp);
  test_helper_test_one(test_windowed_sinc_continuity);
  test_helper_test_one(test_convolve_smaller);
  test_helper_test_one(test_convolve_larger);
  test_helper_test_one(test_sp_noise_event);
  test_helper_test_one(test_sp_wave_event);
  test_helper_test_one(test_sp_cheap_noise_event);
  test_helper_test_one(test_sp_map_event);
  test_helper_test_one(test_sp_group);
  test_helper_test_one(test_sp_seq);
  test_helper_test_one(test_render_range_block);
  test_helper_test_one(test_moving_average);
  test_helper_test_one(test_statistics);
  test_helper_test_one(test_sp_cheap_filter);
  test_helper_test_one(test_sp_random);
  test_helper_test_one(test_sp_triangle_square);
  test_helper_test_one(test_fft);
  test_helper_test_one(test_spectral_inversion_ir);
  test_helper_test_one(test_spectral_reversal_ir);
  test_helper_test_one(test_windowed_sinc);
  test_helper_test_one(test_compositions);
  test_helper_test_one(test_times);
  test_helper_test_one(test_simple_mappings);
  test_helper_test_one(test_random_discrete);
  test_helper_test_one(test_file);
  test_helper_test_one(test_permutations);
  test_helper_test_one(test_sp_seq_parallel);
exit:
  test_helper_display_summary();
  return ((status.id));
}
