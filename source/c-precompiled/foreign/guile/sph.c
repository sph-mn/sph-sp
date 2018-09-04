#include <inttypes.h>
#include <stdio.h>
#define boolean uint8_t
#define pointer_t uintptr_t
#define void void
#define uint8_t uint8_t
#define uint16_t uint16_t
#define uint32_t uint32_t
#define uint64_t uint64_t
#define int8_t int8_t
#define int16_t int16_t
#define int32_t int32_t
#define int64_t int64_t
#define f32_s float
#define f64_s double
/** writes values with current routine name and line info to standard output.
    example: (debug-log "%d" 1)
    otherwise like printf */
#define debug_log(format,...) fprintf(stdout,"%s:%d " format "\n",__func__,__LINE__,__VA_ARGS__)
#define null ((void)(0))
#define zero_p(a) (0==a)
