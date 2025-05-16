
/** x length is point_count minus two.
   x contains only intermediate point x values x(0 + 1) to x(n - 1)
   the path will start at x 0 and end at x length.
   point_count must be greater one.
   y length is point_count.
   c length is point_count minus one.
   c is for curvature and values are between -1.0 and 1.0.
   c can be 0 in which case it is ignored */
status_t sp_path_samples(sp_sample_t** out, sp_time_t length, sp_path_point_count_t point_count, sp_sample_t* x, sp_sample_t* y, sp_sample_t* c) {
  status_declare;
  spline_path_t path;
  spline_path_segment_t segments[sp_path_point_count_limit];
  sp_path_point_count_t last_point_index = (point_count - 1);
  sp_sample_t xn = length;
  sp_sample_t yn = y[last_point_index];
  sp_sample_t cn = (c ? c[(last_point_index - 1)] : 0);
  segments[0] = spline_path_move(0, (y[0]));
  segments[last_point_index] = ((cn < 1.0e-5) ? spline_path_line(xn, yn) : spline_path_bezier_arc(xn, yn, cn));
  for (sp_path_point_count_t i = 1; (i < last_point_index); i += 1) {
    xn = x[(i - 1)];
    yn = y[i];
    cn = (c ? c[(i - 1)] : 0);
    segments[i] = ((cn < 1.0e-5) ? spline_path_line(xn, yn) : spline_path_bezier_arc(xn, yn, cn));
  };
  spline_path_set((&path), segments, point_count);
  status_require((sp_samples_new(length, out)));
  spline_path_get((&path), 0, length, (*out));
exit:
  status_return;
}

/** c is for curvature and values are between -1.0 and 1.0 */
status_t sp_path_times(sp_time_t** out, sp_time_t length, sp_path_point_count_t point_count, sp_sample_t* x, sp_sample_t* y, sp_sample_t* c) {
  status_declare;
  sp_sample_t* temp;
  status_require((sp_path_samples((&temp), length, point_count, x, y, c)));
  status_require((sp_samples_to_times_replace(temp, length, out)));
exit:
  status_return;
}
#define sp_define_path_n(type_name, type) \
  status_t sp_path_##type_name##2(type * *out, sp_time_t length, sp_sample_t y1, sp_sample_t y2) { \
    sp_sample_t y[2]; \
    y[0] = y1; \
    y[1] = y2; \
    sp_path_##type_name(out, length, 2, 0, y, 0); \
  } \
  status_t sp_path_##type_name##3(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3) { \
    sp_sample_t y[3]; \
    y[0] = y1; \
    y[1] = y2; \
    y[2] = y3; \
    sp_path_##type_name(out, length, 3, (&x1), y, 0); \
  } \
  status_t sp_path_##type_name##4(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4) { \
    sp_sample_t x[2]; \
    sp_sample_t y[4]; \
    x[0] = x1; \
    x[1] = x2; \
    y[0] = y1; \
    y[1] = y2; \
    y[2] = y3; \
    y[3] = y4; \
    sp_path_##type_name(out, length, 4, x, y, 0); \
  } \
  status_t sp_path_##type_name##5(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t y5) { \
    sp_sample_t x[3]; \
    sp_sample_t y[5]; \
    x[0] = x1; \
    x[1] = x2; \
    x[2] = x3; \
    y[0] = y1; \
    y[1] = y2; \
    y[2] = y3; \
    y[3] = y4; \
    y[4] = y5; \
    sp_path_##type_name(out, length, 5, x, y, 0); \
  } \
  status_t sp_path_##type_name##_curve##2(type * *out, sp_time_t length, sp_sample_t y1, sp_sample_t y2, sp_sample_t c1) { \
    sp_sample_t y[2]; \
    y[0] = y1; \
    y[1] = y2; \
    sp_path_##type_name(out, length, 2, 0, y, (&c1)); \
  } \
  status_t sp_path_##type_name##_curve##3(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t c1, sp_sample_t c2) { \
    sp_sample_t y[3]; \
    sp_sample_t c[2]; \
    y[0] = y1; \
    y[1] = y2; \
    y[2] = y3; \
    c[0] = c1; \
    c[1] = c2; \
    sp_path_##type_name(out, length, 3, (&x1), y, c); \
  } \
  status_t sp_path_##type_name##_curve##4(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3) { \
    sp_sample_t x[2]; \
    sp_sample_t y[4]; \
    sp_sample_t c[3]; \
    x[0] = x1; \
    x[1] = x2; \
    y[0] = y1; \
    y[1] = y2; \
    y[2] = y3; \
    y[3] = y4; \
    c[0] = c1; \
    c[1] = c2; \
    c[2] = c3; \
    sp_path_##type_name(out, length, 4, x, y, c); \
  } \
  status_t sp_path_##type_name##_curve##5(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t y5, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3, sp_sample_t c4) { \
    sp_sample_t x[3]; \
    sp_sample_t y[5]; \
    sp_sample_t c[4]; \
    x[0] = x1; \
    x[1] = x2; \
    x[2] = x3; \
    y[0] = y1; \
    y[1] = y2; \
    y[2] = y3; \
    y[3] = y4; \
    y[4] = y5; \
    c[0] = c1; \
    c[1] = c2; \
    c[2] = c3; \
    c[3] = c4; \
    sp_path_##type_name(out, length, 5, x, y, c); \
  }
