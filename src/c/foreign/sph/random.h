
#ifndef sph_random_h
#define sph_random_h

#include <inttypes.h>
typedef struct {
  uint64_t data[4];
} sph_random_state_t;
sph_random_state_t sph_random_state_new(uint64_t seed);
uint64_t sph_random_u64(sph_random_state_t* state);
uint64_t sph_random_u64_bounded(sph_random_state_t* state, uint64_t range);
double sph_random_f64_bounded(sph_random_state_t* state, double range);
double sph_random_f64(sph_random_state_t* state);
void sph_random_u64_array(sph_random_state_t* state, size_t size, uint64_t* out);
void sph_random_u64_bounded_array(sph_random_state_t* state, uint64_t range, size_t size, uint64_t* out);
void sph_random_f64_array(sph_random_state_t* state, size_t size, double* out);
void sph_random_f64_bounded_array(sph_random_state_t* state, double range, size_t size, double* out);
uint32_t sph_random_u32(sph_random_state_t* state);
uint32_t sph_random_u32_bounded(sph_random_state_t* state, uint32_t range);
void sph_random_u32_array(sph_random_state_t* state, size_t size, uint32_t* out);
void sph_random_u32_bounded_array(sph_random_state_t* state, uint32_t range, size_t size, uint32_t* out);
double sph_random_f64_1to1(sph_random_state_t* state);
void sph_random_f64_array_1to1(sph_random_state_t* state, size_t size, double* out);
double sph_random_f64_0to1(sph_random_state_t* state);
void sph_random_f64_array_0to1(sph_random_state_t* state, size_t size, double* out);
#endif
