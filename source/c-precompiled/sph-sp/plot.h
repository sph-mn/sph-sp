
#define sp_block_plot_1(a) sp_plot_samples(((a.samples)[0]), (a.size))
void sp_plot_samples(sp_sample_t* a, sp_time_t a_size);
void sp_plot_times(sp_time_t* a, sp_time_t a_size);
void sp_plot_samples_to_file(sp_sample_t* a, sp_time_t a_size, uint8_t* path);
void sp_plot_times_to_file(sp_time_t* a, sp_time_t a_size, uint8_t* path);
void sp_plot_samples_file(uint8_t* path, uint8_t use_steps);
void sp_plot_spectrum_to_file(sp_sample_t* a, sp_time_t a_size, uint8_t* path);
void sp_plot_spectrum_file(uint8_t* path);
void sp_plot_spectrum(sp_sample_t* a, sp_time_t a_size);