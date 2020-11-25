/** out memory is allocated */
status_t sp_path_samples(sp_path_t path, sp_path_time_t size, sp_sample_t** out) {
  status_declare;
  sp_sample_t* out_temp;
  if (0 == size) {
    size = sp_path_size(path);
  };
  status_require((sp_samples_new(size, (&out_temp))));
  spline_path_get(path, 0, size, out_temp);
  *out = out_temp;
exit:
  status_return;
}
/** return a sp_time_t array from path.
   memory is allocated and ownership transferred to the caller */
status_t sp_path_times(sp_path_t path, sp_path_time_t size, sp_time_t** out) {
  status_declare;
  sp_time_t* out_temp;
  sp_sample_t* temp;
  temp = 0;
  if (0 == size) {
    size = sp_path_size(path);
  };
  status_require((sp_path_samples(path, size, (&temp))));
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
  return ((sp_path_times(path, size, out)));
}
status_t sp_path_times_2(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2) {
  sp_path_segment_t s[2] = { s1, s2 };
  sp_path_t path;
  spline_path_set((&path), s, 2);
  return ((sp_path_times(path, size, out)));
}
status_t sp_path_times_3(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3) {
  sp_path_segment_t s[3] = { s1, s2, s3 };
  sp_path_t path;
  spline_path_set((&path), s, 3);
  return ((sp_path_times(path, size, out)));
}
status_t sp_path_times_4(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4) {
  sp_path_segment_t s[4] = { s1, s2, s3, s4 };
  sp_path_t path;
  spline_path_set((&path), s, 4);
  return ((sp_path_times(path, size, out)));
}
status_t sp_path_samples_1(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1) {
  sp_path_segment_t s[1] = { s1 };
  sp_path_t path;
  spline_path_set((&path), s, 1);
  return ((sp_path_samples(path, size, out)));
}
status_t sp_path_samples_2(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2) {
  sp_path_segment_t s[2] = { s1, s2 };
  sp_path_t path;
  spline_path_set((&path), s, 2);
  return ((sp_path_samples(path, size, out)));
}
status_t sp_path_samples_3(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3) {
  sp_path_segment_t s[3] = { s1, s2, s3 };
  sp_path_t path;
  spline_path_set((&path), s, 3);
  return ((sp_path_samples(path, size, out)));
}
status_t sp_path_samples_4(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4) {
  sp_path_segment_t s[4] = { s1, s2, s3, s4 };
  sp_path_t path;
  spline_path_set((&path), s, 4);
  return ((sp_path_samples(path, size, out)));
}
/** changes contains per point an array of values which will be multiplied with x or y values for index.
   each is an array with the layout ((number:derivation_change ...):point_change ...).
   index is the current derivation_change index.
   caveats:
   * changes for segments of type constant or type path are not to be included
   * path size can change, sp_path_size(path) can give the new size
   * invalid paths are possible if x_changes exceed range between the new previous and next point */
status_t sp_path_derivation(sp_path_t path, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_time_t index, sp_path_t* out) {
  status_declare;
  sp_path_t a;
  sp_path_time_t p_i;
  sp_path_point_t* p;
  sp_path_segment_count_t s_i;
  sp_path_time_t sp_i;
  sp_path_segment_t* s;
  sp_path_segment_t* ss;
  /* copy segments */
  ss = 0;
  status_require((sph_helper_malloc((path.segments_count * sizeof(sp_path_segment_t)), (&ss))));
  memcpy(ss, (path.segments), (path.segments_count * sizeof(sp_path_segment_t)));
  /* modify points */
  for (s_i = 0, p_i = 0; (s_i < path.segments_count); s_i += 1) {
    s = (ss + s_i);
    for (sp_i = 0; (sp_i < spline_path_segment_points_count((*s))); sp_i += 1) {
      if (spline_path_i_constant == s->interpolator) {
        break;
      } else {
        if (spline_path_i_path == s->interpolator) {
          continue;
        };
      };
      p = (sp_i + s->points);
      p->x *= x_changes[p_i][index];
      p->y *= y_changes[p_i][index];
      p_i += 1;
    };
  };
  sp_path_prepare_segments(ss, (path.segments_count));
  out->segments = ss;
  out->segments_count = path.segments_count;
exit:
  status_return;
}
/** get one derivation as a sp_sample_t array. out memory will be allocated */
status_t sp_path_samples_derivation(sp_path_t path, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_time_t index, sp_sample_t** out, sp_path_time_t* out_size) {
  status_declare;
  sp_sample_t* out_temp;
  sp_path_time_t size;
  status_require((sp_path_derivation(path, x_changes, y_changes, index, (&path))));
  size = sp_path_size(path);
  status_require((sp_samples_new(size, (&out_temp))));
  spline_path_get(path, 0, size, out_temp);
  *out = out_temp;
  *out_size = size;
exit:
  status_return;
}
/** get one derivation as a sp_time_t array. out memory will be allocated */
status_t sp_path_times_derivation(sp_path_t path, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_time_t index, sp_time_t** out, sp_path_time_t* out_size) {
  status_declare;
  sp_time_t* result;
  sp_sample_t* temp;
  sp_time_t temp_size;
  temp = 0;
  status_require((sp_path_samples_derivation(path, x_changes, y_changes, index, (&temp), (&temp_size))));
  status_require((sp_times_new(temp_size, (&result))));
  sp_samples_to_times(temp, temp_size, result);
  *out = result;
  *out_size = temp_size;
exit:
  free(temp);
  status_return;
}
/** multiply all x and y values of path segments by x_factor and y_factor respectively */
void sp_path_multiply(sp_path_t path, sp_sample_t x_factor, sp_sample_t y_factor) {
  sp_path_segment_t* s;
  sp_path_point_t* p;
  sp_path_segment_count_t s_i;
  sp_path_segment_count_t sp_i;
  for (s_i = 0; (s_i < path.segments_count); s_i += 1) {
    s = (path.segments + s_i);
    for (sp_i = 0; (sp_i < spline_path_segment_points_count((*s))); sp_i += 1) {
      if (spline_path_i_constant == s->interpolator) {
        break;
      } else {
        if (spline_path_i_path == s->interpolator) {
          continue;
        };
      };
      p = (sp_i + s->points);
      p->x *= x_factor;
      p->y *= y_factor;
    };
  };
  sp_path_prepare_segments((path.segments), (path.segments_count));
}
/** get count derived paths and adjust y values so that their sum follows base.y.
   algorithm
     for each (segment, point, path): get sum
     for each (segment, point, path): scale to sum */
