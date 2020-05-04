# sph-sp
c code and shared library for sound file input/output and example implementations of sound processors

# features
* basics
  * file input/output with many supported file [formats](http://www.mega-nerd.com/libsndfile/)
  * 64 bit float sample format by default, other formats possible for many processors
  * processing of non-interleaved sample arrays with one array per channel. number of channels and sample rate can be custom set
  * fast fourier transform (fft) and inverse fast fourier transform (ifft)
  * plotting of samples and sound spectra using gnuplot
* processors that can work seamlessly on blocks of continuous data streams
  * windowed-sinc low-pass, high-pass, band-pass and band-reject filters
  * state-variable high/low/band/all-pass filter
  * moving average filter
  * convolution
* synthesis
  * paths created from interpolation between given points. can be used for amplitude, wavelength and other controls
  * additive fm synthesizer with parameters per channel and start/end time for partials
  * sequencer for parallel event processing of custom routines
  * events for filtered noise and synth output
  * sine/triangle/square/sawtooth-wave and noise generator

# dependencies
* run-time
  * libc
  * libsndfile
  * linux or compatible
  * optional: gnuplot
* quick build
  * gcc and shell for the provided compile script
* development build
  * [sph-sc](https://github.com/sph-mn/sph-sc)

# setup
* install dependencies
* then in the project directory execute

```
./exe/compile-c
./exe/install
```

first argument to `exe/install` can be the destination path prefix, for example `./exe/install /tmp`

installed files
* /usr/include/sph-sp.h
* /usr/lib/libsph-sp.so

# compile-time configuration options
the file `source/c-precompiled/main/sph-sp.h` can be edited before compilation to set the following options. the values must be defined before including the header or the edited header must then be used with the shared library.

| name | default | description |
| --- | --- | --- |
|sp_channels_t|uint8_t|data type for numbers of channels|
|sp_file_format|(SF_FORMAT_WAV \| SF_FORMAT_DOUBLE)|soundfile file format. a combination of file and sample format soundfile constants, for example (SF_FORMAT_AU \| SF_FORMAT_DOUBLE). conversion is done automatically as necessary|
|sp_float_t|double|data type for floating point values other than samples|
|sp_channel_limit|2|maximum number of channels|
|sp_count_t|size_t|data type for numbers of samples|
|sp_sample_rate_t|uint32_t|data type for sample rates|
|sp_sample_sum|f64_sum|function (sp_sample_t* size_t -> sp_sample_t) that sums samples, by default with kahan error compensation|
|sp_sample_t|double|sample format. f32 should be possible but integer formats are currently not possible because most processors dont work with them|
|sp_sf_read|sf_readf_double|libsndfile file reader function to use|
|sp_sf_write|sf_writef_double|libsndfile file writer function to use|
|sp_synth_count_t|uint16_t|the datatype for the number of partials for sp_synth|
|sp_synth_sine|sp_sine_96||
|sp_synth_partial_limit|128|maximum number of partials sp_synth will be able to process|

# c usage
```
#include <sph-sp.h>
```
call sp_initialise(cpu-count) once somewhere

compilation with gcc
```
gcc -lsph-sp main.c
```

## error handling
routines return `status_t`, which is a `struct {int id, uint8-t* group}`.
status.id zero is success

# api
## routines
```
sp_block_to_file :: sp_block_t:block uint8_t*:path sp_time_t:rate -> status_t
sp_block_free :: sp_block_t:a -> void
sp_block_new :: sp_channels_t:channel_count sp_time_t:sample_count sp_block_t*:out_block -> status_t
sp_block_with_offset :: sp_block_t:a sp_time_t:offset -> sp_block_t
sp_cheap_filter :: sp_state_variable_filter_t:type sp_sample_t*:in sp_time_t:in_size sp_float_t:cutoff sp_time_t:passes sp_float_t:q_factor uint8_t:unity_gain sp_cheap_filter_state_t*:state sp_sample_t*:out -> void
sp_cheap_filter_state_free :: sp_cheap_filter_state_t*:a -> void
sp_cheap_filter_state_new :: sp_time_t:max_size sp_time_t:max_passes sp_cheap_filter_state_t*:out_state -> status_t
sp_cheap_noise_event :: sp_time_t:start sp_time_t:end sp_sample_t**:amp sp_state_variable_filter_t:type sp_sample_t*:cut sp_time_t:passes sp_sample_t:q_factor sp_time_t:resolution sp_random_state_t:random_state sp_event_t*:out_event -> status_t
sp_convolution_filter :: sp_sample_t*:in sp_time_t:in_len sp_convolution_filter_ir_f_t:ir_f void*:ir_f_arguments uint8_t:ir_f_arguments_len sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_convolution_filter_state_free :: sp_convolution_filter_state_t*:state -> void
sp_convolution_filter_state_set :: sp_convolution_filter_ir_f_t:ir_f void*:ir_f_arguments uint8_t:ir_f_arguments_len sp_convolution_filter_state_t**:out_state -> status_t
sp_convolve :: sp_sample_t*:a sp_time_t:a_len sp_sample_t*:b sp_time_t:b_len sp_time_t:result_carryover_len sp_sample_t*:result_carryover sp_sample_t*:result_samples -> void
sp_convolve_one :: sp_sample_t*:a sp_time_t:a_len sp_sample_t*:b sp_time_t:b_len sp_sample_t*:result_samples -> void
sp_count_array_new :: sp_time_t:size sp_time_t**:out -> status_t
sp_events_free :: sp_event_t*:events sp_time_t:size -> void
sp_fft :: sp_time_t:input_len double*:input_or_output_real double*:input_or_output_imag -> int
sp_ffti :: sp_time_t:input_len double*:input_or_output_real double*:input_or_output_imag -> int
sp_file_close :: sp_file_t*:a -> status_t
sp_file_open :: uint8_t*:path int:mode sp_channels_t:channel_count sp_sample_rate_t:sample_rate sp_file_t*:result_file -> status_t
sp_file_position :: sp_file_t*:file sp_time_t*:result_position -> status_t
sp_file_position_set :: sp_file_t*:file sp_time_t:sample_offset -> status_t
sp_file_read :: sp_file_t*:file sp_time_t:sample_count sp_sample_t**:result_block sp_time_t*:result_sample_count -> status_t
sp_file_write :: sp_file_t*:file sp_sample_t**:block sp_time_t:sample_count sp_time_t*:result_sample_count -> status_t
sp_filter :: sp_sample_t*:in sp_time_t:in_size sp_float_t:cutoff_l sp_float_t:cutoff_h sp_float_t:transition_l sp_float_t:transition_h boolean:is_reject sp_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_initialise :: uint16_t:cpu_count -> status_t
sp_moving_average :: sp_sample_t*:in sp_sample_t*:in_end sp_sample_t*:in_window sp_sample_t*:in_window_end sp_sample_t*:prev sp_sample_t*:prev_end sp_sample_t*:next sp_sample_t*:next_end sp_time_t:radius sp_sample_t*:out -> status_t
sp_noise_event :: sp_time_t:start sp_time_t:end sp_sample_t**:amp sp_sample_t*:cut_l sp_sample_t*:cut_h sp_sample_t*:trn_l sp_sample_t*:trn_h uint8_t:is_reject sp_time_t:resolution sp_random_state_t:random_state sp_event_t*:out_event -> status_t
sp_null_ir :: sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_passthrough_ir :: sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_phase_96 :: sp_time_t:current sp_time_t:change -> sp_time_t
sp_phase_96_float :: sp_time_t:current double:change -> sp_time_t
sp_plot_counts :: sp_time_t*:a sp_time_t:a_size -> void
sp_plot_counts_to_file :: sp_time_t*:a sp_time_t:a_size uint8_t*:path -> void
sp_plot_samples :: sp_sample_t*:a sp_time_t:a_size -> void
sp_plot_samples_to_file :: sp_sample_t*:a sp_time_t:a_size uint8_t*:path -> void
sp_plot_samples_file :: uint8_t*:path uint8_t:use_steps -> void
sp_plot_spectrum :: sp_sample_t*:a sp_time_t:a_size -> void
sp_plot_spectrum_to_file :: sp_sample_t*:a sp_time_t:a_size uint8_t*:path -> void
sp_plot_spectrum_file :: uint8_t*:path -> void
sp_random_samples :: sp_random_state_t*:state sp_time_t:size sp_sample_t*:out -> void
sp_sample_array_new :: sp_time_t:size sp_sample_t**:out -> status_t
sp_seq :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:events sp_time_t:size -> void
sp_seq_events_prepare :: sp_event_t*:data sp_time_t:size -> void
sp_seq_parallel :: sp_time_t:start sp_time_t:end sp_block_t:out sp_event_t*:events sp_time_t:size -> status_t
sp_sin_lq :: sp_float_t:a -> sp_sample_t
sp_sinc :: sp_float_t:a -> sp_float_t
sp_sine_table_new :: sp_sample_t**:out sp_time_t:size -> status_t
sp_spectral_inversion_ir :: sp_sample_t*:a sp_time_t:a_len -> void
sp_spectral_reversal_ir :: sp_sample_t*:a sp_time_t:a_len -> void
sp_square_96 :: sp_time_t:t -> sp_sample_t
sp_state_variable_filter_all :: sp_sample_t*:out sp_sample_t*:in sp_float_t:in_count sp_float_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_bp :: sp_sample_t*:out sp_sample_t*:in sp_float_t:in_count sp_float_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_br :: sp_sample_t*:out sp_sample_t*:in sp_float_t:in_count sp_float_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_hp :: sp_sample_t*:out sp_sample_t*:in sp_float_t:in_count sp_float_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_lp :: sp_sample_t*:out sp_sample_t*:in sp_float_t:in_count sp_float_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_state_variable_filter_peak :: sp_sample_t*:out sp_sample_t*:in sp_float_t:in_count sp_float_t:cutoff sp_time_t:q_factor sp_sample_t*:state -> void
sp_status_description :: status_t:a -> uint8_t*
sp_status_name :: status_t:a -> uint8_t*
sp_synth :: sp_block_t:out sp_time_t:start sp_time_t:duration sp_synth_count_t:config_len sp_synth_partial_t*:config sp_time_t*:phases -> status_t
sp_synth_event :: sp_time_t:start sp_time_t:end sp_time_t:channel_count sp_time_t:config_len sp_synth_partial_t*:config sp_event_t*:out_event -> status_t
sp_synth_partial_1 :: sp_time_t:start sp_time_t:end sp_synth_count_t:modifies sp_sample_t*:amp sp_time_t*:wvl sp_time_t:phs -> sp_synth_partial_t
sp_synth_partial_2 :: sp_time_t:start sp_time_t:end sp_synth_count_t:modifies sp_sample_t*:amp1 sp_sample_t*:amp2 sp_time_t*:wvl1 sp_time_t*:wvl2 sp_time_t:phs1 sp_time_t:phs2 -> sp_synth_partial_t
sp_synth_state_new :: sp_time_t:channel_count sp_synth_count_t:config_len sp_synth_partial_t*:config sp_time_t**:out_state -> status_t
sp_time_from_samples :: sp_sample_t*:in sp_time_t:in_size sp_time_t*:out -> void
sp_triangle :: sp_time_t:t sp_time_t:a sp_time_t:b -> sp_sample_t
sp_triangle_96 :: sp_time_t:t -> sp_sample_t
sp_window_blackman :: sp_float_t:a sp_time_t:width -> sp_float_t
sp_windowed_sinc_bp_br :: sp_sample_t*:in sp_time_t:in_len sp_float_t:cutoff_l sp_float_t:cutoff_h sp_float_t:transition_l sp_float_t:transition_h boolean:is_reject sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_windowed_sinc_bp_br_ir :: sp_float_t:cutoff_l sp_float_t:cutoff_h sp_float_t:transition_l sp_float_t:transition_h boolean:is_reject sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_bp_br_ir_f :: void*:arguments sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_ir :: sp_float_t:cutoff sp_float_t:transition sp_time_t*:result_len sp_sample_t**:result_ir -> status_t
sp_windowed_sinc_lp_hp :: sp_sample_t*:in sp_time_t:in_len sp_float_t:cutoff sp_float_t:transition boolean:is_high_pass sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_windowed_sinc_lp_hp_ir :: sp_float_t:cutoff sp_float_t:transition boolean:is_high_pass sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_lp_hp_ir :: sp_float_t:cutoff sp_float_t:transition boolean:is_high_pass sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_lp_hp_ir_f :: void*:arguments sp_sample_t**:out_ir sp_time_t*:out_len -> status_t
sp_windowed_sinc_lp_hp_ir_length :: sp_float_t:transition -> sp_time_t
sph_random :: sph_random_state_t*:state u32:size f64*:out -> void
sph_random_state_new :: u64:seed -> sph_random_state_t
spline_path_bezier :: spline_path_time_t:x1 spline_path_value_t:y1 spline_path_time_t:x2 spline_path_value_t:y2 spline_path_time_t:x3 spline_path_value_t:y3 -> spline_path_segment_t
spline_path_constant :: -> spline_path_segment_t
spline_path_end :: spline_path_t:path -> spline_path_point_t
spline_path_free :: spline_path_t:a -> void
spline_path_get :: spline_path_t:path spline_path_time_t:start spline_path_time_t:end spline_path_value_t*:out -> void
spline_path_i_bezier :: spline_path_time_t:start spline_path_time_t:end spline_path_point_t:p_start spline_path_point_t*:p_rest void*:options spline_path_value_t*:out -> void
spline_path_i_constant :: spline_path_time_t:start spline_path_time_t:end spline_path_point_t:p_start spline_path_point_t*:p_rest void*:options spline_path_value_t*:out -> void
spline_path_i_line :: spline_path_time_t:start spline_path_time_t:end spline_path_point_t:p_start spline_path_point_t*:p_rest void*:options spline_path_value_t*:out -> void
spline_path_i_move :: spline_path_time_t:start spline_path_time_t:end spline_path_point_t:p_start spline_path_point_t*:p_rest void*:options spline_path_value_t*:out -> void
spline_path_i_path :: spline_path_time_t:start spline_path_time_t:end spline_path_point_t:p_start spline_path_point_t*:p_rest void*:options spline_path_value_t*:out -> void
spline_path_line :: spline_path_time_t:x spline_path_value_t:y -> spline_path_segment_t
spline_path_move :: spline_path_time_t:x spline_path_value_t:y -> spline_path_segment_t
spline_path_new :: spline_path_segment_count_t:segments_len spline_path_segment_t*:segments spline_path_t*:out_path -> uint8_t
spline_path_new_get :: spline_path_segment_count_t:segments_len spline_path_segment_t*:segments spline_path_time_t:start spline_path_time_t:end spline_path_value_t*:out -> uint8_t
spline_path_new_get_2 :: sp_sample_t*:out sp_time_t:duration spline_path_segment_t:s1 spline_path_segment_t:s2 -> int
spline_path_new_get_3 :: sp_sample_t*:out sp_time_t:duration spline_path_segment_t:s1 spline_path_segment_t:s2 spline_path_segment_t:s3 -> int
spline_path_new_get_4 :: sp_sample_t*:out sp_time_t:duration spline_path_segment_t:s1 spline_path_segment_t:s2 spline_path_segment_t:s3 spline_path_segment_t:s4 -> int
spline_path_path :: spline_path_t*:path -> spline_path_segment_t
spline_path_start :: spline_path_t:path -> spline_path_point_t
```

## macros
```
boolean
boolean
define_sph_random(name, size_type, data_type, transfer)
f32
f64
f64
f64_from_u64(a)
i16
i16_fast
i16_least
i32
i32_fast
i32_least
i64
i64_fast
i64_least
i8
i8_fast
i8_least
sp_block_set_null(a)
sp_cheap_ceiling_positive(a)
sp_cheap_filter_bp(...)
sp_cheap_filter_br(...)
sp_cheap_filter_hp(...)
sp_cheap_filter_lp(...)
sp_cheap_filter_passes_limit
sp_cheap_floor_positive(a)
sp_cheap_noise_event_bp(start, end, amp, ...)
sp_cheap_noise_event_br(start, end, amp, ...)
sp_cheap_noise_event_hp(start, end, amp, ...)
sp_cheap_noise_event_lp(start, end, amp, ...)
sp_cheap_round_positive(a)
sp_event_move(event, start)
sp_event_duration(event)
sp_event_duration_set(event, duration)
sp_file_bit_closed
sp_file_bit_input
sp_file_bit_output
sp_file_bit_position
sp_file_mode_read
sp_file_mode_read_write
sp_file_mode_write
sp_filter_state_free
sp_filter_state_t
sp_octets_to_samples(a)
sp_random
sp_random_state_new
sp_random_state_t
sp_s_group_libc
sp_s_group_sndfile
sp_s_group_sp
sp_s_group_sph
sp_sample_array_zero(a, size)
sp_samples_to_octets(a)
sp_sine_96(t)
spline_path_interpolator_points_len(a)
spline_path_point_limit
status_declare
status_group_undefined
status_i_require(expression)
status_is_failure
status_is_success
status_require(expression)
status_return
status_set(group_id, status_id)
status_set_goto(group_id, status_id)
status_success
u16
u16_fast
u16_least
u32
u32_fast
u32_least
u64
u64_fast
u64_least
u8
u8_fast
u8_least
```

## variables
```
sp_random_state_t sp_default_random_state
sp_sample_t* sp_sine_96_table
```

## types
```
sp_convolution_filter_ir_f_t: void* sp_sample_t** sp_time_t* -> status_t
sp_event_f_t: sp_time_t sp_time_t sp_block_t sp_event_t* -> void
sp_state_variable_filter_t: sp_sample_t* sp_sample_t* sp_float_t sp_float_t sp_time_t sp_sample_t* -> void
spline_path_interpolator_t: spline_path_time_t spline_path_time_t spline_path_point_t spline_path_point_t* void* spline_path_value_t* -> void
sp_block_t: struct
  channels: sp_channels_t
  size: sp_time_t
  samples: array sp_sample_t* sp_channel_limit
sp_cheap_filter_state_t: struct
  in_temp: sp_sample_t*
  out_temp: sp_sample_t*
  svf_state: array sp_sample_t * 2 sp_cheap_filter_passes_limit
sp_convolution_filter_state_t: struct
  carryover: sp_sample_t*
  carryover_len: sp_time_t
  carryover_alloc_len: sp_time_t
  ir: sp_sample_t*
  ir_f: sp_convolution_filter_ir_f_t
  ir_f_arguments: void*
  ir_f_arguments_len: uint8_t
  ir_len: sp_time_t
sp_event_t: struct sp_event_t
  state: void*
  start: sp_time_t
  end: sp_time_t
  f: function_pointer void sp_time_t sp_time_t sp_block_t struct sp_event_t*
  free: function_pointer void struct sp_event_t*
sp_file_t: struct
  flags: uint8_t
  sample_rate: sp_sample_rate_t
  channel_count: sp_channels_t
  data: void*
sp_synth_event_state_t: struct
  config_len: sp_synth_count_t
  config: array sp_synth_partial_t sp_synth_partial_limit
  state: sp_time_t*
sp_synth_partial_t: struct
  start: sp_time_t
  end: sp_time_t
  modifies: sp_synth_count_t
  amp: array sp_sample_t* sp_channel_limit
  wvl: array sp_time_t* sp_channel_limit
  phs: array sp_time_t sp_channel_limit
sph_random_state_t: struct
  data: array u64 4
spline_path_point_t: struct
  x: spline_path_time_t
  y: spline_path_value_t
spline_path_segment_t: struct
  _start: spline_path_point_t
  _points_len: uint8_t
  points: array spline_path_point_t spline_path_point_limit
  interpolator: spline_path_interpolator_t
  options: void*
spline_path_t: struct
  segments_len: spline_path_segment_count_t
  segments: spline_path_segment_t*
status_t: struct
  id: int
  group: uint8_t*
```

## enum
```
sp_s_id_file_channel_mismatch sp_s_id_file_encoding sp_s_id_file_header
  sp_s_id_file_incompatible sp_s_id_file_incomplete sp_s_id_eof
  sp_s_id_input_type sp_s_id_memory sp_s_id_invalid_argument
  sp_s_id_not_implemented sp_s_id_file_closed sp_s_id_file_position
  sp_s_id_file_type sp_s_id_undefined
```

# license
* files under `source/c/foreign/nayuki-fft`: mit license
* rest: lgpl3+

# thanks to
* [tom roelandts](https://tomroelandts.com/) on whose information the windowed sinc filters are based on
* [mborg](https://github.com/mborgerding/kissfft) for the first fft implementation that was used
* [nayuki](https://www.nayuki.io/page/free-small-fft-in-multiple-languages) for the concise fft implementation that is currently used
* [steve smith's dspguide](http://www.dspguide.com/) for information about dsp theory
* [xoshiro.di.unimi.it](http://xoshiro.di.unimi.it/) for the pseudorandom number generator
