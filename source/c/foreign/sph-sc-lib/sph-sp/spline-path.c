
#include <float.h>
#include <strings.h>
#include <stdlib.h>
#include <math.h>

#define spline_path_min(a, b) ((a < b) ? a : b)
#define spline_path_max(a, b) ((a > b) ? a : b)
#define spline_path_abs(a) ((0 > a) ? (-1 * a) : a)
#define spline_path_cheap_round_positive(a) ((size_t)((0.5 + a)))
#define spline_path_limit(x, min_value, max_value) spline_path_max(min_value, (spline_path_min(max_value, x)))
#define spline_path_i_bezier_interpolate(mt, t, a, b, c, d) ((a * mt * mt * mt) + (b * 3 * mt * mt * t) + (c * 3 * mt * t * t) + (d * t * t * t))
#define lerp(a, b, t) (a + (t * (b - a)))
#define convert_point_x(x, x_min, x_max) ((size_t)(spline_path_cheap_round_positive((spline_path_limit(x, x_min, x_max)))))

/** return a point on a perpendicular line across the midpoint of a line between p1 and p2.
   can be used to find control points.
   distance-factor at -1 and 1 are the bounds of a rectangle where p1 and p2 are diagonally opposed edges,
   so that the distance will not be below or above the x and y values of the given points.
   calculates the midpoint, the negative reciprocal slope and a unit vector */
spline_path_point_t spline_path_perpendicular_point(spline_path_point_t p1, spline_path_point_t p2, spline_path_value_t distance_factor) {
  spline_path_value_t dx;
  spline_path_value_t dy;
  spline_path_value_t mx;
  spline_path_value_t my;
  spline_path_value_t d;
  spline_path_value_t ux;
  spline_path_value_t uy;
  spline_path_value_t scale;
  spline_path_point_t result;
  dx = (p2.x - p1.x);
  dy = (p2.y - p1.y);
  mx = ((p1.x + p2.x) / 2);
  my = ((p1.y + p2.y) / 2);
  d = sqrt(((dx * dx) + (dy * dy)));
  ux = ((-dy) / d);
  uy = (dx / d);
  scale = (distance_factor * (spline_path_min((spline_path_abs(dx)), (spline_path_abs(dy))) / 4.0));
  result.x = (mx + (ux * scale));
  result.y = (my + (uy * scale));
  return (result);
}

/** add missing intermediate points by interpolating between neighboring points.
   for interpolation methods that return float values for x that dont map directly to the currently interpolated index
   and may therefore leave gaps.
   assumes unset output values are 0 */
void spline_path_set_missing_points(spline_path_value_t* out, size_t start, size_t end) {
  size_t i2;
  size_t b_size;
  b_size = (end - start);
  for (size_t i = 1; (i < b_size); i += 1) {
    if (0.0 == out[i]) {
      i2 = (i + 1);
      while ((i2 < b_size)) {
        if (0.0 == out[i2]) {
          i2 += 1;
        } else {
          break;
        };
      };
      if (i2 < b_size) {
        out[i] = lerp((out[(i - 1)]), (out[i2]), (0.5 / (i2 - i)));
      };
    };
  };
}

/** p-rest length 1 */
void spline_path_i_move(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out) { memset(out, 0, (sizeof(spline_path_value_t) * (end - start))); }

/** p-rest length 0 */
void spline_path_i_constant(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out) {
  for (size_t i = 0; (i < (end - start)); i += 1) {
    out[i] = p_start.y;
  };
}

/** p-rest length 1 */
void spline_path_i_line(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out) {
  spline_path_point_t p_end;
  spline_path_value_t t;
  size_t p_start_x;
  spline_path_value_t s_size;
  size_t s_offset;
  p_start_x = p_start.x;
  p_end = p_rest[0];
  s_size = (p_end.x - p_start.x);
  s_offset = (start - p_start.x);
  for (size_t i = 0; (i < (end - start)); i += 1) {
    t = ((i + s_offset) / s_size);
    out[i] = lerp((p_start.y), (p_end.y), t);
  };
}

