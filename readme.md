# sph-sp
c code and shared library for sound file input/output and example implementations of sound processors

# features
* file input/output with many supported file [formats](http://www.mega-nerd.com/libsndfile/)
* 64 bit float sample format by default, other formats possible for many processors
* unlimited channel count and sample rate
* processing of non-interleaved sample arrays with one array per channel
* processors for continuous data streams
  * windowed sinc low-pass, high-pass, band-pass and band-reject filters
  * moving average filter
  * convolution
* fast fourier transform (fft) and inverse fast fourier transform (ifft)

# dependencies
* run-time
  * libc
  * libsndfile
  * linux or compatible
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
the file `source/sph-sp.h` can be edited before compilation to set the following options.
the options can also be set before inclusion of sph-sp.h, but the shared library used must have been compiled with the same configuration.

| name | default | description |
| --- | --- | --- |
|sp-channel-count-t|uint8-t|data type for numbers of channels|
|sp-file-format|(SF-FORMAT-WAV \| SF-FORMAT-DOUBLE)|soundfile file format. a combination of file and sample format soundfile constants, for example (SF-FORMAT-AU \| SF-FORMAT-DOUBLE). resampling is done automatically as necessary|
|sp-float-t|double|data type for floating point values other than samples|
|sp-sample-count-t|size-t|data type for numbers of samples|
|sp-sample-rate-t|uint32-t|data type for sample rates|
|sp-sample-sum|f64-sum|function (sp-sample-t* size-t -> sp-sample-t) that sums samples, by default with kahan error compensation|
|sp-sample-t|double|sample format|
|sp-sf-read|sf-readf-double|libsndfile file reader function to use|
|sp-sf-write|sf-writef-double|libsndfile file writer function to use|

# c usage
```
#include <sph-sp.h>
```

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
sp_alloc_channel_array :: sp_channel_count_t:channel_count sp_sample_count_t:sample_count sp_sample_t***:result_array -> status_t
sp_channel_data_free :: sp_sample_t**:a sp_channel_count_t:channel_count -> void
sp_convolution_filter :: sp_sample_t*:in sp_sample_count_t:in_len sp_convolution_filter_ir_f_t:ir_f void*:ir_f_arguments uint8_t:ir_f_arguments_len sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_convolution_filter_state_free :: sp_convolution_filter_state_t*:state -> void
sp_convolution_filter_state_set :: sp_convolution_filter_ir_f_t:ir_f void*:ir_f_arguments uint8_t:ir_f_arguments_len sp_convolution_filter_state_t**:out_state -> status_t
sp_convolve :: sp_sample_t*:a sp_sample_count_t:a_len sp_sample_t*:b sp_sample_count_t:b_len sp_sample_count_t:result_carryover_len sp_sample_t*:result_carryover sp_sample_t*:result_samples -> void
sp_convolve_one :: sp_sample_t*:a sp_sample_count_t:a_len sp_sample_t*:b sp_sample_count_t:b_len sp_sample_t*:result_samples -> void
sp_fft :: sp_sample_count_t:input_len sp_sample_t*:input_real sp_sample_t*:input_imag sp_sample_t*:output_real sp_sample_t*:output_imag -> status_t
sp_ffti :: sp_sample_count_t:input_len sp_sample_t*:input_real sp_sample_t*:input_imag sp_sample_t*:output_real sp_sample_t*:output_imag -> status_t
sp_file_close :: sp_file_t*:a -> status_t
sp_file_open :: uint8_t*:path int:mode sp_channel_count_t:channel_count sp_sample_rate_t:sample_rate sp_file_t*:result_file -> status_t
sp_file_position :: sp_file_t*:file sp_sample_count_t*:result_position -> status_t
sp_file_position_set :: sp_file_t*:file size_t:sample_offset -> status_t
sp_file_read :: sp_file_t*:file sp_sample_count_t:sample_count sp_sample_t**:result_channel_data sp_sample_count_t*:result_sample_count -> status_t
sp_file_write :: sp_file_t*:file sp_sample_t**:channel_data sp_sample_count_t:sample_count sp_sample_count_t*:result_sample_count -> status_t
sp_moving_average :: sp_sample_t*:source sp_sample_count_t:source_len sp_sample_t*:prev sp_sample_count_t:prev_len sp_sample_t*:next sp_sample_count_t:next_len sp_sample_count_t:radius sp_sample_count_t:start sp_sample_count_t:end sp_sample_t*:result_samples -> status_t
sp_null_ir :: sp_sample_t**:out_ir sp_sample_count_t*:out_len -> status_t
sp_passthrough_ir :: sp_sample_t**:out_ir sp_sample_count_t*:out_len -> status_t
sp_sin_lq :: sp_float_t:a -> sp_sample_t
sp_sinc :: sp_float_t:a -> sp_float_t
sp_spectral_inversion_ir :: sp_sample_t*:a sp_sample_count_t:a_len -> void
sp_spectral_reversal_ir :: sp_sample_t*:a sp_sample_count_t:a_len -> void
sp_status_description :: status_t:a -> uint8_t*
sp_status_name :: status_t:a -> uint8_t*
sp_window_blackman :: sp_float_t:a sp_sample_count_t:width -> sp_float_t
sp_windowed_sinc_bp_br :: sp_sample_t*:in sp_sample_count_t:in_len sp_float_t:cutoff_l sp_float_t:cutoff_h sp_float_t:transition_l sp_float_t:transition_h boolean:is_reject sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_windowed_sinc_bp_br_ir :: sp_float_t:cutoff_l sp_float_t:cutoff_h sp_float_t:transition_l sp_float_t:transition_h boolean:is_reject sp_sample_t**:out_ir sp_sample_count_t*:out_len -> status_t
sp_windowed_sinc_bp_br_ir_f :: void*:arguments sp_sample_t**:out_ir sp_sample_count_t*:out_len -> status_t
sp_windowed_sinc_ir :: sp_float_t:cutoff sp_float_t:transition sp_sample_count_t*:result_len sp_sample_t**:result_ir -> status_t
sp_windowed_sinc_lp_hp :: sp_sample_t*:in sp_sample_count_t:in_len sp_float_t:cutoff sp_float_t:transition boolean:is_high_pass sp_convolution_filter_state_t**:out_state sp_sample_t*:out_samples -> status_t
sp_windowed_sinc_lp_hp_ir :: sp_float_t:cutoff sp_float_t:transition boolean:is_high_pass sp_sample_t**:out_ir sp_sample_count_t*:out_len -> status_t
sp_windowed_sinc_lp_hp_ir :: sp_float_t:cutoff sp_float_t:transition boolean:is_high_pass sp_sample_t**:out_ir sp_sample_count_t*:out_len -> status_t
sp_windowed_sinc_lp_hp_ir_f :: void*:arguments sp_sample_t**:out_ir sp_sample_count_t*:out_len -> status_t
sp_windowed_sinc_lp_hp_ir_length :: sp_float_t:transition -> sp_sample_count_t
```

## macros
```
sp_file_bit_closed
sp_file_bit_input
sp_file_bit_output
sp_file_bit_position
sp_file_mode_read
sp_file_mode_read_write
sp_file_mode_write
sp_octets_to_samples(a)
sp_samples_to_octets(a)
sp_status_group_libc
sp_status_group_sndfile
sp_status_group_sp
sp_status_group_sph
sp_windowed_sinc_ir_cutoff(freq, sample_rate)
sp_windowed_sinc_ir_transition
status_declare
status_declare_group(group)
status_goto
status_group_undefined
status_id_require(expression)
status_id_success
status_is_failure
status_is_success
status_require(expression)
status_reset
status_set_both(group_id, status_id)
status_set_both_goto(group_id, status_id)
status_set_group_goto(group_id)
status_set_id_goto(status_id)
```

## types
```
status_id_t: int32_t
sp_convolution_filter_ir_f_t: void* sp_sample_t** sp_sample_count_t* -> status_t
sp_convolution_filter_state_t: struct
  carryover: sp_sample_t*
  carryover_len: sp_sample_count_t
  carryover_alloc_len: sp_sample_count_t
  ir: sp_sample_t*
  ir_f: sp_convolution_filter_ir_f_t
  ir_f_arguments: void*
  ir_len: sp_sample_count_t
sp_file_t: struct
  flags: uint8_t
  sample_rate: sp_sample_rate_t
  channel_count: sp_channel_count_t
  data: void*
status_t: struct
  id: status_id_t
  group: uint8_t*
```

## enum
```
sp_status_id_file_channel_mismatch sp_status_id_file_encoding sp_status_id_file_header
  sp_status_id_file_incompatible sp_status_id_file_incomplete sp_status_id_eof
  sp_status_id_input_type sp_status_id_memory sp_status_id_invalid_argument
  sp_status_id_not_implemented sp_status_id_file_closed sp_status_id_file_position
  sp_status_id_file_type sp_status_id_undefined
```

# other language bindings
* scheme: [sph-sp-guile](https://github.com/sph-mn/sph-sp-guile)

# license
* files under `source/c/foreign/nayuki-fft`: mit license
* rest: lgpl3+

# thanks to
* [tom roelandts](https://tomroelandts.com/) on whose information the windowed sinc filters are based on
* [mborg](https://github.com/mborgerding/kissfft) for the first fft implementation that was used
* [nayuki](https://www.nayuki.io/page/free-small-fft-in-multiple-languages) for a concise fft implementation that is currently used
* [steve smith's dspguide](http://www.dspguide.com/) for information about dsp theory

# other interesting projects
* [yodel](https://github.com/rclement/yodel)
* [soundpipe](https://github.com/PaulBatchelor/Soundpipe)
* [ciglet](https://github.com/Sleepwalking/ciglet)
