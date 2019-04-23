/* create discrete 2d paths interpolated between given points.
  * maps between one independent discrete value to one dependent continuous value
  * multidimensional interpolation can be archieved with multiple configs and calls. time is the only dependent variable
  * a copy of segments is made internally and only the copy is used
  * uses points as structs because pre-defined size arrays can not be used in structs */
typedef uint16_t sp_path_count_t;
typedef double sp_path_value_t;
typedef struct {
  sp_path_value_t x;
  sp_path_value_t y;
} sp_path_point_t;
typedef void (*sp_path_interpolator_t)(sp_sample_count_t, sp_sample_count_t, sp_path_point_t, sp_path_point_t*, void*, sp_path_value_t*);
typedef struct {
  sp_path_point_t _start;
  sp_path_count_t points_len;
  sp_path_point_t* points;
  sp_path_interpolator_t interpolator;
  sp_path_count_t options_len;
  void* options;
} sp_path_segment_t;
typedef struct {
  sp_path_count_t segments_len;
  sp_path_segment_t* segments;
} sp_path_t;
void sp_path_i_line(sp_sample_count_t start, sp_sample_count_t end, sp_path_point_t p_start, sp_path_point_t* p_rest, void* options, sp_path_value_t* out) {
  sp_sample_count_t i;
  sp_path_point_t p_end;
  sp_sample_count_t size;
  sp_path_value_t factor;
  p_end = p_rest[0];
  size = (p_end.x - p_start.x);
  for (i = 0; (i < size); i = (1 + i)) {
    factor = (i / size);
    out[i] = ((p_end.y * factor) + (p_start.y * (1 - factor)));
  };
};
void sp_path_free(sp_path_t a) { free((a.segments)); };
status_t sp_path_new(sp_path_count_t segments_len, sp_path_segment_t* segments, sp_path_t* out_path) {
  status_declare;
  sp_path_segment_t s;
  sp_path_count_t i;
  sp_path_point_t start;
  sp_path_t path;
  path.segments = 0;
  start.x = 0;
  start.y = 0;
  status_require((sph_helper_malloc((segments_len * sizeof(sp_path_segment_t)), (&(path.segments)))));
  memcpy((path.segments), segments, (segments_len * sizeof(sp_path_segment_t)));
  for (i = 0; (i < segments_len); i = (1 + i)) {
    s = (path.segments)[i];
    s._start = start;
    start = (s.points)[(s.points_len - 1)];
  };
  *out_path = path;
exit:
  if (status_is_failure && path.segments) {
    free((path.segments));
  };
  return (status);
};
void sp_path_get(sp_path_t path, sp_sample_count_t start, sp_sample_count_t end, sp_path_value_t* out) {
  sp_path_count_t i;
  sp_path_segment_t s;
  sp_sample_count_t s_start;
  sp_sample_count_t s_end;
  for (i = 0; (i < path.segments_len); i = (1 + i)) {
    s = (path.segments)[i];
    s_start = s._start.x;
    if (s_start > start) {
      break;
    };
    s_end = ((s.points)[(s.points_len - 1)]).x;
    (s.interpolator)(start, (min(s_end, end)), (s._start), (s.points), (s.options), (start + out));
    s_start = s_end;
  };
  /* last segment ended before requested end */
  if (end > s_start) {
    memset((s_start + out), 0, (end - s_start));
  };
};
/** create a path array immediately from segments without creating a path object */
void sp_path_new_get(sp_path_count_t segments_len, sp_path_segment_t* segments, sp_sample_count_t start, sp_sample_count_t end, sp_path_value_t* out) {
  sp_path_segment_t s;
  sp_path_count_t i;
  sp_path_point_t p_start;
  sp_sample_count_t s_start;
  sp_sample_count_t s_end;
  p_start.x = 0;
  p_start.y = 0;
  for (i = 0; (i < segments_len); i = (1 + i)) {
    s = segments[i];
    s_start = p_start.x;
    if (s_start > start) {
      break;
    };
    s_end = ((s.points)[(s.points_len - 1)]).x;
    (s.interpolator)(start, (min(s_end, end)), (s._start), (s.points), (s.options), (start + out));
    p_start = (s.points)[(s.points_len - 1)];
  };
  if (end > s_start) {
    memset((s_start + out), 0, (end - s_start));
  };
};