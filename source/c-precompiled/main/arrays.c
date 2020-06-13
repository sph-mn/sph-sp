/* there are currently two types of arrays:
   * plain memory pointers, called sample-array and time-array
   * structs {.data, .size. .used}, called sp-samples-t and sp-times-t
     * "size" is to not have to pass size as an extra function argument
     * "used" is an index for variable length content to avoid realloc if the size changes with modifications */
sph_random_define_x256p(sp_sample_array_random, sp_sample_t, ((2 * sph_random_f64_from_u64(a)) - 1.0));
sph_random_define_x256ss(sp_time_array_random, sp_time_t, a);
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
/** display a sample array in one line */
void sp_sample_array_display(sp_sample_t* a, sp_time_t size) {
  sp_time_t i;
  printf(("%.5f"), (a[0]));
  for (i = 1; (i < size); i = (1 + i)) {
    printf((" %.5f"), (a[i]));
  };
  printf("\n");
}
/** adjust amplitude of out to match the one of in */
void sp_sample_array_set_unity_gain(sp_sample_t* in, sp_time_t in_size, sp_sample_t* out) {
  sp_time_t i;
  sp_sample_t in_max;
  sp_sample_t out_max;
  sp_sample_t difference;
  sp_sample_t correction;
  in_max = sp_sample_array_absolute_max(in, in_size);
  out_max = sp_sample_array_absolute_max(out, in_size);
  if ((0 == in_max) || (0 == out_max)) {
    return;
  };
  difference = (out_max / in_max);
  correction = (1 + ((1 - difference) / difference));
  for (i = 0; (i < in_size); i = (1 + i)) {
    out[i] = (correction * out[i]);
  };
}
void sp_samples_to_times(sp_samples_t a, sp_times_t b) {
  size_t i;
  for (i = 0; (i < a.size); i = (1 + i)) {
    array3_get(b, i) = sp_cheap_round_positive((array3_get(a, i)));
  };
}