/** p-rest length 3 */
void spline_path_i_bezier(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out) {
  size_t b_size;
  size_t i;
  size_t ix;
  spline_path_value_t mt;
  spline_path_point_t p_end;
  spline_path_point_t p;
  size_t s_offset;
  spline_path_value_t s_size;
  spline_path_value_t t;
  p_end = p_rest[2];
  s_size = (p_end.x - p_start.x);
  s_offset = (start - p_start.x);
  b_size = (end - start);
  i = 0;
  ix = i;
  while ((ix < b_size)) {
    t = ((i + s_offset) / s_size);
    if (t >= 1.0) {
      break;
    };
    mt = (1 - t);
    p.x = spline_path_i_bezier_interpolate(mt, t, (p_start.x), ((p_rest[0]).x), ((p_rest[1]).x), (p_end.x));
    p.y = spline_path_i_bezier_interpolate(mt, t, (p_start.y), ((p_rest[0]).y), ((p_rest[1]).y), (p_end.y));
    ix = (convert_point_x((p.x), start, end) - start);
    out[ix] = p.y;
    i += 1;
  };
  spline_path_set_missing_points(out, start, end);
}
void spline_path_i_bezier_arc(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out) {
  if (!*data) {
    spline_path_value_t distance;
    spline_path_point_t p_end;
    spline_path_point_t p_temp;
    spline_path_value_t x_distance;
    spline_path_value_t y_distance;
    distance = (p_rest->y * (4.0 / 3.0) * (sqrt((2.0)) - 1.0));
    p_end = p_rest[2];
    x_distance = ((p_end.x > p_start.x) ? (p_end.x - p_start.x) : (p_start.x - p_end.x));
    y_distance = ((p_end.y > p_start.y) ? (p_end.y - p_start.y) : (p_start.y - p_end.y));
    p_temp.x = (p_end.x + x_distance);
    p_temp.y = (p_end.y + y_distance);
    p_rest[1] = spline_path_perpendicular_point(p_start, p_temp, distance);
    p_temp.x = (p_start.x - x_distance);
    p_temp.y = (p_start.y - y_distance);
    p_rest[0] = spline_path_perpendicular_point(p_temp, p_end, distance);
    *data = ((void*)(1));
  };
  spline_path_i_bezier(start, end, p_start, p_rest, data, out);
}

/** get values on path between start (inclusive) and end (exclusive).
   since x values are integers, a path from (0 0) to (10 20) for example would have reached 20 only at the 11th point.
   out memory is managed by the caller. the size required for out is end minus start */
void spline_path_get(spline_path_t* path, size_t start, size_t end, spline_path_value_t* out) {
  /* find all segments that overlap with requested range */
  spline_path_segment_count_t i;
  spline_path_segment_t s;
  size_t s_start;
  size_t s_end;
  size_t out_start;
  spline_path_segment_count_t segments_count;
  uint8_t second_search;
  segments_count = path->segments_count;
  i = path->current_segment;
  second_search = 0;
  while ((i < segments_count)) {
    s = (path->segments)[i];
    s_start = s._start.x;
    s_end = ((s.points)[(s._points_count - 1)]).x;
    if (s_start > end) {
      if (second_search || (0 == i)) {
        break;
      } else {
        /* to allow randomly ordered access */
        i = 0;
        second_search = 1;
        continue;
      };
    };
    if (s_end < start) {
      i += 1;
      continue;
    };
    path->current_segment = i;
    out_start = ((s_start > start) ? (s_start - start) : 0);
    s_start = ((s_start > start) ? s_start : start);
    s_end = ((s_end < end) ? s_end : end);
    (s.interpolator)(s_start, s_end, (s._start), (s.points), (&(s.data)), (out_start + out));
    i += 1;
  };
  /* outside points are zero */
  if (end > s_end) {
    out_start = ((start > s_end) ? 0 : (s_end - start));
    memset((out + out_start), 0, ((end - out_start) * sizeof(spline_path_value_t)));
  };
}

/** p-rest length 0. data is one spline-path-t */
void spline_path_i_path(size_t start, size_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void** data, spline_path_value_t* out) { spline_path_get((*data), (start - p_start.x), (end - p_start.x), out); }
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
}

/** ends at constants */
spline_path_point_t spline_path_end(spline_path_t path) {
  spline_path_segment_t s;
  s = (path.segments)[(path.segments_count - 1)];
  if (spline_path_i_constant == s.interpolator) {
    s = (path.segments)[(path.segments_count - 2)];
  };
  return (((s.points)[(s._points_count - 1)]));
}
size_t spline_path_size(spline_path_t path) {
  spline_path_point_t p;
  p = spline_path_end(path);
  return ((p.x));
}

