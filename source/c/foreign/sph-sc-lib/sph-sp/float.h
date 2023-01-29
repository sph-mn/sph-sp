
#ifndef sph_float_h
#define sph_float_h

#include <inttypes.h>
#define sph_define_float_sum(prefix, type) \
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
#define sph_define_float_array_nearly_equal(prefix, type) \
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
uint8_t sph_f64_nearly_equal(double a, double b, double margin);
uint8_t sph_f32_nearly_equal(float a, float b, float margin);
uint8_t sph_f32_array_nearly_equal(float* a, size_t a_len, float* b, size_t b_len, float error_margin);
uint8_t sph_f64_array_nearly_equal(double* a, size_t a_len, double* b, size_t b_len, double error_margin);
float sph_f32_sum(float* numbers, size_t len);
double sph_f64_sum(double* numbers, size_t len);
#endif
