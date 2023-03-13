
#ifndef sph_spline_path_h
#define sph_spline_path_h

/* * spline-path creates discrete 2d paths interpolated between some given points
 * maps from one independent value to one dependent continuous value
 * only the dependent value is returned
 * kept minimal (only 2d, only selected interpolators, limited segment count) to be extremely fast
 * negative independent values are not supported
 * segments-count must be greater than zero
 * multidimensional interpolation could only be archieved with multiple configs and calls
 * a copy of segments is made internally and only the copy is used
 * uses points as structs because pre-defined size arrays can not be used in structs
 * segments must be a valid spline-path segment configuration
 * interpolators are called with path-relative start/end inside segment and with out positioned at offset for this start/end block
 * all segment types require a fixed number of given points. line: 1, bezier: 3, move: 1, constant: 0, path: 0
 * segments start at the previous point or (0 0)
 * bezier interpolation assume that output array values are set to zero before use
 * segments draw from the start point inclusive to end point exclusive
 * both dimensions are float types for precision with internal calculations */
#include <inttypes.h>

/* spline-path-size-max must be a value that fits in spline-path-value-t and size-t */

#ifndef spline_path_value_t
#define spline_path_value_t double
#endif
#ifndef spline_path_segment_count_t
#define spline_path_segment_count_t uint16_t
#endif
#ifndef spline_path_size_max
#define spline_path_size_max (SIZE_MAX / 2)
#endif
#ifndef spline_path_segment_points_count
#define spline_path_segment_points_count(s) (((spline_path_i_bezier == s.interpolator) || (spline_path_i_bezier_arc == s.interpolator)) ? 3 : 1)
#endif
typedef struct {
  spline_path_value_t x;
  spline_path_value_t y;
} spline_path_point_t;
typedef void (*spline_path_interpolator_t)(size_t, size_t, spline_path_point_t, spline_path_point_t*, void**, spline_path_value_t*);
typedef struct {
  spline_path_point_t _start;
  uint8_t _points_count;
  spline_path_point_t points[3];
  spline_path_interpolator_t interpolator;
  void* data;
  void (*free)(void*);
} spline_path_segment_t;
typedef struct {
  spline_path_segment_count_t segments_count;
  spline_path_segment_t* segments;
  spline_path_segment_count_t current_segment;
} spline_path_t;
void spline_path_i_move(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out);
void spline_path_i_constant(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out);
void spline_path_i_line(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out);
void spline_path_i_bezier(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out);
void spline_path_get(spline_path_t* path, size_t start, size_t end, spline_path_value_t* out);
void spline_path_i_path(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out);
spline_path_point_t spline_path_start(spline_path_t path);
spline_path_point_t spline_path_end(spline_path_t path);
void spline_path_set(spline_path_t* path, spline_path_segment_t* segments, spline_path_segment_count_t segments_count);
uint8_t spline_path_set_copy(spline_path_t* path, spline_path_segment_t* segments, spline_path_segment_count_t segments_count);
uint8_t spline_path_segments_get(spline_path_segment_t* segments, spline_path_segment_count_t segments_count, size_t start, size_t end, spline_path_value_t* out);
spline_path_segment_t spline_path_move(spline_path_value_t x, spline_path_value_t y);
spline_path_segment_t spline_path_line(spline_path_value_t x, spline_path_value_t y);
spline_path_segment_t spline_path_bezier(spline_path_value_t x1, spline_path_value_t y1, spline_path_value_t x2, spline_path_value_t y2, spline_path_value_t x3, spline_path_value_t y3);
spline_path_segment_t spline_path_constant();
spline_path_segment_t spline_path_path(spline_path_t path);
void spline_path_prepare_segments(spline_path_segment_t* segments, spline_path_segment_count_t segments_count);
size_t spline_path_size(spline_path_t path);
void spline_path_free(spline_path_t path);
spline_path_point_t spline_path_perpendicular_point(spline_path_point_t p1, spline_path_point_t p2, spline_path_value_t distance_factor);
void spline_path_i_bezier_arc(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out);
spline_path_segment_t spline_path_bezier_arc(spline_path_value_t x, spline_path_value_t y, spline_path_value_t curvature);
#endif