/** set _start and _points_count for segments */
void spline_path_prepare_segments(spline_path_segment_t* segments, spline_path_segment_count_t segments_count) {
  spline_path_segment_count_t i;
  spline_path_segment_t s;
  spline_path_point_t start;
  start.x = 0;
  start.y = 0;
  for (i = 0; (i < segments_count); i += 1) {
    (segments[i])._start = start;
    s = segments[i];
    s._points_count = spline_path_segment_points_count(s);
    if (spline_path_i_path == s.interpolator) {
      start = spline_path_end((*((spline_path_t*)(s.data))));
      *(s.points) = start;
    } else if (spline_path_i_constant == s.interpolator) {
      (s.points)->x = spline_path_size_max;
      (s.points)->y = start.y;
    } else {
      start = (s.points)[(s._points_count - 1)];
    };
    segments[i] = s;
  };
}

/** set segments for a path and initialise it */
void spline_path_set(spline_path_t* path, spline_path_segment_t* segments, spline_path_segment_count_t segments_count) {
  spline_path_prepare_segments(segments, segments_count);
  path->segments = segments;
  path->segments_count = segments_count;
  path->current_segment = 0;
}

/** like spline-path-set but copies segments to new memory in .segments that has to be freed
   when not needed anymore */
uint8_t spline_path_set_copy(spline_path_t* path, spline_path_segment_t* segments, spline_path_segment_count_t segments_count) {
  spline_path_segment_t* s = malloc((segments_count * sizeof(spline_path_segment_t)));
  if (!s) {
    return (1);
  };
  memcpy(s, segments, (segments_count * sizeof(spline_path_segment_t)));
  spline_path_set(path, s, segments_count);
  return (0);
}

/** create a path array immediately from segments without creating a path object */
uint8_t spline_path_segments_get(spline_path_segment_t* segments, spline_path_segment_count_t segments_count, size_t start, size_t end, spline_path_value_t* out) {
  spline_path_t path;
  spline_path_set((&path), segments, segments_count);
  spline_path_get((&path), start, end, out);
  return (0);
}

/** returns a move segment to move to the specified point */
spline_path_segment_t spline_path_move(spline_path_value_t x, spline_path_value_t y) {
  spline_path_segment_t s;
  s.interpolator = spline_path_i_move;
  (s.points)->x = x;
  (s.points)->y = y;
  s.free = 0;
  return (s);
}
spline_path_segment_t spline_path_line(spline_path_value_t x, spline_path_value_t y) {
  spline_path_segment_t s;
  s.interpolator = spline_path_i_line;
  (s.points)->x = x;
  (s.points)->y = y;
  s.free = 0;
  return (s);
}

/** the first two points are the control points */
spline_path_segment_t spline_path_bezier(spline_path_value_t x1, spline_path_value_t y1, spline_path_value_t x2, spline_path_value_t y2, spline_path_value_t x3, spline_path_value_t y3) {
  spline_path_segment_t s;
  s.free = 0;
  s.interpolator = spline_path_i_bezier;
  (s.points)->x = x1;
  (s.points)->y = y1;
  (1 + s.points)->x = x2;
  (1 + s.points)->y = y2;
  (2 + s.points)->x = x3;
  (2 + s.points)->y = y3;
  return (s);
}

/** curvature is a real between -1..1, with the maximum being the edge of the segment */
spline_path_segment_t spline_path_bezier_arc(spline_path_value_t curvature, spline_path_value_t x, spline_path_value_t y) {
  spline_path_segment_t s;
  s.free = 0;
  s.data = 0;
  s.interpolator = spline_path_i_bezier_arc;
  (s.points)->y = curvature;
  (2 + s.points)->x = x;
  (2 + s.points)->y = y;
  return (s);
}
spline_path_segment_t spline_path_constant() {
  spline_path_segment_t s;
  s.interpolator = spline_path_i_constant;
  s.free = 0;
  return (s);
}

/** return a segment that is another spline-path. length is the full length of the path.
   the path does not necessarily connect and is drawn as it would be on its own starting from the preceding segment */
spline_path_segment_t spline_path_path(spline_path_t path) {
  spline_path_segment_t s;
  s.data = malloc((sizeof(spline_path_t)));
  if (s.data) {
    *((spline_path_t*)(s.data)) = path;
    s.interpolator = spline_path_i_path;
    s.free = free;
  } else {
    s.interpolator = spline_path_i_constant;
    s.free = 0;
  };
  return (s);
}

/** only needs to be called if a segment with state has been used,
   which is currently only spline_path_path */
void spline_path_free(spline_path_t path) {
  spline_path_segment_t* s;
  spline_path_segment_count_t i;
  for (i = 0; (i < path.segments_count); i += 1) {
    s = (i + path.segments);
    if (s->free) {
      (s->free)((s->data));
    };
  };
}