status_t sp_path_derivations_normalized(sp_path_t base, sp_time_t count, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_path_t** out) {
  sp_path_t* paths;
  sp_sample_t y_sum;
  sp_time_t segment_i;
  sp_path_segment_t* bs;
  sp_path_point_t* bp;
  sp_time_t i;
  sp_path_segment_t* s;
  sp_path_point_t* p;
  sp_time_t point_i;
  sp_time_t path_i;
  sp_sample_t factor;
  status_declare;
  status_require((sph_helper_calloc((count * sizeof(sp_path_t)), (&paths))));
  for (path_i = 0; (path_i < count); path_i += 1) {
    status_require((sp_path_derivation(base, x_changes, y_changes, path_i, (paths + path_i))));
  };
  for (segment_i = 0; (segment_i < base.segments_count); segment_i += 1) {
    bs = (base.segments + segment_i);
    for (point_i = 0; (point_i < spline_path_segment_points_count((*bs))); point_i += 1) {
      if (spline_path_i_constant == bs->interpolator) {
        break;
      } else {
        if (spline_path_i_path == bs->interpolator) {
          continue;
        };
      };
      bp = (bs->points + point_i);
      y_sum = 0;
      for (path_i = 0; (path_i < count); path_i += 1) {
        s = ((paths[path_i]).segments + segment_i);
        p = (s->points + point_i);
        y_sum += p->y;
      };
      factor = ((0 == y_sum) ? 0 : (bp->y / y_sum));
      for (path_i = 0; (path_i < count); path_i += 1) {
        s = ((paths[path_i]).segments + segment_i);
        p = (s->points + point_i);
        p->y *= factor;
      };
    };
  };
  for (path_i = 0; (path_i < count); path_i += 1) {
    sp_path_prepare_segments(((paths[path_i]).segments), (base.segments_count));
  };
  *out = paths;
exit:
  if (status_is_failure) {
    if (paths) {
      free(paths);
    };
  };
  status_return;
}
/** get sp_path_derivations_normalized as sample arrays. out and out_sizes is allocated and passed to the caller */
status_t sp_path_samples_derivations_normalized(sp_path_t path, sp_time_t count, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_sample_t*** out, sp_time_t** out_sizes) {
  sp_time_t i;
  sp_time_t samples_i;
  sp_time_t size;
  sp_path_t* paths;
  sp_sample_t** samples;
  sp_time_t* sizes;
  status_declare;
  memreg_init(2);
  status_require((sph_helper_calloc((count * sizeof(sp_sample_t*)), (&samples))));
  memreg_add(samples);
  status_require((sph_helper_malloc((count * sizeof(sp_time_t)), (&sizes))));
  memreg_add(sizes);
  status_require((sp_path_derivations_normalized(path, count, x_changes, y_changes, (&paths))));
  for (i = 0; (i < count); i += 1) {
    size = sp_path_size((paths[i]));
    sizes[i] = size;
    status_require((sp_samples_new(size, (samples + i))));
    sp_path_get((paths[i]), 0, size, (samples[i]));
  };
  *out = samples;
  *out_sizes = sizes;
exit:
  if (status_is_failure) {
    for (i = 0; (i < count); i += 1) {
      free((samples[i]));
    };
    memreg_free;
  };
  if (paths) {
    free(paths);
  };
  status_return;
}
