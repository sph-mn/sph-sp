/* depends on spline-path-h.c */
/** p-rest length 1 */
void spline_path_i_move(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) { memset(out, 0, (sizeof(spline_path_value_t) * (end - start))); }
/** p-rest length 0 */
void spline_path_i_constant(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) {
  spline_path_time_t i;
  for (i = start; (i < end); i = (1 + i)) {
    out[(i - start)] = p_start.y;
  };
}
/** p-rest length 1 */
void spline_path_i_line(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) {
  spline_path_time_t i;
  spline_path_point_t p_end;
  spline_path_value_t s_size;
  spline_path_value_t t;
  p_end = p_rest[0];
  s_size = (p_end.x - p_start.x);
  for (i = start; (i < end); i = (1 + i)) {
    t = ((i - p_start.x) / s_size);
    out[(i - start)] = ((p_end.y * t) + (p_start.y * (1 - t)));
  };
}
/** p-rest length 3 */
void spline_path_i_bezier(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) {
  spline_path_time_t i;
  spline_path_value_t mt;
  spline_path_point_t p_end;
  spline_path_value_t s_size;
  spline_path_value_t t;
  p_end = p_rest[2];
  s_size = (p_end.x - p_start.x);
  for (i = start; (i < end); i = (1 + i)) {
    t = ((i - p_start.x) / s_size);
    mt = (1 - t);
    out[(i - start)] = ((p_start.y * mt * mt * mt) + ((p_rest[0]).y * 3 * mt * mt * t) + ((p_rest[1]).y * 3 * mt * t * t) + (p_end.y * t * t * t));
  };
}
/** get values on path between start (inclusive) and end (exclusive).
   since x values are integers, a path from (0 0) to (10 20) for example will have reached 20 only at the 11th point.
   out memory is managed by the caller. the size required for out is end minus start */
void spline_path_get(spline_path_t path, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out) {
  /* find all segments that overlap with requested range */
  spline_path_segment_count_t i;
  spline_path_segment_t s;
  spline_path_time_t s_start;
  spline_path_time_t s_end;
  spline_path_time_t out_start;
  for (i = 0; (i < path.segments_count); i = (1 + i)) {
    s = (path.segments)[i];
    s_start = s._start.x;
    s_end = ((s.points)[(s._points_count - 1)]).x;
    if (s_start > end) {
      break;
    };
    if (s_end < start) {
      continue;
    };
    out_start = ((s_start > start) ? (s_start - start) : 0);
    s_start = ((s_start > start) ? s_start : start);
    s_end = ((s_end < end) ? s_end : end);
    (s.interpolator)(s_start, s_end, (s._start), (s.points), (s.options), (out_start + out));
  };
  /* outside points zero. set the last segment point which would be set by a following segment.
can only be true for the last segment */
  if (end > s_end) {
    out[s_end] = ((s.points)[(s._points_count - 1)]).y;
    s_end = (1 + s_end);
    if (end > s_end) {
      memset((s_end + out), 0, ((end - s_end) * sizeof(spline_path_value_t)));
    };
  };
}
/** p-rest length 0. options is one spline-path-t */
void spline_path_i_path(spline_path_time_t start, spline_path_time_t end, spline_path_point_t p_start, spline_path_point_t* p_rest, void* options, spline_path_value_t* out) { spline_path_get((*((spline_path_t*)(options))), (start - p_start.x), (end - p_start.x), out); }
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
spline_path_point_t spline_path_end(spline_path_t path) {
  spline_path_segment_t s;
  s = (path.segments)[(path.segments_count - 1)];
  if (spline_path_i_constant == s.interpolator) {
    s = (path.segments)[(path.segments_count - 2)];
  };
  return (((s.points)[(s._points_count - 1)]));
}
/** segments are copied into out-path */
uint8_t spline_path_new(spline_path_segment_count_t segments_count, spline_path_segment_t* segments, spline_path_t* out_path) {
  spline_path_segment_count_t i;
  spline_path_t path;
  spline_path_segment_t s;
  spline_path_point_t start;
  start.x = 0;
  start.y = 0;
  path.segments = malloc((segments_count * sizeof(spline_path_segment_t)));
  if (!path.segments) {
    return (1);
  };
  memcpy((path.segments), segments, (segments_count * sizeof(spline_path_segment_t)));
  for (i = 0; (i < segments_count); i = (1 + i)) {
    ((path.segments)[i])._start = start;
    s = (path.segments)[i];
    s._points_count = ((spline_path_i_bezier == s.interpolator) ? 3 : 1);
    if ((spline_path_i_path == s.interpolator)) {
      start = spline_path_end((*((spline_path_t*)(s.options))));
      *(s.points) = start;
    } else if ((spline_path_i_constant == s.interpolator)) {
      *(s.points) = start;
      (s.points)->x = spline_path_time_max;
    } else {
      start = (s.points)[(s._points_count - 1)];
    };
    (path.segments)[i] = s;
  };
  path.segments_count = segments_count;
  *out_path = path;
  return (0);
}
void spline_path_free(spline_path_t a) { free((a.segments)); }
/** create a path array immediately from segments without creating a path object */
uint8_t spline_path_new_get(spline_path_segment_count_t segments_count, spline_path_segment_t* segments, spline_path_time_t start, spline_path_time_t end, spline_path_value_t* out) {
  spline_path_t path;
  if (spline_path_new(segments_count, segments, (&path))) {
    return (1);
  };
  spline_path_get(path, start, end, out);
  spline_path_free(path);
  return (0);
}
/** returns a move segment for the specified point */
spline_path_segment_t spline_path_move(spline_path_time_t x, spline_path_value_t y) {
  spline_path_segment_t s;
  s.interpolator = spline_path_i_move;
  (s.points)->x = x;
  (s.points)->y = y;
  return (s);
}
spline_path_segment_t spline_path_line(spline_path_time_t x, spline_path_value_t y) {
  spline_path_segment_t s;
  s.interpolator = spline_path_i_line;
  (s.points)->x = x;
  (s.points)->y = y;
  return (s);
}
spline_path_segment_t spline_path_bezier(spline_path_time_t x1, spline_path_value_t y1, spline_path_time_t x2, spline_path_value_t y2, spline_path_time_t x3, spline_path_value_t y3) {
  spline_path_segment_t s;
  s.interpolator = spline_path_i_bezier;
  (s.points)->x = x1;
  (s.points)->y = y1;
  (1 + s.points)->x = x2;
  (1 + s.points)->y = y2;
  (2 + s.points)->x = x3;
  (2 + s.points)->y = y3;
  return (s);
}
spline_path_segment_t spline_path_constant() {
  spline_path_segment_t s;
  s.interpolator = spline_path_i_constant;
  return (s);
}
/** return a segment that is another spline-path. length is the full length of the path.
   the path does not necessarily connect and is drawn as it would be on its own starting from the preceding segment */
spline_path_segment_t spline_path_path(spline_path_t* path) {
  spline_path_segment_t s;
  s.interpolator = spline_path_i_path;
  s.options = path;
  return (s);
}
