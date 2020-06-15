/* there are currently two types of arrays:
   * plain memory pointers, called samples and times */
status_t sp_samples_new(sp_time_t size, sp_sample_t** out) { return ((sph_helper_calloc((size * sizeof(sp_sample_t)), out))); }
status_t sp_times_new(sp_time_t size, sp_time_t** out) { return ((sph_helper_calloc((size * sizeof(sp_time_t)), out))); }
void sp_samples_to_times(sp_sample_t* in, sp_time_t size, sp_time_t* out) {
  sp_time_t i;
  for (i = 0; (i < size); i = (1 + i)) {
    out[i] = sp_cheap_round_positive((in[i]));
  };
}
/** get the maximum value in samples array, disregarding sign */
sp_sample_t sp_samples_absolute_max(sp_sample_t* in, sp_time_t in_size) {
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
void sp_samples_display(sp_sample_t* a, sp_time_t size) {
  sp_time_t i;
  printf(("%.5f"), (a[0]));
  for (i = 1; (i < size); i = (1 + i)) {
    printf((" %.5f"), (a[i]));
  };
  printf("\n");
}
/** display a time array in one line */
void sp_times_display(sp_time_t* a, sp_time_t size) {
  sp_time_t i;
  printf("%lu", (a[0]));
  for (i = 1; (i < size); i = (1 + i)) {
    printf(" %lu", (a[i]));
  };
  printf("\n");
}
/** adjust amplitude of out to match the one of in */
void sp_samples_set_unity_gain(sp_sample_t* in, sp_time_t in_size, sp_sample_t* out) {
  sp_time_t i;
  sp_sample_t in_max;
  sp_sample_t out_max;
  sp_sample_t difference;
  sp_sample_t correction;
  in_max = sp_samples_absolute_max(in, in_size);
  out_max = sp_samples_absolute_max(out, in_size);
  if ((0 == in_max) || (0 == out_max)) {
    return;
  };
  difference = (out_max / in_max);
  correction = (1 + ((1 - difference) / difference));
  for (i = 0; (i < in_size); i = (1 + i)) {
    out[i] = (correction * out[i]);
  };
}
