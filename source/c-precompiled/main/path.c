/* * sp-path creates discrete 2d paths interpolated between given points
  * maps fro one independent discrete value to one dependent continuous value
  * only the dependent value is returned
  * multidimensional interpolation can be archieved with multiple configs and calls
  * a copy of segments is made internally and only the copy is used
  * uses points as structs because pre-defined size arrays can not be used in structs
  * segments-len must be greater than zero
  * segments must be a valid sp-path segment configuration
  * interpolators are called with path-relative start/end inside segment and with out positioned at the segment output
  * all segment types have a fixed number of points. line: 1, bezier: 3, move: 1, constant: 0, path: 0 */
/** p-rest length 1 */
void sp_path_i_move(sp_sample_count_t start, sp_sample_count_t end, sp_path_point_t p_start, sp_path_point_t* p_rest, void* options, sp_path_value_t* out) {
  memset(out, 0, (end - start));
  out[(end - start - 1)] = (p_rest[0]).y;
};
/** p-rest length 0 */
void sp_path_i_constant(sp_sample_count_t start, sp_sample_count_t end, sp_path_point_t p_start, sp_path_point_t* p_rest, void* options, sp_path_value_t* out) {
  sp_sample_count_t i;
  for (i = 0; (i < (end - start)); i = (1 + i)) {
    out[i] = p_start.y;
  };
};
/** p-rest length 1 */
void sp_path_i_line(sp_sample_count_t start, sp_sample_count_t end, sp_path_point_t p_start, sp_path_point_t* p_rest, void* options, sp_path_value_t* out) {
  sp_sample_count_t i;
  sp_path_point_t p_end;
  sp_sample_count_t s_size;
  sp_sample_count_t s_start;
  sp_path_value_t t;
  p_end = p_rest[0];
  s_size = (p_end.x - p_start.x);
  s_start = (start - p_start.x);
  for (i = 0; (i < (end - start)); i = (1 + i)) {
    t = ((s_start + i) / s_size);
    out[i] = ((p_end.y * t) + (p_start.y * (1 - t)));
  };
};
/** p-rest length 3 */
void sp_path_i_bezier(sp_sample_count_t start, sp_sample_count_t end, sp_path_point_t p_start, sp_path_point_t* p_rest, void* options, sp_path_value_t* out) {
  sp_sample_count_t i;
  sp_path_value_t mt;
  sp_path_point_t p_end;
  sp_sample_count_t s_size;
  sp_sample_count_t s_start;
  sp_path_value_t t;
  p_end = p_rest[2];
  s_size = (p_end.x - p_start.x);
  s_start = (start - p_start.x);
  for (i = 0; (i < (end - start)); i = (1 + i)) {
    t = ((s_start + i) / s_size);
    mt = (1 - t);
    out[i] = ((p_start.y * mt * mt * mt) + ((p_rest[0]).y * 3 * mt * mt * t) + ((p_rest[1]).y * 3 * mt * t * t) + (p_end.x * t * t * t));
  };
};
/** p-rest length 0. options is one sp-path-t */
void sp_path_i_path(sp_sample_count_t start, sp_sample_count_t end, sp_path_point_t p_start, sp_path_point_t* p_rest, void* options, sp_path_value_t* out) { sp_path_get((*((sp_path_t*)(options))), (start - p_start.x), (end - p_start.x), out); };
sp_path_point_t sp_path_start(sp_path_t path) {
  sp_path_point_t p;
  sp_path_segment_t s;
  s = (path.segments)[0];
  if (sp_path_i_move == s.interpolator) {
    p = (s.points)[0];
  } else {
    p.x = 0;
    p.y = 0;
  };
  return (p);
};
sp_path_point_t sp_path_end(sp_path_t path) {
  sp_path_segment_t s;
  s = (path.segments)[(path.segments_len - 1)];
  if (sp_path_i_constant == s.interpolator) {
    s = (path.segments)[(path.segments_len - 2)];
  };
  return (((s.points)[(s._points_len - 1)]));
};
status_t sp_path_new(sp_path_segment_count_t segments_len, sp_path_segment_t* segments, sp_path_t* out_path) {
  status_declare;
  sp_path_segment_count_t i;
  sp_path_t path;
  sp_path_segment_t s;
  sp_path_point_t start;
  s = segments[0];
  if (sp_path_i_move == s.interpolator) {
    start = (s.points)[0];
  } else {
    start.x = 0;
    start.y = 0;
  };
  status_require((sph_helper_malloc((segments_len * sizeof(sp_path_segment_t)), (&(path.segments)))));
  memcpy((path.segments), segments, (segments_len * sizeof(sp_path_segment_t)));
  for (i = 0; (i < segments_len); i = (1 + i)) {
    ((path.segments)[i])._start = start;
    s = (path.segments)[i];
    s._points_len = sp_path_interpolator_points_len((s.interpolator));
    start = ((sp_path_i_path == s.interpolator) ? sp_path_end((*((sp_path_t*)(s.options)))) : (s.points)[(s._points_len - 1)]);
  };
  path.segments_len = segments_len;
  *out_path = path;
exit:
  return (status);
};
void sp_path_get(sp_path_t path, sp_sample_count_t start, sp_sample_count_t end, sp_path_value_t* out) {
  sp_path_segment_count_t i;
  sp_path_segment_t s;
  sp_sample_count_t s_start;
  sp_sample_count_t s_end;
  sp_path_segment_count_t t;
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
    (s.interpolator)(t, (min(s_end, end)), (s._start), (s.points), (s.options), ((t - start) + out));
    t = s_end;
  };
  if (end > t) {
    memset(((t - start) + out), 0, (end - t));
  };
};
void sp_path_free(sp_path_t a) { free((a.segments)); };