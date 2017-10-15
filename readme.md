# sph-sp
basic utilities for digital sound processing as a shared library and c code.
reference implementation for learning and, with some extra testing, practical usage.
prefers higher precision to faster calculation.

# features
* convolution
* windowed sinc filter
* moving average filter
* processors work for segments of continous streams
* 32 bit float samples by default
* avoids and compensates for rounding errors
* sample format customisable to some extent
* io for alsa and au format files

# dependencies
* run-time
  * alsa
  * libc
* quick build
  * gcc and shell for the provided compile script
* development build
  * sph-sc

# setup
```
./exe/compile-c
./exe/install
```

# usage
```
#include <sph-sp.c>
```

# types
```c
status_i_t
sp_port_t struct
  b32 sample_rate;
  b32 channel_count;
  boolean closed_p;
  b8 flags;
  b8 type;
  b64 position;
  b16 position_offset;
  b0* data;
  int data_int;
sp_windowed_sinc_state_t struct
  sp_sample_t* data;
  size_t data_len;
  size_t ir_len_prev;
  sp_sample_t* ir;
  size_t ir_len;
  b32 sample_rate;
  f32_s freq;
  f32_s transition;
status_t struct
  status_i_t id;
  b8 group;
```

# enum
```c
sp_status_id_undefined, sp_status_id_input_type, sp_status_id_not_implemented,
sp_status_id_memory, sp_status_id_file_incompatible, sp_status_id_file_encoding,
sp_status_id_file_header, sp_status_id_port_closed, sp_status_id_port_position,
sp_status_id_file_channel_mismatch, sp_status_id_file_incomplete, sp_status_id_port_type,
sp_status_group_sp, sp_status_group_libc, sp_status_group_alsa
```

# routines
```c
b0 sp_convolve(sp_sample_t* result, sp_sample_t* a, size_t a_len, sp_sample_t* b, size_t b_len, sp_sample_t* carryover, size_t carryover_len)
b0 sp_convolve_one(sp_sample_t* result, sp_sample_t* a, size_t a_len, sp_sample_t* b, size_t b_len)
b0 sp_sine(sp_sample_t* data, b32 start, b32 end, f32_s sample_duration, f32_s freq, f32_s phase, f32_s amp)
b0 sp_sine_lq(sp_sample_t* data, b32 start, b32 end, f32_s sample_duration, f32_s freq, f32_s phase, f32_s amp)
b0 sp_spectral_inversion_ir(sp_sample_t* a, size_t a_len)
b0 sp_spectral_reversal_ir(sp_sample_t* a, size_t a_len)
b0 sp_windowed_sinc_ir(sp_sample_t** result, size_t* result_len, b32 sample_rate, f32_s freq, f32_s transition)
b0 sp_windowed_sinc_state_destroy(sp_windowed_sinc_state_t* state)
b8 sp_windowed_sinc_state_create(b32 sample_rate, f32_s freq, f32_s transition, sp_windowed_sinc_state_t** state)
b8* sp_status_description(status_t a)
b8* sp_status_name(status_t a)
boolean sp_moving_average(sp_sample_t* result, sp_sample_t* source, b32 source_len, sp_sample_t* prev, b32 prev_len, sp_sample_t* next, b32 next_len, b32 start, b32 end, b32 distance)
f32_s sp_sin_lq(f32_s a)
f32_s sp_sinc(f32_s a)
f32_s sp_window_blackman(f32_s a, size_t width)
size_t sp_windowed_sinc_ir_length(f32_s transition)
sp_sample_t sample_reverse_endian(sp_sample_t a)
sp_sample_t** sp_alloc_channel_data(b32 channel_count, b32 sample_count)
status_i_t sp_windowed_sinc(sp_sample_t* result, sp_sample_t* source, size_t source_len, b32 sample_rate, f32_s freq, f32_s transition, sp_windowed_sinc_state_t** state)
status_t sp_alsa_open(sp_port_t* result, b8* device_name, boolean input_p, b32_s channel_count, b32_s sample_rate, b32_s latency)
status_t sp_fft(sp_sample_t* result, b32 result_len, sp_sample_t* source, b32 source_len)
status_t sp_file_open(sp_port_t* result, b8* path, b32_s channel_count, b32_s sample_rate)
status_t sp_ifft(sp_sample_t* result, b32 result_len, sp_sample_t* source, b32 source_len)
status_t sp_port_close(sp_port_t* a)
status_t sp_port_read(sp_sample_t** result, sp_port_t* port, b32 sample_count)
status_t sp_port_write(sp_port_t* port, b32 sample_count, sp_sample_t** channel_data)
```

# macros
```c
sp_alsa_status_require_x(expression)
sp_default_alsa_enable_soft_resample
sp_default_alsa_latency
sp_default_channel_count
sp_default_sample_rate
sp_octets_to_samples(a)
sp_port_bit_input
sp_port_bit_output
sp_port_bit_position
sp_port_type_alsa
sp_port_type_file
sp_sample_t
sp_samples_to_octets(a)
sp_status_init
sp_status_require_alloc(a)
sp_system_status_require_id(id)
sp_system_status_require_x(expression)
sp_windowed_sinc_cutoff(freq, sample_rate)
status_failure_p
status_goto
status_group_undefined
status_id_is_p(status_id)
status_id_success
status_init
status_init_group(group)
status_require
status_require_x(expression)
status_reset
status_set_both(group_id, status_id)
status_set_both_goto(group_id, status_id)
status_set_group(group_id)
status_set_group_goto(group_id)
status_set_id(status_id)
status_set_id_goto(status_id)
status_success_p
```
