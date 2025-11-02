
#ifndef sph_float_c_included
#define sph_float_c_included

#include <math.h>
#include <sph-sp/sph/float.h>

/** approximate float comparison. margin is a factor and is low for low accepted differences */
uint8_t sph_f64_nearly_equal(double a, double b, double margin) { return ((fabs((a - b)) < margin)); }

/** approximate float comparison. margin is a factor and is low for low accepted differences */
uint8_t sph_f32_nearly_equal(float a, float b, float margin) { return ((fabs((a - b)) < margin)); }
sph_define_float_array_nearly_equal(sph_f32, float)
  sph_define_float_array_nearly_equal(sph_f64, double)
    sph_define_float_sum(sph_f32, float)
      sph_define_float_sum(sph_f64, double)
#endif
