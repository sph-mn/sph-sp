/* depends on sph/types.c */
#include <float.h>
#include <math.h>
#define define_float_sum(prefix, type) \
  type prefix##_sum(type* numbers, size_t len) { \
    type temp; \
    type element; \
    type correction = 0; \
    len = (len - 1); \
    type result = numbers[len]; \
    while (len) { \
      len = (len - 1); \
      element = numbers[len]; \
      temp = (result + element); \
      correction = (correction + ((result >= element) ? ((result - temp) + element) : ((element - temp) + result))); \
      result = temp; \
    }; \
    return ((correction + result)); \
  }
#define define_float_array_nearly_equal(prefix, type) \
  uint8_t prefix##_array_nearly_equal(type* a, size_t a_len, type* b, size_t b_len, type error_margin) { \
    size_t index = 0; \
    if (!(a_len == b_len)) { \
      return (0); \
    }; \
    while ((index < a_len)) { \
      if (!prefix##_nearly_equal((a[index]), (b[index]), error_margin)) { \
        return (0); \
      }; \
      index = (1 + index); \
    }; \
    return (1); \
  }
/** approximate float comparison. margin is a factor and is low for low accepted differences.
   http://floating-point-gui.de/errors/comparison/ */
uint8_t f64_nearly_equal(f64 a, f64 b, f64 margin) {
  if (a == b) {
    return (1);
  } else {
    f64 diff = fabs((a - b));
    return ((((0 == a) || (0 == b) || (diff < DBL_MIN)) ? (diff < (margin * DBL_MIN)) : ((diff / fmin((fabs(a) + fabs(b)), DBL_MAX)) < margin)));
  };
};
/** approximate float comparison. margin is a factor and is low for low accepted differences.
   http://floating-point-gui.de/errors/comparison/ */
uint8_t f32_nearly_equal(f32 a, f32 b, f32 margin) {
  if (a == b) {
    return (1);
  } else {
    f32 diff = fabs((a - b));
    return ((((0 == a) || (0 == b) || (diff < FLT_MIN)) ? (diff < (margin * FLT_MIN)) : ((diff / fmin((fabs(a) + fabs(b)), FLT_MAX)) < margin)));
  };
};
define_float_array_nearly_equal(f32, f32);
define_float_array_nearly_equal(f64, f64);
define_float_sum(f32, f32);
define_float_sum(f64, f64);