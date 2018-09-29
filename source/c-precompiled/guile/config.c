#if sp_sample_format_f64
#define scm_from_sp_sample(a) scm_from_double(a)
#define scm_to_sp_sample(a) scm_to_double(a)
#define scm_c_make_sp_samples(len) scm_make_f64vector((scm_from_sp_sample_count(len)), (scm_from_uint8(0)))
#define scm_c_take_samples(a, len) scm_take_f64vector(a, len)
#elif sp_sample_format_f32
#define scm_from_sp_sample(a) scm_from_double(((f32)(a)))
#define scm_to_sp_sample(a) ((f32)(scm_to_double(a)))
#define scm_c_make_sp_samples(len) scm_make_f32vector((scm_from_sp_sample_count(len)), (scm_from_uint8(0)))
#define scm_c_take_samples(a, len) scm_take_f32vector(a, len)
#elif sp_sample_format_int32
#define scm_from_sp_sample(a) scm_from_int32(a)
#define scm_to_sp_sample(a) scm_to_int32(a)
#define scm_c_make_sp_samples(len) scm_make_s32vector((scm_from_sp_sample_count(len)), (scm_from_uint8(0)))
#define scm_c_take_samples(a, len) scm_take_s32vector(a, len)
#elif sp_sample_format_int16
#define scm_from_sp_sample(a) scm_from_int16(a)
#define scm_to_sp_sample(a) scm_to_int16(a)
#define scm_c_make_sp_samples(len) scm_make_s16vector((scm_from_sp_sample_count(len)), (scm_from_uint8(0)))
#define scm_c_take_samples(a, len) scm_take_s16vector(a, len)
#elif sp_sample_format_int8
#define scm_from_sp_sample(a) scm_from_int8(a)
#define scm_to_sp_sample(a) scm_to_int8(a)
#define scm_c_make_sp_samples(len) scm_make_s8vector((scm_from_sp_sample_count(len)), (scm_from_uint8(0)))
#define scm_c_take_samples(a, len) scm_take_s8vector(a, len)
#endif
#define scm_from_sp_channel_count(a) scm_from_uint32(a)
#define scm_from_sp_sample_rate(a) scm_from_uint32(a)
#define scm_from_sp_sample_count(a) scm_from_size_t(a)
#define scm_from_sp_float(a) scm_from_double(a)
#define scm_to_sp_channel_count(a) scm_to_uint32(a)
#define scm_to_sp_sample_rate(a) scm_to_uint32(a)
#define scm_to_sp_sample_count(a) scm_to_size_t(a)
#define scm_to_sp_float(a) scm_to_double(a)
