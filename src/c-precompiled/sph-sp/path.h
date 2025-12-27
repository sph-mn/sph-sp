
#define sp_path_curve_strength 12.0
typedef struct {
  sp_time_t* t;
  sp_sample_t* values[2];
  sp_time_t point_count;
} sp_path_t;
void sp_path_get(sp_path_t* path, sp_time_t start, sp_time_t end, sp_sample_t* out, sp_time_t* cursor);