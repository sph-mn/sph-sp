
/** out memory is allocated */
status_t sp_path_samples_new(sp_path_t path, sp_time_t size, sp_sample_t** out) {
  status_declare;
  sp_sample_t* out_temp;
  if (0 == size) {
    size = sp_path_size(path);
  };
  status_require((sp_samples_new(size, (&out_temp))));
  spline_path_get((&path), 0, size, out_temp);
  *out = out_temp;
exit:
  status_return;
}

/** return a sp_time_t array from path.
   memory is allocated and ownership transferred to the caller */
status_t sp_path_times_new(sp_path_t path, sp_time_t size, sp_time_t** out) {
  status_declare;
  sp_time_t* out_temp;
  sp_sample_t* temp;
  temp = 0;
  if (0 == size) {
    size = sp_path_size(path);
  };
  status_require((sp_path_samples_new(path, size, (&temp))));
  status_require((sp_times_new(size, (&out_temp))));
  sp_samples_to_times(temp, size, out_temp);
  *out = out_temp;
exit:
  free(temp);
  status_return;
}

/** return a newly allocated sp_time_t array for a path with one segment */
status_t sp_path_times_1(sp_time_t** out, sp_time_t size, sp_path_segment_t s1) {
  sp_path_segment_t s[1] = { s1 };
  sp_path_t path;
  spline_path_set((&path), s, 1);
  return ((sp_path_times_new(path, size, out)));
}
status_t sp_path_times_2(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2) {
  sp_path_segment_t s[2] = { s1, s2 };
  sp_path_t path;
  spline_path_set((&path), s, 2);
  return ((sp_path_times_new(path, size, out)));
}
status_t sp_path_times_3(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3) {
  sp_path_segment_t s[3] = { s1, s2, s3 };
  sp_path_t path;
  spline_path_set((&path), s, 3);
  return ((sp_path_times_new(path, size, out)));
}
status_t sp_path_times_4(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4) {
  sp_path_segment_t s[4] = { s1, s2, s3, s4 };
  sp_path_t path;
  spline_path_set((&path), s, 4);
  return ((sp_path_times_new(path, size, out)));
}
status_t sp_path_samples_1(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1) {
  sp_path_segment_t s[1] = { s1 };
  sp_path_t path;
  spline_path_set((&path), s, 1);
  return ((sp_path_samples_new(path, size, out)));
}
status_t sp_path_samples_2(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2) {
  sp_path_segment_t s[2] = { s1, s2 };
  sp_path_t path;
  spline_path_set((&path), s, 2);
  return ((sp_path_samples_new(path, size, out)));
}
status_t sp_path_samples_3(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3) {
  sp_path_segment_t s[3] = { s1, s2, s3 };
  sp_path_t path;
  spline_path_set((&path), s, 3);
  return ((sp_path_samples_new(path, size, out)));
}
status_t sp_path_samples_4(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4) {
  sp_path_segment_t s[4] = { s1, s2, s3, s4 };
  sp_path_t path;
  spline_path_set((&path), s, 4);
  return ((sp_path_samples_new(path, size, out)));
}
status_t sp_path_curves_config_new(sp_time_t segment_count, sp_path_curves_config_t* out) {
  status_declare;
  srq((sp_samples_new(segment_count, (&(out->x)))));
  srq((sp_samples_new(segment_count, (&(out->y)))));
  srq((sp_samples_new(segment_count, (&(out->c)))));
  out->segment_count = segment_count;
exit:
  status_return;
}
void sp_path_curves_config_free(sp_path_curves_config_t a) {
  free((a.x));
  free((a.y));
  free((a.c));
}

/** a path that uses linear or circular interpolation depending on the values of the config.c.
   config.c 0 is linear and other values between -1.0 and 1.0 add curvature */
status_t sp_path_curves_new(sp_path_curves_config_t config, sp_path_t* out) {
  status_declare;
  sp_path_segment_t* ss;
  sp_time_t x;
  sp_sample_t y;
  sp_sample_t c;
  srq((sp_malloc_type((config.segment_count), sp_path_segment_t, (&ss))));
  ss[0] = sp_path_move(((config.x)[0]), ((config.y)[0]));
  sp_time_t ss_index = 1;
  for (sp_time_t i = 1; (i < config.segment_count); i += 1) {
    x = (config.x)[i];
    if (x == (config.x)[(i - 1)]) {
      continue;
    };
    y = (config.y)[i];
    c = (config.c)[i];
    ss[ss_index] = ((c < 1.0e-5) ? sp_path_line(x, y) : sp_path_bezier_arc(c, x, y));
    ss_index += 1;
  };
  spline_path_set(out, ss, ss_index);
exit:
  status_return;
}
status_t sp_path_curves_times_new(sp_path_curves_config_t config, sp_time_t length, sp_time_t** out) {
  status_declare;
  sp_path_t path;
  error_memory_init(1);
  srq((sp_path_curves_new(config, (&path))));
  error_memory_add((path.segments));
  srq((sp_path_times_new(path, length, out)));
exit:
  if (status_is_failure) {
    error_memory_free;
  };
  status_return;
}
status_t sp_path_curves_samples_new(sp_path_curves_config_t config, sp_time_t length, sp_sample_t** out) {
  status_declare;
  sp_path_t path;
  error_memory_init(1);
  srq((sp_path_curves_new(config, (&path))));
  error_memory_add((path.segments));
  srq((sp_path_samples_new(path, length, out)));
exit:
  if (status_is_failure) {
    error_memory_free;
  };
  status_return;
}
