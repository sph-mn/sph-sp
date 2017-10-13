
#include <sph-sp.c>
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
#ifndef sc_included_string_h
#include <string.h>
#define sc_included_string_h
#endif
#ifndef sc_included_stdlib_h
#include <stdlib.h>
#define sc_included_stdlib_h
#endif
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
#ifndef sc_included_float_h
#include <float.h>
#define sc_included_float_h
#endif
#define file_exists_p(path) !(access(path, F_OK) == -1)
#define pointer_equal_p(a, b) (((b0 *)(a)) == ((b0 *)(b)))
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
/** sum numbers with rounding error compensation using kahan summation with
 * neumaier modification */
f32_s float_sum(f32_s *numbers, b32 len) {
  f32_s temp;
  f32_s element;
  f32_s correction = 0;
  len = (len - 1);
  f32_s result = (*(numbers + len));
  while (len) {
    len = (len - 1);
    element = (*(numbers + len));
    temp = (result + element);
    correction =
        (correction + ((result >= element) ? ((result - temp) + element)
                                           : ((element - temp) + result)));
    result = temp;
  };
  return ((correction + result));
};
/** approximate float comparison. margin is a factor and is low for low accepted
   differences. http://floating-point-gui.de/errors/comparison/ */
boolean float_nearly_equal_p(f32_s a, f32_s b, f32_s margin) {
  if ((a == b)) {
    return (1);
  } else {
    f32_s diff = fabs((a - b));
    return (((((0 == a)) || ((0 == b)) || (diff < DBL_MIN))
                 ? (diff < (margin * DBL_MIN))
                 : ((diff / fmin((fabs(a) + fabs(b)), DBL_MAX)) < margin)));
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
#define error_margin 1.0e-6
status_t test_base() {
  status_init;
  test_helper_assert("input 0.5",
                     float_nearly_equal_p(0.63662, sp_sinc(0.5), error_margin));
  test_helper_assert("input 1",
                     float_nearly_equal_p(1.0, sp_sinc(0), error_margin));
  test_helper_assert("window-blackman 1.1 20",
                     float_nearly_equal_p(0.550175, sp_window_blackman(1.1, 20),
                                          error_margin));
exit:
  return (status);
};
status_t test_spectral_inversion_ir() {
  status_init;
  size_t a_len = 5;
  sp_sample_t a[5] = {0.1, -0.2, 0.3, -0.2, 0.1};
  sp_spectral_inversion_ir(a, a_len);
  test_helper_assert("result check",
                     (float_nearly_equal_p(-0.1, (*(a + 0)), error_margin) &&
                      float_nearly_equal_p(0.2, (*(a + 1)), error_margin) &&
                      float_nearly_equal_p(-0.3, (*(a + 2)), error_margin) &&
                      float_nearly_equal_p(0.2, (*(a + 3)), error_margin) &&
                      float_nearly_equal_p(-0.1, (*(a + 4)), error_margin)));
exit:
  return (status);
};
status_t test_spectral_reversal_ir() {
  status_init;
  size_t a_len = 5;
  sp_sample_t a[5] = {0.1, -0.2, 0.3, -0.2, 0.1};
  sp_spectral_reversal_ir(a, a_len);
  test_helper_assert("result check",
                     (float_nearly_equal_p(0.1, (*(a + 0)), error_margin) &&
                      float_nearly_equal_p(0.2, (*(a + 1)), error_margin) &&
                      float_nearly_equal_p(0.3, (*(a + 2)), error_margin) &&
                      float_nearly_equal_p(0.2, (*(a + 3)), error_margin) &&
                      float_nearly_equal_p(0.1, (*(a + 4)), error_margin)));
exit:
  return (status);
};
b8 *test_file_path = "/tmp/test-sph-sp-file";
status_t test_port() {
  status_init;
  local_memory_init(1);
  if (file_exists_p(test_file_path)) {
    unlink(test_file_path);
  };
  b32 channel_count = 2;
  b32 sample_rate = 8000;
  sp_port_t port;
  status_require_x(
      sp_file_open(&port, test_file_path, channel_count, sample_rate));
  debug_log("type %d", port.type);
  sp_sample_t **channel_data = sp_alloc_channel_data(channel_count, 5);
  sp_status_require_alloc(channel_data);
  local_memory_add(channel_data);
  b32 sample_count = 5;
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
  status_require_x(sp_port_write(&port, sample_count, channel_data));
  status_require_x(sp_port_close(&port));
  status_require_x(sp_file_open(&port, test_file_path, 2, 8000));
  status_require_x(sp_port_close(&port));
exit:
  local_memory_free;
  return (status);
};
int main() {
  status_init;
  test_helper_test_one(test_port);
exit:
  test_helper_display_summary();
  return (status.id);
};