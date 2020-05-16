int spline_path_new_get_2(sp_sample_t* out, sp_time_t duration, spline_path_segment_t s1, spline_path_segment_t s2) {
  spline_path_segment_t segments[2];
  segments[0] = s1;
  segments[1] = s2;
  return ((spline_path_new_get(2, segments, 0, duration, out)));
}
int spline_path_new_get_3(sp_sample_t* out, sp_time_t duration, spline_path_segment_t s1, spline_path_segment_t s2, spline_path_segment_t s3) {
  spline_path_segment_t segments[3];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  return ((spline_path_new_get(3, segments, 0, duration, out)));
}
int spline_path_new_get_4(sp_sample_t* out, sp_time_t duration, spline_path_segment_t s1, spline_path_segment_t s2, spline_path_segment_t s3, spline_path_segment_t s4) {
  spline_path_segment_t segments[4];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  segments[3] = s4;
  return ((spline_path_new_get(4, segments, 0, duration, out)));
}
status_t sp_path_samples(spline_path_segment_count_t segment_count, spline_path_segment_t* segments, spline_path_time_t size, sp_sample_t** out) {
  status_declare;
  sp_sample_t* a = 0;
  status_require((sp_samples_new(size, (&a))));
  if (spline_path_new_get(segment_count, segments, 0, size, a)) {
    free(a);
    sp_memory_error;
  };
  *out = a;
exit:
  return (status);
}
status_t sp_path_times(spline_path_segment_count_t segment_count, spline_path_segment_t* segments, spline_path_time_t size, sp_time_t** out) {
  status_declare;
  sp_time_t* b;
  sp_sample_t* a = 0;
  status_require((sp_path_samples(segment_count, segments, size, (&a))));
  status_require((sp_times_new(size, (&b))));
  sp_samples_to_time(a, size, b);
  *out = b;
exit:
  if (a) {
    free(a);
  };
  return (status);
}
status_t sp_path_samples_2(sp_sample_t** out, spline_path_time_t size, spline_path_segment_t s1, spline_path_segment_t s2) {
  spline_path_segment_t segments[2];
  segments[0] = s1;
  segments[1] = s2;
  return ((sp_path_samples(2, segments, size, out)));
}
status_t sp_path_samples_3(sp_sample_t** out, spline_path_time_t size, spline_path_segment_t s1, spline_path_segment_t s2, spline_path_segment_t s3) {
  spline_path_segment_t segments[3];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  return ((sp_path_samples(3, segments, size, out)));
}
status_t sp_path_samples_4(sp_sample_t** out, spline_path_time_t size, spline_path_segment_t s1, spline_path_segment_t s2, spline_path_segment_t s3, spline_path_segment_t s4) {
  spline_path_segment_t segments[4];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  segments[3] = s4;
  return ((sp_path_samples(4, segments, size, out)));
}
status_t sp_path_times_2(sp_time_t** out, spline_path_time_t size, spline_path_segment_t s1, spline_path_segment_t s2) {
  spline_path_segment_t segments[2];
  segments[0] = s1;
  segments[1] = s2;
  return ((sp_path_times(2, segments, size, out)));
}
status_t sp_path_times_3(sp_time_t** out, spline_path_time_t size, spline_path_segment_t s1, spline_path_segment_t s2, spline_path_segment_t s3) {
  spline_path_segment_t segments[3];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  return ((sp_path_times(3, segments, size, out)));
}
status_t sp_path_times_4(sp_time_t** out, spline_path_time_t size, spline_path_segment_t s1, spline_path_segment_t s2, spline_path_segment_t s3, spline_path_segment_t s4) {
  spline_path_segment_t segments[4];
  segments[0] = s1;
  segments[1] = s2;
  segments[2] = s3;
  segments[3] = s4;
  return ((sp_path_times(4, segments, size, out)));
}
