
#define spline_path_value_t sp_sample_t
#define sp_path_point_count_t spline_path_segment_count_t
#define sp_path_point_count_limit 24
status_t sp_path_samples(sp_sample_t** out, sp_time_t length, sp_path_point_count_t point_count, sp_sample_t* x, sp_sample_t* y, sp_sample_t* c);
status_t sp_path_samples2(sp_sample_t** out, sp_time_t length, sp_sample_t y1, sp_sample_t y2);
status_t sp_path_samples3(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3);
status_t sp_path_samples4(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4);
status_t sp_path_samples5(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t y5);
status_t sp_path_samples_curve2(sp_sample_t** out, sp_time_t length, sp_sample_t y1, sp_sample_t y2, sp_sample_t c1);
status_t sp_path_samples_curve3(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t c1, sp_sample_t c2);
status_t sp_path_samples_curve4(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3);
status_t sp_path_samples_curve5(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t y5, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3, sp_sample_t c4);
status_t sp_path_times(sp_time_t** out, sp_time_t length, sp_path_point_count_t point_count, sp_sample_t* x, sp_sample_t* y, sp_sample_t* c);
status_t sp_path_times2(sp_time_t** out, sp_time_t length, sp_sample_t y1, sp_sample_t y2);
status_t sp_path_times3(sp_time_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3);
status_t sp_path_times4(sp_time_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4);
status_t sp_path_times5(sp_time_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t y5);
status_t sp_path_times_curve2(sp_time_t** out, sp_time_t length, sp_sample_t y1, sp_sample_t y2, sp_sample_t c1);
status_t sp_path_times_curve3(sp_time_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t c1, sp_sample_t c2);
status_t sp_path_times_curve4(sp_time_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3);
status_t sp_path_times_curve5(sp_time_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t y5, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3, sp_sample_t c4);
status_t sp_envelope_zero(sp_sample_t** out, sp_time_t length, sp_path_point_count_t point_count, sp_sample_t* x, sp_sample_t* y, sp_sample_t* c);
status_t sp_envelope_zero3(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t y1);
status_t sp_envelope_zero4(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2);
status_t sp_envelope_zero5(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3);
status_t sp_envelope_zero_curve3(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t y1, sp_sample_t c1, sp_sample_t c2);
status_t sp_envelope_zero_curve4(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3);
status_t sp_envelope_zero_curve5(sp_sample_t** out, sp_time_t length, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3, sp_sample_t c4);
status_t sp_envelope_scale(sp_time_t** out, sp_time_t length, sp_path_point_count_t point_count, sp_sample_t y_scalar, sp_sample_t* x, sp_sample_t* y, sp_sample_t* c);
status_t sp_envelope_scale3(sp_time_t** out, sp_time_t length, sp_sample_t y_scalar, sp_sample_t x1, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3);
status_t sp_envelope_scale4(sp_time_t** out, sp_time_t length, sp_sample_t y_scalar, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4);
status_t sp_envelope_scale5(sp_time_t** out, sp_time_t length, sp_sample_t y_scalar, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t y5);
status_t sp_envelope_scale_curve3(sp_time_t** out, sp_time_t length, sp_sample_t y_scalar, sp_sample_t x1, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t c1, sp_sample_t c2);
status_t sp_envelope_scale_curve4(sp_time_t** out, sp_time_t length, sp_sample_t y_scalar, sp_sample_t x1, sp_sample_t x2, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3);
status_t sp_envelope_scale_curve5(sp_time_t** out, sp_time_t length, sp_sample_t y_scalar, sp_sample_t x1, sp_sample_t x2, sp_sample_t x3, sp_sample_t y1, sp_sample_t y2, sp_sample_t y3, sp_sample_t y4, sp_sample_t y5, sp_sample_t c1, sp_sample_t c2, sp_sample_t c3, sp_sample_t c4);