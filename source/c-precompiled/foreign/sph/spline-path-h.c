/* * spline-path creates discrete 2d paths interpolated between some given points
   * maps from one independent discrete value to one dependent continuous value
   * only the dependent value is returned
   * kept minimal (only 2d, only selected interpolators, limited segment count) to be extremely fast
   * multidimensional interpolation can be archieved with multiple configs and calls
   * a copy of segments is made internally and only the copy is used
   * uses points as structs because pre-defined size arrays can not be used in structs
   * segments-count must be greater than zero
   * segments must be a valid spline-path segment configuration
   * interpolators are called with path-relative start/end inside segment and with out positioned at the segment output
   * all segment types require a fixed number of given points. line: 1, bezier: 3, move: 1, constant: 0, path: 0
   * negative x values not supported
   * internally all segments start at (0 0) and no gaps are between segments
   * assumes that bit 0 is spline-path-value-t zero
   * segments draw to the endpoint inclusive, start point exclusive
   * spline-path-interpolator-points-count */
#include <inttypes.h>
#include <strings.h>
#include <stdlib.h>
#ifndef spline_path_time_t
#define spline_path_time_t uint32_t
#endif
#ifndef spline_path_value_t
#define spline_path_value_t double
#endif
#ifndef spline_path_segment_count_t
#define spline_path_segment_count_t uint16_t
#endif
#ifndef spline_path_time_max
#define spline_path_time_max UINT32_MAX
#endif
typedef struct {
  spline_path_time_t x;
  spline_path_value_t y;
} spline_path_point_t;
typedef void (*spline_path_interpolator_t)(spline_path_time_t, spline_path_time_t, spline_path_point_t, spline_path_point_t*, void*, spline_path_value_t*);
typedef struct {
  spline_path_point_t _start;
  uint8_t _points_count;
  spline_path_point_t points[3];
  spline_path_interpolator_t interpolator;
  void* options;
} spline_path_segment_t;
typedef struct {
  spline_path_segment_count_t segments_count;
  spline_path_segment_t* segments;
} spline_path_t;
void spline_path_i_move(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_i_constant(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_i_line(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_i_bezier(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_get(spline_path_t path, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out);
void spline_path_i_path(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
spline_path_point_t spline_path_start(spline_path_t path);
spline_path_point_t spline_path_end(spline_path_t path);
uint8_t spline_path_new(spline_path_segment_count_t segments_count, spline_path_segment_t* segments, spline_path_t* out_path);
void spline_path_free(spline_path_t a);
uint8_t spline_path_new_get(spline_path_segment_count_t segments_count, spline_path_segment_t* segments, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out);
spline_path_segment_t spline_path_move(spline_path_time_t x, spline_path_value_t y);
spline_path_segment_t spline_path_line(spline_path_time_t x, spline_path_value_t y);
spline_path_segment_t spline_path_bezier(spline_path_time_t x1, spline_path_value_t y1, spline_path_time_t x2, spline_path_value_t y2, spline_path_time_t x3, spline_path_value_t y3);
spline_path_segment_t spline_path_constant();
spline_path_segment_t spline_path_path(spline_path_t* path);