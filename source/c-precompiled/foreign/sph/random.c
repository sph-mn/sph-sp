/* depends on random-h.c */
#define rotl(x, k) ((x << k) | (x >> (64 - k)))
/** use the given u64 as a seed and set state with splitmix64 results.
   the same seed will lead to the same series of random numbers from sp-random */
sph_random_state_t sph_random_state_new(u64 seed) {
  u8 i;
  u64 z;
  sph_random_state_t result;
  for (i = 0; (i < 4); i = (1 + i)) {
    seed = (seed + UINT64_C(11400714819323198485));
    z = seed;
    z = ((z ^ (z >> 30)) * UINT64_C(13787848793156543929));
    z = ((z ^ (z >> 27)) * UINT64_C(10723151780598845931));
    (result.data)[i] = (z ^ (z >> 31));
  };
  return (result);
};
define_sph_random(sph_random, u32, f64, (f64_from_u64(result_plus)));