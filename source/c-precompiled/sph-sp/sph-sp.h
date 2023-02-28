
#ifndef sph_sp_h
#define sph_sp_h

#include <byteswap.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sph-sp/float.h>
#include <sph-sp/random.h>

/* configuration. changes need recompilation of sph-sp shared library */

#ifndef sp_channel_count_t
#define sp_channel_count_t uint8_t
#endif
#ifndef sp_channel_limit
#define sp_channel_limit 2
#endif
#ifndef sp_cheap_filter_passes_limit
#define sp_cheap_filter_passes_limit 8
#endif
#ifndef spline_path_value_t
#define spline_path_value_t sp_sample_t
#endif
#ifndef sp_random_seed
#define sp_random_seed 1557083953
#endif
#ifndef sp_sample_nearly_equal
#define sp_sample_nearly_equal sph_f64_nearly_equal
#endif
#ifndef sp_sample_random_primitive
#define sp_sample_random_primitive sph_random_f64_1to1
#endif
#ifndef sp_samples_nearly_equal
#define sp_samples_nearly_equal sph_f64_array_nearly_equal
#endif
#ifndef sp_samples_random_bounded_primitive
#define sp_samples_random_bounded_primitive sph_random_f64_bounded_array
#endif
#ifndef sp_samples_random_primitive
#define sp_samples_random_primitive sph_random_f64_array_1to1
#endif
#ifndef sp_samples_sum
#define sp_samples_sum sph_f64_sum
#endif
#ifndef sp_sample_t
#define sp_sample_t double
#endif
#ifndef sp_size_t
#define sp_size_t sp_time_t
#endif
#ifndef sp_ssize_t
#define sp_ssize_t int32_t
#endif
#ifndef sp_time_half_t
#define sp_time_half_t uint16_t
#endif
#ifndef sp_time_random_bounded_primitive
#define sp_time_random_bounded_primitive sph_random_u32_bounded
#endif
#ifndef sp_time_random_primitive
#define sp_time_random_primitive sph_random_u32
#endif
#ifndef sp_times_random_bounded_primitive
#define sp_times_random_bounded_primitive sph_random_u32_bounded_array
#endif
#ifndef sp_times_random_primitive
#define sp_times_random_primitive sph_random_u32_array
#endif
#ifndef sp_time_t
#define sp_time_t uint32_t
#endif
#ifndef sp_unit_random_primitive
#define sp_unit_random_primitive sph_random_f64_0to1
#endif
#ifndef sp_units_random_primitive
#define sp_units_random_primitive sph_random_f64_array_0to1
#endif
#ifndef sp_unit_t
#define sp_unit_t double
#endif
#include <string.h>
#include <stdio.h>
#include <sph-sp/status.h>
#include <sph-sp/array3.c>
#include <sph-sp/hashtable.c>
#include <sph-sp/helper.h>
#include <sph-sp/memreg.c>
#include <sph-sp/set.c>
#include <sph-sp/spline-path.h>
#include <sph-sp/main.h>
#include <sph-sp/arrays.h>
#include <sph-sp/path.h>
#include <sph-sp/filter.h>
#include <sph-sp/sequencer.h>
#include <sph-sp/statistics.h>
#include <sph-sp/plot.h>
#include <sph-sp/main2.h>
#endif