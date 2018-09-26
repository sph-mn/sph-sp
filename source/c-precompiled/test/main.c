#include <sph-sp.h>
#include "./helper.c"
#if (sp_sample_type_f64 == sp_sample_type)
#define sp_sample_nearly_equal_p f64_nearly_equal_p
#define sp_sample_array_nearly_equal_p f64_array_nearly_equal_p
#else
#if (sp_sample_type_f32 == sp_sample_type)
#define sp_sample_nearly_equal_p f32_nearly_equal_p
#define sp_sample_array_nearly_equal_p f32_array_nearly_equal_p
#endif
#endif
sp_sample_t error_margin = 0.1;
status_t test_base() {
  status_declare;
  test_helper_assert(("input 0.5"), (sp_sample_nearly_equal_p((0.63662), (sp_sinc((0.5))), error_margin)));
  test_helper_assert("input 1", (sp_sample_nearly_equal_p((1.0), (sp_sinc(0)), error_margin)));
  test_helper_assert(("window-blackman 1.1 20"), (sp_sample_nearly_equal_p((0.550175), (sp_window_blackman((1.1), 20)), error_margin)));
exit:
  return (status);
};
status_t test_spectral_inversion_ir() {
  status_declare;
  size_t a_len;
  sp_sample_t a[5] = { 0.1, -0.2, 0.3, -0.2, 0.1 };
  a_len = 5;
  sp_spectral_inversion_ir(a, a_len);
  test_helper_assert("result check", (sp_sample_nearly_equal_p((-0.1), (a[0]), error_margin) && sp_sample_nearly_equal_p((0.2), (a[1]), error_margin) && sp_sample_nearly_equal_p((-0.3), (a[2]), error_margin) && sp_sample_nearly_equal_p((0.2), (a[3]), error_margin) && sp_sample_nearly_equal_p((-0.1), (a[4]), error_margin)));
exit:
  return (status);
};
status_t test_spectral_reversal_ir() {
  status_declare;
  size_t a_len;
  sp_sample_t a[5] = { 0.1, -0.2, 0.3, -0.2, 0.1 };
  a_len = 5;
  sp_spectral_reversal_ir(a, a_len);
  test_helper_assert("result check", (sp_sample_nearly_equal_p((0.1), (a[0]), error_margin) && sp_sample_nearly_equal_p((0.2), (a[1]), error_margin) && sp_sample_nearly_equal_p((0.3), (a[2]), error_margin) && sp_sample_nearly_equal_p((0.2), (a[3]), error_margin) && sp_sample_nearly_equal_p((0.1), (a[4]), error_margin)));
exit:
  return (status);
};
uint8_t* test_file_path = "/tmp/test-sph-sp-file";
status_t test_port() {
  status_declare;
  size_t position;
  uint32_t channel_count;
  uint32_t sample_rate;
  size_t sample_count;
  sp_sample_t** channel_data;
  int8_t unequal;
  sp_sample_t** channel_data_2;
  size_t channel;
  sp_port_t port;
  size_t len;
  local_memory_init(2);
  if (file_exists_p(test_file_path)) {
    unlink(test_file_path);
  };
  channel_count = 2;
  sample_rate = 8000;
  sample_count = 5;
  channel_data = sp_alloc_channel_array(channel_count, sample_count);
  channel_data_2 = sp_alloc_channel_array(channel_count, sample_count);
  sp_alloc_require(channel_data);
  local_memory_add(channel_data);
  local_memory_add(channel_data_2);
  channel = channel_count;
  while (channel) {
    dec(channel);
    len = sample_count;
    while (len) {
      dec(len);
      (channel_data[channel])[len] = len;
    };
  };
  status_require((sp_file_open((&port), test_file_path, channel_count, sample_rate)));
  printf(" create\n");
  position = 0;
  status_require((sp_port_position((&position), (&port)())));
  status_require((sp_port_write((&port), sample_count, channel_data)));
  status_require((sp_port_position((&position), (&port)())));
  test_helper_assert("sp-port-position file after write", (sample_count == position));
  status_require((sp_port_set_position((&port), 0)));
  sp_port_read(channel_data_2, (&port), sample_count);
  len = channel_count;
  unequal = 0;
  while ((len && !unequal)) {
    dec(len);
    unequal = !sp_sample_array_nearly_equal_p((channel_data[len]), sample_count, (channel_data_2[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-port-read new file result", !unequal);
  status_require((sp_port_close((&port))));
  printf("  write\n");
  status_require((sp_file_open((&port), test_file_path, 2, 8000)));
  status_require((sp_port_position((&position), (&port))));
  test_helper_assert("sp-port-position existing file", (sample_count == position));
  status_require((sp_port_set_position((&port), 0)));
  sp_port_read(channel_data_2, (&port), sample_count);
  unequal = 0;
  len = channel_count;
  while ((len && !unequal)) {
    dec(len);
    unequal = !sp_sample_array_nearly_equal_p((channel_data[len]), sample_count, (channel_data_2[len]), sample_count, error_margin);
  };
  test_helper_assert("sp-port-read existing result", !unequal);
  status_require((sp_port_close((&port))));
  printf("  open\n");
exit:
  local_memory_free;
  return (status);
};
status_t test_convolve() {
  status_declare;
  size_t sample_count;
  size_t b_len;
  size_t result_len;
  size_t a_len;
  size_t carryover_len;
  sample_count = 5;
  b_len = 3;
  result_len = sample_count;
  a_len = sample_count;
  carryover_len = b_len;
  local_memory_init(4);
  sp_alloc_define_samples(result, result_len);
  local_memory_add(result);
  sp_alloc_define_samples(a, a_len);
  local_memory_add(a);
  sp_alloc_define_samples(b, b_len);
  local_memory_add(b);
  sp_alloc_define_samples(carryover, carryover_len);
  local_memory_add(carryover);
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
  define_array(expected_result, sp_sample_t, (5()), 2, 7, 16, 22, 28);
  define_array(expected_carryover, sp_sample_t, (3()), 27, 18, 0);
  sp_convolve(result, a, a_len, b, b_len, carryover, carryover_len);
  test_helper_assert("first result", (sp_sample_array_nearly_equal_p(result, result_len, expected_result, result_len, error_margin)));
  test_helper_assert("first result carryover", (sp_sample_array_nearly_equal_p(carryover, carryover_len, expected_carryover, carryover_len, error_margin)));
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
  sp_convolve(result, a, a_len, b, b_len, carryover, carryover_len);
  test_helper_assert("second result", (sp_sample_array_nearly_equal_p(result, result_len, expected_result, result_len, error_margin)));
  test_helper_assert("second result carryover", (sp_sample_array_nearly_equal_p(carryover, carryover_len, expected_carryover, carryover_len, error_margin)));
exit:
  local_memory_free;
  return (status);
};
status_t test_moving_average() {
  status_declare;
  size_t source_len;
  size_t result_len;
  size_t prev_len;
  size_t next_len;
  size_t radius;
  source_len = 8;
  result_len = source_len;
  prev_len = 4;
  next_len = prev_len;
  local_memory_init(4);
  sp_alloc_define_samples(result, result_len);
  local_memory_add(result);
  sp_alloc_define_samples(source, source_len);
  local_memory_add(source);
  sp_alloc_define_samples(prev, prev_len);
  local_memory_add(prev);
  sp_alloc_define_samples(next, next_len);
  local_memory_add(next);
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
  radius = 4;
  sp_moving_average(result, source, source_len, prev, prev_len, next, next_len, 0, (source_len - 1), radius);
  sp_moving_average(result, source, source_len, 0, 0, 0, 0, 0, (source_len - 1), (1 + (source_len / 2)));
exit:
  local_memory_free;
  return (status);
};
int main() {
  status_declare;
  test_helper_test_one(test_port);
  test_helper_test_one(test_convolve);
  test_helper_test_one(test_moving_average);
  test_helper_test_one(test_base);
  test_helper_test_one(test_spectral_reversal_ir);
  test_helper_test_one(test_spectral_inversion_ir);
exit:
  test_helper_display_summary();
  return ((status.id));
};