
#define sp_path_t spline_path_t
#define sp_path_value_t spline_path_value_t
#define sp_path_point_t spline_path_point_t
#define sp_path_segment_t spline_path_segment_t
#define sp_path_segment_count_t spline_path_segment_count_t
#define sp_path_line spline_path_line
#define sp_path_move spline_path_move
#define sp_path_bezier spline_path_bezier
#define sp_path_bezier_arc spline_path_bezier_arc
#define sp_path_constant spline_path_constant
#define sp_path_path spline_path_path
#define sp_path_prepare_segments spline_path_prepare_segments
#define sp_path_i_line spline_path_i_line
#define sp_path_i_move spline_path_i_move
#define sp_path_i_bezier spline_path_i_bezier
#define sp_path_i_bezier_arc spline_path_i_bezier_arc
#define sp_path_i_constant spline_path_i_constant
#define sp_path_i_path spline_path_i_path
#define sp_path_end spline_path_end
#define sp_path_size spline_path_size
#define sp_path_free spline_path_free
#define sp_path_get spline_path_get
#define sp_path_set spline_path_set
#define sp_path_times_constant(out, size, value) sp_path_times_2(out, size, (sp_path_move(0, value)), (sp_path_constant()))
#define sp_path_samples_constant(out, size, value) sp_path_samples_2(out, size, (sp_path_move(0, value)), (sp_path_constant()))
#define sp_path_curves_config_declare(name, _segment_count) \
  sp_path_curves_config_t name; \
  sp_path_value_t name##_x[_segment_count]; \
  sp_path_value_t name##_y[_segment_count]; \
  sp_path_value_t name##_c[_segment_count]; \
  name.segment_count = _segment_count; \
  name.x = name##_x; \
  name.y = name##_y; \
  name.c = name##_c
typedef struct {
  sp_time_t segment_count;
  sp_path_value_t* x;
  sp_path_value_t* y;
  sp_path_value_t* c;
} sp_path_curves_config_t;
status_t sp_path_samples_new(sp_path_t path, sp_time_t size, sp_sample_t** out);
status_t sp_path_samples_1(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1);
status_t sp_path_samples_2(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2);
status_t sp_path_samples_3(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3);
status_t sp_path_samples_4(sp_sample_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4);
status_t sp_path_times_new(sp_path_t path, sp_time_t size, sp_time_t** out);
status_t sp_path_times_1(sp_time_t** out, sp_time_t size, sp_path_segment_t s1);
status_t sp_path_times_2(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2);
status_t sp_path_times_3(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3);
status_t sp_path_times_4(sp_time_t** out, sp_time_t size, sp_path_segment_t s1, sp_path_segment_t s2, sp_path_segment_t s3, sp_path_segment_t s4);
status_t sp_path_derivation(sp_path_t path, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_time_t index, sp_path_t* out);
status_t sp_path_samples_derivation(sp_path_t path, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_time_t index, sp_sample_t** out, sp_time_t* out_size);
status_t sp_path_times_derivation(sp_path_t path, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_time_t index, sp_time_t** out, sp_time_t* out_size);
void sp_path_multiply(sp_path_t path, sp_sample_t x_factor, sp_sample_t y_factor);
status_t sp_path_derivations_normalized(sp_path_t base, sp_time_t count, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_path_t** out);
status_t sp_path_samples_derivations_normalized(sp_path_t path, sp_time_t count, sp_sample_t** x_changes, sp_sample_t** y_changes, sp_sample_t*** out, sp_time_t** out_sizes);
status_t sp_path_curves_config_new(sp_time_t segment_count, sp_path_curves_config_t* out);
void sp_path_curves_config_free(sp_path_curves_config_t a);
status_t sp_path_curves_new(sp_path_curves_config_t config, sp_path_t* out);
status_t sp_path_curves_times_new(sp_path_curves_config_t config, sp_time_t length, sp_time_t** out);
status_t sp_path_curves_samples_new(sp_path_curves_config_t config, sp_time_t length, sp_sample_t** out);