
#include <sph-sp.h>
#ifndef sc_included_stdio_h
#include <stdio.h>
#define sc_included_stdio_h
#endif
#ifndef sc_included_stdlib_h
#include <stdlib.h>
#define sc_included_stdlib_h
#endif
#ifndef sc_included_errno_h
#include <errno.h>
#define sc_included_errno_h
#endif
#ifndef sc_included_pthread_h
#include <pthread.h>
#define sc_included_pthread_h
#endif
#ifndef sc_included_float_h
#include <float.h>
#define sc_included_float_h
#endif
#ifndef sc_included_math_h
#include <math.h>
#define sc_included_math_h
#endif
#define inc(a) a = (1 + a)
#define dec(a) a = (a - 1)
#define test_helper_test_one(func)                                             \
  printf("%s\n", #func);                                                       \
  status_require_x(func())
#define test_helper_assert(description, expression)                            \
  if (!expression) {                                                           \
    printf("%s failed\n", description);                                        \
    status_set_id_goto(1);                                                     \
  }
#define test_helper_display_summary()                                          \
  if (status_success_p) {                                                      \
    printf("--\ntests finished successfully.\n");                              \
  } else {                                                                     \
    printf("\ntests failed. %d %s\n", status.id,                               \
           sp_status_description(status));                                     \
  }
/* depends on sph.sc */
#ifndef sc_included_string_h
#include <string.h>
#define sc_included_string_h
#endif
#ifndef sc_included_stdlib_h
#include <stdlib.h>
#define sc_included_stdlib_h
#endif
/** set result to a new string with a trailing slash added, or the given string
  if it already has a trailing slash. returns 0 if result is the given string, 1
  if new memory could not be allocated, 2 if result is a new string */
b8 ensure_trailing_slash(b8 *a, b8 **result) {
  b32 a_len = strlen(a);
  if ((!a_len || (('/' == (*(a + (a_len - 1))))))) {
    (*result) = a;
    return (0);
  } else {
    char *new_a = malloc((2 + a_len));
    if (!new_a) {
      return (1);
    };
    memcpy(new_a, a, a_len);
    memcpy((new_a + a_len), "/", 1);
    (*(new_a + (1 + a_len))) = 0;
    (*result) = new_a;
    return (2);
  };
};
/** always returns a new string */
b8 *string_append(b8 *a, b8 *b) {
  size_t a_length = strlen(a);
  size_t b_length = strlen(b);
  b8 *result = malloc((1 + a_length + b_length));
  if (result) {
    memcpy(result, a, a_length);
    memcpy((result + a_length), b, (1 + b_length));
  };
  return (result);
};
/** return a new string with the same contents as the given string. return 0 if
 * the memory allocation failed */
b8 *string_clone(b8 *a) {
  size_t a_size = (1 + strlen(a));
  b8 *result = malloc(a_size);
  if (result) {
    memcpy(result, a, a_size);
  };
  return (result);
};
#ifndef sc_included_unistd_h
#include <unistd.h>
#define sc_included_unistd_h
#endif
#ifndef sc_included_sys_stat_h
#include <sys/stat.h>
#define sc_included_sys_stat_h
#endif
#ifndef sc_included_libgen_h
#include <libgen.h>
#define sc_included_libgen_h
#endif
#ifndef sc_included_errno_h
#include <errno.h>
#define sc_included_errno_h
#endif
#define file_exists_p(path) !(access(path, F_OK) == -1)
/** like posix dirname, but never modifies its argument and always returns a new
 * string */
b8 *dirname_2(b8 *a) {
  b8 *path_copy = string_clone(a);
  return (dirname(path_copy));
};
/** return 1 if the path exists or has been successfully created */
boolean ensure_directory_structure(b8 *path, mode_t mkdir_mode) {
  if (file_exists_p(path)) {
    return (1);
  } else {
    b8 *path_dirname = dirname_2(path);
    boolean status = ensure_directory_structure(path_dirname, mkdir_mode);
    free(path_dirname);
    return ((status &&
             ((((EEXIST == errno)) || ((0 == mkdir(path, mkdir_mode)))))));
  };
};
/** register memory in a local variable to free all memory allocated at point */
#define local_memory_init(register_size)                                       \
  b0 *sph_local_memory_register[register_size];                                \
  b8 sph_local_memory_index = 0
;
/** do not try to add more entries than specified by register-size or a buffer
 * overflow occurs */
#define local_memory_add(address)                                              \
  (*(sph_local_memory_register + sph_local_memory_index)) = address;           \
  sph_local_memory_index = (1 + sph_local_memory_index)
;
#define local_memory_free                                                      \
  while (sph_local_memory_index) {                                             \
    sph_local_memory_index = (sph_local_memory_index - 1);                     \
    free((*(sph_local_memory_register + sph_local_memory_index)));             \
  }
;
b0 debug_log_samples(sp_sample_t *a, size_t len) {
  size_t column_width;
  size_t column_end;
  size_t index;
  column_width = 8;
  index = 0;
  while ((index < len)) {
    column_end = (index + column_width);
    while (((index < len) && (index < column_end))) {
      printf("%f ", (*(a + index)));
      index = (1 + index);
    };
    printf("\n");
  };
};
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
  status_init;
  test_helper_assert("input 0.5", sp_sample_nearly_equal_p(
                                      0.63662, sp_sinc(0.5), error_margin));
  test_helper_assert("input 1",
                     sp_sample_nearly_equal_p(1.0, sp_sinc(0), error_margin));
  test_helper_assert("window-blackman 1.1 20",
                     sp_sample_nearly_equal_p(
                         0.550175, sp_window_blackman(1.1, 20), error_margin));
exit:
  return (status);
};
status_t test_spectral_inversion_ir() {
  status_init;
  size_t a_len = 5;
  sp_sample_t a[5] = {0.1, -0.2, 0.3, -0.2, 0.1};
  sp_spectral_inversion_ir(a, a_len);
  test_helper_assert(
      "result check",
      (sp_sample_nearly_equal_p(-0.1, (*(a + 0)), error_margin) &&
       sp_sample_nearly_equal_p(0.2, (*(a + 1)), error_margin) &&
       sp_sample_nearly_equal_p(-0.3, (*(a + 2)), error_margin) &&
       sp_sample_nearly_equal_p(0.2, (*(a + 3)), error_margin) &&
       sp_sample_nearly_equal_p(-0.1, (*(a + 4)), error_margin)));
exit:
  return (status);
};
status_t test_spectral_reversal_ir() {
  status_init;
  size_t a_len = 5;
  sp_sample_t a[5] = {0.1, -0.2, 0.3, -0.2, 0.1};
  sp_spectral_reversal_ir(a, a_len);
  test_helper_assert("result check",
                     (sp_sample_nearly_equal_p(0.1, (*(a + 0)), error_margin) &&
                      sp_sample_nearly_equal_p(0.2, (*(a + 1)), error_margin) &&
                      sp_sample_nearly_equal_p(0.3, (*(a + 2)), error_margin) &&
                      sp_sample_nearly_equal_p(0.2, (*(a + 3)), error_margin) &&
                      sp_sample_nearly_equal_p(0.1, (*(a + 4)), error_margin)));
exit:
  return (status);
};
b8 *test_file_path = "/tmp/test-sph-sp-file";
status_t test_port() {
  status_init;
  local_memory_init(2);
  if (file_exists_p(test_file_path)) {
    unlink(test_file_path);
  };
  b32 channel_count = 2;
  b32 sample_rate = 8000;
  sp_port_t port;
  size_t sample_count = 5;
  sp_sample_t **channel_data =
      sp_alloc_channel_array(channel_count, sample_count);
  sp_sample_t **channel_data_2 =
      sp_alloc_channel_array(channel_count, sample_count);
  sp_alloc_require(channel_data);
  local_memory_add(channel_data);
  local_memory_add(channel_data_2);
  size_t len;
  size_t channel = channel_count;
  while (channel) {
    dec(channel);
    len = sample_count;
    while (len) {
      dec(len);
      (*((*(channel_data + channel)) + len)) = len;
    };
  };
  status_require_x(
      sp_file_open(&port, test_file_path, channel_count, sample_rate));
  printf(" create\n");
  size_t position = 0;
  status_require_x(sp_port_position(&position, &port));
  status_require_x(sp_port_write(&port, sample_count, channel_data));
  status_require_x(sp_port_position(&position, &port));
  test_helper_assert("sp-port-position file after write",
                     (sample_count == position));
  status_require_x(sp_port_set_position(&port, 0));
  status_require_x(sp_port_read(channel_data_2, &port, sample_count));
  len = channel_count;
  b8_s unequal = 0;
  while ((len && !unequal)) {
    dec(len);
    unequal = !sp_sample_array_nearly_equal_p(
        (*(channel_data + len)), sample_count, (*(channel_data_2 + len)),
        sample_count, error_margin);
  };
  test_helper_assert("sp-port-read new file result", !unequal);
  status_require_x(sp_port_close(&port));
  printf("  write\n");
  status_require_x(sp_file_open(&port, test_file_path, 2, 8000));
  status_require_x(sp_port_position(&position, &port));
  test_helper_assert("sp-port-position existing file",
                     (sample_count == position));
  status_require_x(sp_port_set_position(&port, 0));
  status_require_x(sp_port_read(channel_data_2, &port, sample_count));
  unequal = 0;
  len = channel_count;
  while ((len && !unequal)) {
    dec(len);
    unequal = !sp_sample_array_nearly_equal_p(
        (*(channel_data + len)), sample_count, (*(channel_data_2 + len)),
        sample_count, error_margin);
  };
  test_helper_assert("sp-port-read existing result", !unequal);
  status_require_x(sp_port_close(&port));
  printf("  open\n");
exit:
  local_memory_free;
  return (status);
};
status_t test_convolve() {
  status_init;
  size_t sample_count = 5;
  size_t b_len = 3;
  size_t result_len = sample_count;
  size_t a_len = sample_count;
  size_t carryover_len = b_len;
  local_memory_init(4);
  sp_alloc_define_samples(result, result_len);
  local_memory_add(result);
  sp_alloc_define_samples(a, a_len);
  local_memory_add(a);
  sp_alloc_define_samples(b, b_len);
  local_memory_add(b);
  sp_alloc_define_samples(carryover, carryover_len);
  local_memory_add(carryover);
  (*(a + 0)) = 2;
  (*(a + 1)) = 3;
  (*(a + 2)) = 4;
  (*(a + 3)) = 5;
  (*(a + 4)) = 6;
  (*(b + 0)) = 1;
  (*(b + 1)) = 2;
  (*(b + 2)) = 3;
  (*(carryover + 0)) = 0;
  (*(carryover + 1)) = 0;
  (*(carryover + 2)) = 0;
  sp_sample_t expected_result[5] = {2, 7, 16, 22, 28};
  sp_sample_t expected_carryover[3] = {27, 18, 0};
  sp_convolve(result, a, a_len, b, b_len, carryover, carryover_len);
  test_helper_assert("first result", sp_sample_array_nearly_equal_p(
                                         result, result_len, expected_result,
                                         result_len, error_margin));
  test_helper_assert("first result carryover",
                     sp_sample_array_nearly_equal_p(
                         carryover, carryover_len, expected_carryover,
                         carryover_len, error_margin));
  (*(a + 0)) = 8;
  (*(a + 1)) = 9;
  (*(a + 2)) = 10;
  (*(a + 3)) = 11;
  (*(a + 4)) = 12;
  (*(expected_result + 0)) = 35;
  (*(expected_result + 1)) = 43;
  (*(expected_result + 2)) = 52;
  (*(expected_result + 3)) = 58;
  (*(expected_result + 4)) = 64;
  (*(expected_carryover + 0)) = 57;
  (*(expected_carryover + 1)) = 36;
  (*(expected_carryover + 2)) = 0;
  sp_convolve(result, a, a_len, b, b_len, carryover, carryover_len);
  test_helper_assert("second result", sp_sample_array_nearly_equal_p(
                                          result, result_len, expected_result,
                                          result_len, error_margin));
  test_helper_assert("second result carryover",
                     sp_sample_array_nearly_equal_p(
                         carryover, carryover_len, expected_carryover,
                         carryover_len, error_margin));
exit:
  local_memory_free;
  return (status);
};
status_t test_moving_average() {
  status_init;
  size_t source_len = 8;
  size_t result_len = source_len;
  size_t prev_len = 4;
  size_t next_len = prev_len;
  local_memory_init(4);
  sp_alloc_define_samples(result, result_len);
  local_memory_add(result);
  sp_alloc_define_samples(source, source_len);
  local_memory_add(source);
  sp_alloc_define_samples(prev, prev_len);
  local_memory_add(prev);
  sp_alloc_define_samples(next, next_len);
  local_memory_add(next);
  (*(source + 0)) = 1;
  (*(source + 1)) = 4;
  (*(source + 2)) = 8;
  (*(source + 3)) = 12;
  (*(source + 4)) = 3;
  (*(source + 5)) = 32;
  (*(source + 6)) = 2;
  (*(prev + 0)) = 3;
  (*(prev + 1)) = 2;
  (*(prev + 2)) = 1;
  (*(prev + 3)) = -12;
  (*(next + 0)) = 83;
  (*(next + 1)) = 12;
  (*(next + 2)) = -32;
  (*(next + 3)) = 2;
  size_t radius = 4;
  sp_moving_average(result, source, source_len, prev, prev_len, next, next_len,
                    0, (source_len - 1), radius);
  sp_moving_average(result, source, source_len, 0, 0, 0, 0, 0, (source_len - 1),
                    (1 + (source_len / 2)));
exit:
  local_memory_free;
  return (status);
};
int main() {
  status_init;
  test_helper_test_one(test_convolve);
  test_helper_test_one(test_moving_average);
  test_helper_test_one(test_port);
  test_helper_test_one(test_base);
  test_helper_test_one(test_spectral_reversal_ir);
  test_helper_test_one(test_spectral_inversion_ir);
exit:
  test_helper_display_summary();
  return (status.id);
};