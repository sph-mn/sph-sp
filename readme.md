# sph-sp
basic sound processing toolset. c code and shared library

* create sample arrays and write them to files or alsa sound output using a generic port object
* various exemplary utilities, for example for convolution and filters

# features
* generic port object for alsa and file io with many supported file [formats](http://www.mega-nerd.com/libsndfile/)
* compile-time customisable sample format
* unlimited number of channels and sample rate
* by default, processing on non-interleaved sample arrays with one array per channel
* tries to avoid floating point errors, prioritises precision over performance

# dependencies
* run-time
  * libc
  * libsndfile
  * alsa
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

first argument to `exe/install` can be the destination path prefix, for example `./exe/install /tmp`.
there is also exe/install-extended which can symlink files but needs [sph-lib](https://github.com/sph-mn/sph-lib)

installed files
* /usr/include/sph-sp.h
* /usr/lib/libsph-sp.so

# configuration options
the file ``source/c-precompiled/config.c`` can be edited before compilation to set the following options

| name | default | description |
| --- | --- | --- |
|sp-file-format|(SF-FORMAT-WAV \| SF-FORMAT-DOUBLE)|soundfile file format. a combination of file and sample format soundfile constants, for example (SF-FORMAT-AU \| SF-FORMAT-DOUBLE). resampling is done automatically as necessary|
|sp-sample-format|sp-sample-format-f64|sample format used for internal processing. see below for possible values|
|sp-float-t|double||
|sp-default-sample-rate|16000|low sample rate for quick test listens|
|sp-default-channel-count|1||
|sp-default-alsa-enable-soft-resample|1||
|sp-default-alsa-latency|128||

## possible values for sp-sample-format
sp-sample-format is the format used internally for processing and is independent of the file format.
all internal sample formats are currently lower endian. the integer formats are preliminary because some dependencies which dont usually use integers like libc `sin` and therefore `sp-sine` might not work

| name | description |
| --- | --- |
|sp-sample-format-f64|64 bit floating point|
|sp-sample-format-f32|32 bit floating point|
|sp-sample-format-int32|32 bit signed integer|
|sp-sample-format-int16|16 bit signed integer|
|sp-sample-format-int8|8 bit signed integer|

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
sp_alsa_open :: uint8_t*:device_name boolean:is_input sp_channel_count_t:channel_count sp_sample_rate_t:sample_rate int32_t:latency sp_port_t*:result_port -> status_t
sp_channel_data_free :: sp_sample_t**:a sp_channel_count_t:channel_count -> void
sp_convolve :: sp_sample_t*:a sp_sample_count_t:a_len sp_sample_t*:b sp_sample_count_t:b_len sp_sample_count_t:result_carryover_len sp_sample_t*:result_carryover sp_sample_t*:result_samples -> void
sp_convolve_one :: sp_sample_t*:a sp_sample_count_t:a_len sp_sample_t*:b sp_sample_count_t:b_len sp_sample_t*:result_samples -> void
sp_fft :: sp_sample_count_t:len sp_sample_t*:source sp_sample_count_t:source_len sp_sample_t*:result_samples -> status_t
sp_file_open :: uint8_t*:path sp_channel_count_t:channel_count sp_sample_rate_t:sample_rate sp_port_t*:result_port -> status_t
sp_ifft :: sp_sample_count_t:len sp_sample_t*:source sp_sample_count_t:source_len sp_sample_t*:result_samples -> status_t
sp_moving_average :: sp_sample_t*:source sp_sample_count_t:source_len sp_sample_t*:prev sp_sample_count_t:prev_len sp_sample_t*:next sp_sample_count_t:next_len sp_sample_count_t:radius sp_sample_count_t:start sp_sample_count_t:end sp_sample_t*:result_samples -> status_t
sp_port_close :: sp_port_t*:a -> status_t
sp_port_position :: sp_port_t*:port sp_sample_count_t*:result_position -> status_t
sp_port_read :: sp_port_t*:port sp_sample_count_t:sample_count sp_sample_t**:result_samples -> status_t
sp_port_set_position :: sp_port_t*:port int64_t:sample_offset -> status_t
sp_port_write :: sp_port_t*:port sp_sample_count_t:sample_count sp_sample_t**:channel_data -> status_t
sp_sinc :: sp_float_t:a -> sp_float_t
sp_sine :: sp_sample_count_t:len sp_float_t:sample_duration sp_float_t:freq sp_float_t:phase sp_float_t:amp sp_sample_t*:result_samples -> void
sp_sine_lq :: sp_sample_count_t:len sp_float_t:sample_duration sp_float_t:freq sp_float_t:phase sp_float_t:amp sp_sample_t*:result_samples -> void
sp_spectral_inversion_ir :: sp_sample_t*:a sp_sample_count_t:a_len -> void
sp_spectral_reversal_ir :: sp_sample_t*:a sp_sample_count_t:a_len -> void
sp_status_description :: status_t:a -> uint8_t*
sp_status_name :: status_t:a -> uint8_t*
sp_window_blackman :: sp_float_t:a sp_sample_count_t:width -> sp_float_t
sp_windowed_sinc :: sp_sample_t*:source sp_sample_count_t:source_len sp_sample_rate_t:sample_rate sp_float_t:freq sp_float_t:transition sp_windowed_sinc_state_t**:result_state sp_sample_t*:result_samples -> status_t
sp_windowed_sinc_ir :: sp_sample_rate_t:sample_rate sp_float_t:freq sp_float_t:transition sp_sample_count_t*:result_len sp_sample_t**:result_ir -> status_t
sp_windowed_sinc_ir_length :: sp_float_t:transition -> sp_sample_count_t
sp_windowed_sinc_state_create :: sp_sample_rate_t:sample_rate sp_float_t:freq sp_float_t:transition sp_windowed_sinc_state_t**:result_state -> status_t
sp_windowed_sinc_state_free :: sp_windowed_sinc_state_t*:state -> void
```

## macros
```
boolean
debug_log(format, ...)
debug_trace(n)
duration_to_sample_count(seconds, sample_rate)
f32
f64
sample_count_to_duration(sample_count, sample_rate)
sp_octets_to_samples(a)
sp_port_bit_closed
sp_port_bit_input
sp_port_bit_output
sp_port_bit_position
sp_port_type_alsa
sp_port_type_file
sp_sample_format_f32
sp_sample_format_f64
sp_sample_format_int16
sp_sample_format_int32
sp_sample_format_int8
sp_samples_to_octets(a)
sp_status_group_alsa
sp_status_group_libc
sp_status_group_sndfile
sp_status_group_sp
sp_status_group_sph
sp_windowed_sinc_cutoff(freq, sample_rate)
sph_status
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
sp_port_t: struct
  type: uint8_t
  flags: uint8_t
  sample_rate: sp_sample_rate_t
  channel_count: sp_channel_count_t
  data: void*
sp_windowed_sinc_state_t: struct
  carryover: sp_sample_t*
  carryover_len: sp_sample_count_t
  carryover_alloc_len: sp_sample_count_t
  freq: sp_float_t
  ir: sp_sample_t*
  ir_len: sp_sample_count_t
  sample_rate: sp_sample_rate_t
  transition: sp_float_t
status_t: struct
  id: status_id_t
  group: uint8_t*
```

## enum
```
sp_status_id_file_channel_mismatch sp_status_id_file_encoding sp_status_id_file_header
  sp_status_id_file_incompatible sp_status_id_file_incomplete sp_status_id_eof
  sp_status_id_input_type sp_status_id_memory sp_status_id_not_implemented
  sp_status_id_port_closed sp_status_id_port_position sp_status_id_port_type
  sp_status_id_undefined
```

# other language bindings
* scheme: [sph-sp-guile](https://github.com/sph-mn/sph-sp-guile)

# other interesting projects
* [soundpipe](https://github.com/PaulBatchelor/Soundpipe)
* [ciglet](https://github.com/Sleepwalking/ciglet)
