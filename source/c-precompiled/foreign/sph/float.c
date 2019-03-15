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
/** approximate float comparison. margin is a factor and is low for low accepted differences */
uint8_t f64_nearly_equal(double a, double b, double margin) { return ((fabs((a - b)) < margin)); };
/** approximate float comparison. margin is a factor and is low for low accepted differences */
uint8_t f32_nearly_equal(float a, float b, float margin) { return ((fabs((a - b)) < margin)); };
define_float_array_nearly_equal(f32, float);
define_float_array_nearly_equal(f64, double);
define_float_sum(f32, float);
define_float_sum(f64, double);