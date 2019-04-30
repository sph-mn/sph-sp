/* * spline-path creates discrete 2d paths interpolated between some given points
  * maps from one independent discrete value to one dependent continuous value
  * only the dependent value is returned
  * kept minimal (only 2d, only selected interpolators, limited segment count) to be extremely fast
  * multidimensional interpolation can be archieved with multiple configs and calls
  * a copy of segments is made internally and only the copy is used
  * uses points as structs because pre-defined size arrays can not be used in structs
  * segments-len must be greater than zero
  * segments must be a valid spline-path segment configuration
  * interpolators are called with path-relative start/end inside segment and with out positioned at the segment output
  * all segment types have a fixed number of points. line: 1, bezier: 3, move: 1, constant: 0, path: 0
  * negative x values not supported
  * internally all segments start at (0 0) and no gaps are between segments
  * assumes that bit 0 is spline-path-value-t zero
  * segments draw to the endpoint inclusive, start point exclusive */
#include <inttypes.h>
#include <strings.h>
#include <stdlib.h>
#ifndef spline_path_time_t
#define spline_path_time_t uint32_t
#endif
#ifndef spline_path_value_t
#define spline_path_value_t double
#endif
#ifndef spline_path_point_count_t
#define spline_path_point_count_t uint8_t
#endif
#ifndef spline_path_segment_count_t
#define spline_path_segment_count_t uint16_t
#endif
#ifndef spline_path_time_max
#define spline_path_time_max UINT32_MAX
#endif
#define spline_path_point_limit 3
#define spline_path_interpolator_points_len(a) ((spline_path_i_bezier == a) ? 3 : 1)
typedef struct {
  spline_path_value_t x;
  spline_path_value_t y;
} spline_path_point_t;
typedef void (*spline_path_interpolator_t)(spline_path_time_t, spline_path_time_t, spline_path_point_t, spline_path_point_t*, void*, spline_path_value_t*);
typedef struct {
  spline_path_point_t _start;
  spline_path_point_count_t _points_len;
  spline_path_point_t points[spline_path_point_limit];
  spline_path_interpolator_t interpolator;
  void* options;
} spline_path_segment_t;
typedef struct {
  spline_path_segment_count_t segments_len;
  spline_path_segment_t* segments;
} spline_path_t;
void spline_path_i_line(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_free(spline_path_t a);
uint8_t spline_path_new(spline_path_segment_count_t segments_len, spline_path_segment_t* segments, spline_path_t* out_path);
void spline_path_get(spline_path_t path, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out);
uint8_t spline_path_new_get(spline_path_segment_count_t segments_len, spline_path_segment_t* segments, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out);
void spline_path_i_move(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_i_constant(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_i_bezier(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
void spline_path_i_path(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out);
spline_path_point_t spline_path_start(spline_path_t path);
spline_path_point_t spline_path_end(spline_path_t path);
/** p-rest length 1 */
void spline_path_i_move(spline_path_time_t start, spline_path_time_t size, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) { memset(out, 0, (sizeof(spline_path_value_t) * size)); };
/** p-rest length 0 */
void spline_path_i_constant(spline_path_time_t start, spline_path_time_t size, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) {
  spline_path_time_t i;
  for (i = 0; (i < size); i = (1 + i)) {
    out[i] = p_start.y;
  };
};
/** p-rest length 1 */
void spline_path_i_line(spline_path_time_t start, spline_path_time_t size, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) {
  spline_path_time_t i;
  spline_path_point_t p_end;
  spline_path_value_t s_size;
  spline_path_time_t s_relative_start;
  spline_path_value_t t;
  p_end = p_rest[0];
  s_size = (p_end.x - p_start.x);
  s_relative_start = (start - p_start.x);
  for (i = 0; (i < size); i = (1 + i)) {
    t = ((s_relative_start + i) / s_size);
    out[i] = ((p_end.y * t) + (p_start.y * (1 - t)));
  };
};
/** p-rest length 3 */
void spline_path_i_bezier(spline_path_time_t start, spline_path_time_t size, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) {
  spline_path_time_t i;
  spline_path_value_t mt;
  spline_path_point_t p_end;
  spline_path_value_t s_size;
  spline_path_time_t s_relative_start;
  spline_path_value_t t;
  p_end = p_rest[2];
  s_size = (p_end.x - p_start.x);
  s_relative_start = (start - p_start.x);
  for (i = 0; (i < size); i = (1 + i)) {
    t = ((s_relative_start + i) / s_size);
    mt = (1 - t);
    out[i] = ((p_start.y * mt * mt * mt) + ((p_rest[0]).y * 3 * mt * mt * t) + ((p_rest[1]).y * 3 * mt * t * t) + (p_end.y * t * t * t));
  };
};
/** p-rest length 0. options is one spline-path-t */
void spline_path_i_path(spline_path_time_t start, spline_path_time_t size, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) { spline_path_get((*((spline_path_t*)(options))), (start - p_start.x), ((start - p_start.x) + size), out); };
spline_path_point_t spline_path_start(spline_path_t path) {
  spline_path_point_t p;
  spline_path_segment_t s;
  s = (path.segments)[0];
  if (spline_path_i_move == s.interpolator) {
    p = (s.points)[0];
  } else {
    p.x = 0;
    p.y = 0;
  };
  return (p);
};
spline_path_point_t spline_path_end(spline_path_t path) {
  spline_path_segment_t s;
  s = (path.segments)[(path.segments_len - 1)];
  if (spline_path_i_constant == s.interpolator) {
    s = (path.segments)[(path.segments_len - 2)];
  };
  return (((s.points)[(s._points_len - 1)]));
};
uint8_t spline_path_new(spline_path_segment_count_t segments_len, spline_path_segment_t* segments, spline_path_t* out_path) {
  spline_path_segment_count_t i;
  spline_path_t path;
  spline_path_segment_t s;
  spline_path_point_t start;
  start.x = 0;
  start.y = 0;
  path.segments = malloc((segments_len * sizeof(spline_path_segment_t)));
  if (!path.segments) {
    return (1);
  };
  memcpy((path.segments), segments, (segments_len * sizeof(spline_path_segment_t)));
  for (i = 0; (i < segments_len); i = (1 + i)) {
    ((path.segments)[i])._start = start;
    s = (path.segments)[i];
    s._points_len = spline_path_interpolator_points_len((s.interpolator));
    if ((spline_path_i_path == s.interpolator)) {
      start = spline_path_end((*((spline_path_t*)(s.options))));
      *(s.points) = start;
    } else if ((spline_path_i_constant == s.interpolator)) {
      *(s.points) = start;
      (s.points)->x = spline_path_time_max;
    } else {
      start = (s.points)[(s._points_len - 1)];
    };
    (path.segments)[i] = s;
  };
  path.segments_len = segments_len;
  *out_path = path;
  return (0);
};
/** get values on path between start (inclusive) and end (exclusive) */
void spline_path_get(spline_path_t path, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out) {
  /* write segment start first, then the segment such that it ends before the next start */
  spline_path_segment_count_t i;
  spline_path_segment_t s;
  spline_path_time_t s_start;
  spline_path_time_t s_end;
  spline_path_segment_count_t t;
  t = start;
  for (i = 0; (i < path.segments_len); i = (1 + i)) {
    s = (path.segments)[i];
    s_start = s._start.x;
    if (end < s_start) {
      break;
    };
    s_end = ((s.points)[(s._points_len - 1)]).x;
    if (t > s_end) {
      continue;
    };
    if (t == s_start) {
      out[(t - start)] = s._start.y;
      t = (1 + t);
    };
    (s.interpolator)(t, (((end > s_end) ? s_end : end) - t), (s._start), (s.points), (s.options), ((t - start) + out));
    t = s_end;
  };
  if (t <= end) {
    (s.interpolator)(t, 1, (s._start), (s.points), (s.options), ((t - start) + out));
    t = (1 + t);
    if (t <= end) {
      memset(((t - start) + out), 0, (sizeof(spline_path_value_t) * (end - t)));
    };
  };
};
void spline_path_free(spline_path_t a) { free((a.segments)); };
/** create a path array immediately from segments without creating a path object */
uint8_t spline_path_new_get(spline_path_segment_count_t segments_len, spline_path_segment_t* segments, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out) {
  spline_path_t path;
  if (spline_path_new(segments_len, segments, (&path))) {
    return (1);
  };
  spline_path_get(path, start, end, out);
  spline_path_free(path);
  return (0);
};