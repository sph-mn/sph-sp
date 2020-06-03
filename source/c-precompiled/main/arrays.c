/* there are currently two types of arrays:
   * plain memory pointers, called sample-array and time-array
   * structs {.data, .size. .used}, called sp-samples-t and sp-times-t
     * "size" is to not have to pass size as an extra function argument
     * "used" is an index for variable length content to avoid realloc if the size changes with modifications */
status_t sp_sample_array_new(sp_time_t size, sp_sample_t** out) { return ((sph_helper_calloc((size * sizeof(sp_sample_t)), out))); }
status_t sp_time_array_new(sp_time_t size, sp_time_t** out) { return ((sph_helper_calloc((size * sizeof(sp_time_t)), out))); }
void sp_sample_array_to_time_array(sp_sample_t* in, sp_time_t in_size, sp_time_t* out) {
  sp_time_t i;
  for (i = 0; (i < in_size); i = (1 + i)) {
    out[i] = sp_cheap_round_positive((in[i]));
  };
}
/** get the maximum value in samples array, disregarding sign */
sp_sample_t sp_sample_array_absolute_max(sp_sample_t* in, sp_time_t in_size) {
  sp_sample_t result;
  sp_sample_t a;
  sp_time_t i;
  for (i = 0, result = 0; (i < in_size); i = (1 + i)) {
    a = fabs((in[i]));
    if (a > result) {
      result = a;
    };
  };
  return (result);
}
void sp_samples_to_times(sp_samples_t a, sp_times_t b) {
  size_t i;
  for (i = 0; (i < a.size); i = (1 + i)) {
    array3_get(b, i) = sp_cheap_round_positive((array3_get(a, i)));
  };
}
