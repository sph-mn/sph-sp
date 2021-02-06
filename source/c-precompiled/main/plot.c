uint32_t sp_plot_temp_file_index = 0;
#define sp_plot_temp_path "/tmp/sp-plot"
#define sp_plot_temp_file_index_maxlength 10
#define sp_plot_command_pattern_lines "gnuplot --persist -e 'set key off; set size ratio 0.5; plot \"%s\" with lines lc rgb \"blue\"'"
#define sp_plot_command_pattern_steps "gnuplot --persist -e 'set key off; set size ratio 0.5; plot \"%s\" with histeps lc rgb \"blue\"'"
#define sp_plot_command_pattern_bars "gnuplot --persist -e 'set key off; set size ratio 0.5; set grid; plot \"%s\" with steps lc rgb \"red\"'"
void sp_plot_samples_to_file(sp_sample_t* a, sp_time_t a_size, uint8_t* path) {
  FILE* file;
  sp_time_t i;
  file = fopen(path, "w");
  for (i = 0; (i < a_size); i = (1 + i)) {
    fprintf(file, ("%.3f\n"), (a[i]));
  };
  fclose(file);
}
void sp_plot_times_to_file(sp_time_t* a, sp_time_t a_size, uint8_t* path) {
  FILE* file;
  sp_time_t i;
  file = fopen(path, "w");
  for (i = 0; (i < a_size); i += 1) {
    fprintf(file, "%lu\n", (a[i]));
  };
  fclose(file);
}
void sp_plot_samples_file(uint8_t* path, uint8_t use_steps) {
  uint8_t* command;
  uint8_t* command_pattern;
  size_t command_size;
  command_pattern = (use_steps ? sp_plot_command_pattern_steps : sp_plot_command_pattern_lines);
  command_size = (strlen(path) + strlen(command_pattern));
  command = malloc(command_size);
  if (!command) {
    return;
  };
  snprintf(command, command_size, command_pattern, path);
  system(command);
  free(command);
}
void sp_plot_times_file(uint8_t* path, uint8_t use_steps) {
  uint8_t* command;
  uint8_t* command_pattern;
  size_t command_size;
  command_pattern = (use_steps ? sp_plot_command_pattern_steps : sp_plot_command_pattern_bars);
  command_size = (strlen(path) + strlen(command_pattern));
  command = malloc(command_size);
  if (!command) {
    return;
  };
  snprintf(command, command_size, command_pattern, path);
  system(command);
  free(command);
}
void sp_plot_samples(sp_sample_t* a, sp_time_t a_size) {
  uint8_t path_size = (1 + sp_plot_temp_file_index_maxlength + strlen(sp_plot_temp_path));
  uint8_t* path = calloc(path_size, 1);
  if (!path) {
    return;
  };
  snprintf(path, path_size, "%s-%lu", sp_plot_temp_path, sp_plot_temp_file_index);
  sp_plot_temp_file_index = (1 + sp_plot_temp_file_index);
  sp_plot_samples_to_file(a, a_size, path);
  sp_plot_samples_file(path, 0);
  free(path);
}
void sp_plot_times(sp_time_t* a, sp_time_t a_size) {
  uint8_t path_size = (1 + sp_plot_temp_file_index_maxlength + strlen(sp_plot_temp_path));
  uint8_t* path = calloc(path_size, 1);
  if (!path) {
    return;
  };
  snprintf(path, path_size, "%s-%lu", sp_plot_temp_path, sp_plot_temp_file_index);
  sp_plot_temp_file_index = (1 + sp_plot_temp_file_index);
  sp_plot_times_to_file(a, a_size, path);
  sp_plot_times_file(path, 1);
  free(path);
}

/** take the fft for given samples, convert complex values to magnitudes and write plot data to file */
void sp_plot_spectrum_to_file(sp_sample_t* a, sp_time_t a_size, uint8_t* path) {
  FILE* file;
  sp_time_t i;
  double* imag;
  double* real;
  imag = calloc(a_size, (sizeof(sp_sample_t)));
  if (!imag) {
    return;
  };
  real = malloc((sizeof(sp_sample_t) * a_size));
  if (!real) {
    return;
  };
  memcpy(real, a, (sizeof(sp_sample_t) * a_size));
  if (sp_fft(a_size, real, imag)) {
    return;
  };
  file = fopen(path, "w");
  for (i = 0; (i < a_size); i = (1 + i)) {
    fprintf(file, ("%.3f\n"), (2 * (sqrt(((real[i] * real[i]) + (imag[i] * imag[i]))) / a_size)));
  };
  fclose(file);
  free(imag);
  free(real);
}
void sp_plot_spectrum_file(uint8_t* path) { sp_plot_samples_file(path, 1); }
void sp_plot_spectrum(sp_sample_t* a, sp_time_t a_size) {
  uint8_t path_size = (1 + sp_plot_temp_file_index_maxlength + strlen(sp_plot_temp_path));
  uint8_t* path = calloc(path_size, 1);
  if (!path) {
    return;
  };
  snprintf(path, path_size, "%s-%lu", sp_plot_temp_path, sp_plot_temp_file_index);
  sp_plot_temp_file_index = (1 + sp_plot_temp_file_index);
  sp_plot_spectrum_to_file(a, a_size, path);
  sp_plot_spectrum_file(path);
  free(path);
}
