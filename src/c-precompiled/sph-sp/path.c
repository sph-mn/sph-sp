
/** g(t,c) = (exp(k*c*t) - 1) / (exp(k*c) - 1) */
void sp_path_get(sp_path_t* path, sp_time_t start, sp_time_t end, sp_sample_t* out, sp_time_t* cursor) {
  sp_time_t point_index;
  sp_time_t out_index;
  sp_time_t* t;
  sp_sample_t* y;
  sp_sample_t* c;
  sp_time_t segment_start_time;
  sp_time_t segment_end_time;
  sp_time_t segment_length;
  sp_time_t segment_to_time;
  sp_time_t step_index;
  sp_sample_t start_value;
  sp_sample_t end_value;
  sp_sample_t step_value;
  sp_sample_t curve_value;
  sp_sample_t a_value;
  sp_sample_t denom_value;
  sp_sample_t ratio_value;
  sp_sample_t exp_value;
  t = path->t;
  y = (path->values)[0];
  c = (path->values)[1];
  point_index = *cursor;
  out_index = 0;
  if (c == 0) {
    while ((start < end)) {
      segment_start_time = t[point_index];
      segment_end_time = t[(point_index + 1)];
      segment_length = (segment_end_time - segment_start_time);
      segment_to_time = ((end < segment_end_time) ? end : segment_end_time);
      start_value = y[point_index];
      end_value = y[(point_index + 1)];
      step_value = ((end_value - start_value) / segment_length);
      step_index = (start - segment_start_time);
      while ((start < segment_to_time)) {
        out[out_index] = (start_value + (step_value * step_index));
        out_index += 1;
        start += 1;
        step_index += 1;
      };
      if (segment_to_time == segment_end_time) {
        point_index += 1;
      };
    };
    *cursor = point_index;
    return;
  };
  while ((start < end)) {
    segment_start_time = t[point_index];
    segment_end_time = t[(point_index + 1)];
    segment_length = (segment_end_time - segment_start_time);
    segment_to_time = ((end < segment_end_time) ? end : segment_end_time);
    start_value = y[point_index];
    end_value = y[(point_index + 1)];
    curve_value = c[point_index];
    if (curve_value == 0.0) {
      step_value = ((end_value - start_value) / segment_length);
      step_index = (start - segment_start_time);
      while ((start < segment_to_time)) {
        out[out_index] = (start_value + (step_value * step_index));
        out_index += 1;
        start += 1;
        step_index += 1;
      };
    } else {
      a_value = (sp_path_curve_strength * curve_value);
      denom_value = expm1(a_value);
      ratio_value = exp((a_value / segment_length));
      step_index = (start - segment_start_time);
      exp_value = exp((a_value * (step_index / segment_length)));
      while ((start < segment_to_time)) {
        out[out_index] = (start_value + ((end_value - start_value) * ((exp_value - 1.0) / denom_value)));
        out_index += 1;
        start += 1;
        exp_value *= ratio_value;
      };
    };
    if (segment_to_time == segment_end_time) {
      point_index += 1;
    };
  };
  *cursor = point_index;
}
