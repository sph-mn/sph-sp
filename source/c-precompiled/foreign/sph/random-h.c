/* depends on types.c. have to use sph- prefix because standard lib uses random */
/** guarantees that all dyadic rationals of the form (k / 2**−53) will be equally likely. this conversion prefers the high bits of x.
     from http://xoshiro.di.unimi.it/ */
#define f64_from_u64(a) ((a >> 11) * (1.0 / (UINT64_C(1) << 53)))
#define define_sph_random(name, size_type, data_type, transfer) \
  /** return uniformly distributed random real numbers in the range -1 to 1. \
       implements xoshiro256plus from http://xoshiro.di.unimi.it/ \
       referenced by https://nullprogram.com/blog/2017/09/21/ */ \
  void name(sph_random_state_t* state, size_type size, data_type* out) { \
    u64 result_plus; \
    size_type i; \
    u64 t; \
    sph_random_state_t s; \
    s = *state; \
    for (i = 0; (i < size); i = (1 + i)) { \
      result_plus = ((s.data)[0] + (s.data)[3]); \
      t = ((s.data)[1] << 17); \
      (s.data)[2] = ((s.data)[2] ^ (s.data)[0]); \
      (s.data)[3] = ((s.data)[3] ^ (s.data)[1]); \
      (s.data)[1] = ((s.data)[1] ^ (s.data)[2]); \
      (s.data)[0] = ((s.data)[0] ^ (s.data)[3]); \
      (s.data)[2] = ((s.data)[2] ^ t); \
      (s.data)[3] = rotl(((s.data)[3]), 45); \
      out[i] = transfer; \
    }; \
    *state = s; \
  }
typedef struct {
  u64 data[4];
} sph_random_state_t;
sph_random_state_t sph_random_state_new(u64 seed);
void sph_random(sph_random_state_t* state, u32 size, f64* out);