sp_define_path_n(samples, sp_sample_t)
  sp_define_path_n(times, sp_time_t)

  /** from a first implicit y value 0 to a final implicit y value 0.
     only the intermediate points are provided.
     x, y and c length is point_count minus two.
     x values will be cumulative (0.1, 0.2) -> 0.3.
     x values will be multiplied by length. */
  status_t sp_envelope_zero(sp_sample_t** out, sp_time_t length, sp_path_point_count_t point_count, sp_sample_t* x, sp_sample_t* y, sp_sample_t* c) {
  sp_sample_t path_y[sp_path_point_count_limit];
  x[1] *= length;
  path_y[0] = 0;
  path_y[(point_count - 1)] = 0;
  for (sp_path_point_count_t i = 2; (i < (point_count - 1)); i += 1) {
    x[i] = (x[(i - 1)] + (x[i] * length));
    path_y[i] = y[i];
  };
  return ((sp_path_samples(out, length, point_count, x, path_y, c)));
}
#define sp_define_envelope_zero_n(prefix, type_name, type) \
  status_t prefix##3(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t y1) { return ((sp_path_##type_name##3(out, length, (x1 * length), 0, y1, 0))); } \
  status_t prefix##4(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2) { return ((sp_path_##type_name##4(out, length, (x1 * length), ((x1 * length) + (x2 * length)), 0, y1, y2, 0))); } \
  status_t prefix##5(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3) { \
    x1 *= length; \
    x2 = (x1 + (x2 * length)); \
    x3 = (x2 + (x3 * length)); \
    return ((sp_path_##type_name##5(out, length, x1, x2, x3, 0, y1, y2, y3, 0))); \
  } \
  status_t prefix##_curve##3(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t y1, sp_sample_t c1, sp_sample_t c2) { return ((sp_path_##type_name##_curve##3(out, length, (x1 * length), 0, y1, 0, c1, c2))); } \
  status_t prefix##_curve##4(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3) { return ((sp_path_##type_name##_curve##4(out, length, (x1 * length), ((x1 * length) + (x2 * length)), 0, y1, y2, 0, c1, c2, c3))); } \
  status_t prefix##_curve##5(type * *out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3, sp_sample_t c4) { \
    x1 *= length; \
    x2 = (x1 + (x2 * length)); \
    x3 = (x2 + (x3 * length)); \
    return ((sp_path_##type_name##_curve##5(out, length, x1, x2, x3, 0, y1, y2, y3, 0, c1, c2, c3, c4))); \
  }
sp_define_envelope_zero_n(sp_envelope_zero, samples, sp_sample_t)

  /** y and c length is point_count.
     x length is point_count minus two.
     x values will be cumulative (0.1, 0.2) -> 0.3.
     x values will be multiplied by length. */
  status_t sp_envelope_scale(sp_time_t** out, sp_time_t length, sp_path_point_count_t point_count, sp_sample_t y_scalar, sp_sample_t* x, sp_sample_t* y, sp_sample_t* c) {
  sp_sample_t path_y[sp_path_point_count_limit];
  x[1] *= length;
  path_y[0] = (y_scalar * y[0]);
  path_y[(point_count - 1)] = (y_scalar * y[(point_count - 1)]);
  for (sp_path_point_count_t i = 2; (i < (point_count - 1)); i += 1) {
    x[i] = (x[(i - 1)] + (x[i] * length));
    path_y[i] = (y_scalar * y[i]);
  };
  return ((sp_path_times(out, length, point_count, x, path_y, c)));
}
#define sp_define_envelope_scale_n(prefix, type_name, type) \
  status_t prefix##3(type * *out, sp_time_t length, sp_sample_t y_scalar, sp_sample_t x1, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3) { return ((sp_path_##type_name##3(out, length, (x1 * length), (y1 * y_scalar), (y2 * y_scalar), (y3 * y_scalar)))); } \
  status_t prefix##4(type * *out, sp_time_t length, sp_sample_t y_scalar, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4) { return ((sp_path_##type_name##4(out, length, (x1 * length), ((x1 * length) + (x2 * length)), (y_scalar * y1), (y_scalar * y2), (y_scalar * y3), (y_scalar * y4)))); } \
  status_t prefix##5(type * *out, sp_time_t length, sp_sample_t y_scalar, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t y5) { \
    x1 *= length; \
    x2 = (x1 + (x2 * length)); \
    x3 = (x2 + (x3 * length)); \
    return ((sp_path_##type_name##5(out, length, x1, x2, x3, (y_scalar * y1), (y_scalar * y2), (y_scalar * y3), (y_scalar * y4), (y_scalar * y5)))); \
  }
sp_define_envelope_scale_n(sp_envelope_scale, times, sp_time_t)
