# sph-sp
scheme sound synthesis and processing toolset. c code, shared library, guile extension and scheme modules.
multi-dimensional sound generator

see also [sph.mn](http://sph.mn/c/view/nm)

# features
## scheme
* generator function that maps time to samples
* sequencer for custom functions with designated shared state and unlimited multi-stage cross modulation
* custom paths with gap, line, bezier curve and elliptical arc segments
* purely functional

## base library
these features do not depend on guile

* port object for alsa and file io with many supported file formats ([full list](http://www.mega-nerd.com/libsndfile/)). the file format to use is currently a compile-time configuration
* 64 bit float sample calculations by default, compile-time customisable sample type
* arbitrary number of channels
* arbitrary sample rate
* processing on non-interleaved sample arrays by default
* avoids floating point errors, prefers precision to performance

### transform - experimental
example code for learning - barely tested.
all processors are designed to process segments of continuous streams

* convolution
* moving average filter
* windowed sinc filter
* spectral inversion
* spectral reversal

# dependencies
* run-time
  * alsa
  * libc
  * libsndfile
  * guile >= 2.2
* quick build
  * gcc and shell for the provided compile script
* development build
  * [sph-sc](https://github.com/sph-mn/sph-sc)

# setup
* install dependencies
* then from inside the project directory, execute

```
./exe/compile-c
./exe/install
```

first argument to `exe/install` is the destination path prefix, for example `./exe/install /tmp`.
there is also exe/install-extended which can symlink files but needs [sph-lib](https://github.com/sph-mn/sph-lib)

installed files
* /usr/include/sph-sp.h
* /usr/lib/libguile-sph-sp.so
* /usr/share/guile/site/sph/*
* /usr/share/guile/site/test/sph/*

# scheme usage

```scheme
(import (sph sp) (sph sp generate))

(define sample-rate 16000)
(define channel-count 1)

;-- basic io
(define latency 4096)
(define input-port? #f)
(define dac (sp-alsa-open "default" input-port? channel-count sample-rate latency))
(sp-port-write dac (list (f64vector 1 2 3 4)))
(sp-port-close dac)

(define file (sp-file-open "tmp/sp-file.au" channel-count sample-rate))
(sp-port-write file (list (f64vector 1 2 3 4)))
(sp-port-close file)

;-- sp-generate
(import (sph sp generate))

(define time-start 0)
(define duration-seconds 2)

(let*
  ( (result-states
      (sp-generate sample-rate time-start duration-seconds
        ; segment-f - maps segments with samples previously set by sample-f
        (l (env time segment result . states)
          (pair (pair segment result) states))
        ; sample-f - sets samples in segments for time
        (l (env time . states)
          (pair (* 0.5 (sp-sine time)) states))
        ; all following arguments are passed to segment-f/sample-f
        (list)))
    (result-segments (reverse (first result-states))))
  (sp-segments->alsa result-segments))
```

sequencer usage example

```scheme
(import (sph sp) (sph sp generate) (sph sp generate sequencer))

(define (sound-a time state event duration custom)
  (and (< duration 1) (seq-output (sp-sine time 100) state)))

(define (sound-b time state event duration custom)
  (seq-output (* 0.5 (sp-sine time 200)) state (alist-q freq time)))

(define (sound-c time state event duration custom) (seq-output (* 0.5 (sp-sine time 400)) state))

(define (events-f time end seq-state)
  ; this returns a list of next event objects. events-f is called again after (seq-state-duration seq-state) seconds
  (list
    (seq-event a sound-a 0)
    (seq-event b sound-b 1)
    (seq-event c sound-c 1.5)
    (seq-event c sound-b (+ time 1))
    (seq-event d sound-b (+ time 20))))

(define (sample-f env time gen-result seq-state . custom)
  ; this maps time to sample value
  (seq time seq-state (l (data seq-state) (pairs data gen-result seq-state custom))))

(define (segment-f env time segment gen-result . custom)
  ; this maps time and a possibly empty sample array to a new sample array
  (pair (pair (list segment) gen-result) custom))

(define (run)
  (let*
    ( (seq-state (seq-state-new events-f))
      (result-states
        (sp-generate sample-rate 0 duration segment-f sample-f
          ; custom state values
          null seq-state))
      (result-segments (reverse (first result-states))))
    (sp-segments->alsa result-segments sample-rate "plughw:0" 4096)))
```

## modules
more documentation is currently to be found in the code files.

(sph sp)
```scheme
f32vector-sum :: f32vector [start end] -> number
f64vector-sum :: f64vector [start end] -> number
float-nearly-equal? :: a b c ->
sp-alsa-open :: device-name input? channel-count sample-rate latency -> sp-port
sp-duration->sample-count :: seconds sample-rate ->
sp-file-open :: path channel-count sample-rate -> sp-port
sp-pi
sp-plot-render :: file-path ->
sp-port-channel-count :: sp-port -> integer
sp-port-close :: sp-port -> boolean
sp-port-input? :: sp-port -> boolean
sp-port-position :: sp-port -> integer/boolean
sp-port-position? :: sp-port -> boolean
sp-port-read :: sp-port integer:sample-count -> (f32vector ...):channel-data
sp-port-sample-rate :: sp-port -> integer/boolean
sp-port-set-position :: sp-port integer:sample-offset -> boolean
sp-port-write :: sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean
sp-port? :: sp-port -> boolean
sp-sample-count->duration :: sample-count sample-rate ->
sp-segments->alsa :: ((vector ...) ...) ->
sp-segments->file :: ((vector ...) ...) string ->
sp-segments->plot :: ((vector ...) ...) string ->
sp-segments->plot-render :: a path ->
sp-sine! :: data len sample-duration freq phase amp -> unspecified
sp-sine-lq! :: data len sample-duration freq phase amp -> unspecified
```

(sph sp generate)
```
sp-clip :: a ->
sp-fold-integers :: start end f states ... ->
sp-generate :: integer number number procedure procedure any ... -> (any ...):states
sp-noise :: integer [{random-state -> real} random-state] -> f64vector
sp-path :: number path-state [procedure -> result]
sp-path-new :: sample-rate (symbol:segment-type param ...) ...
sp-path-new-p :: number ((symbol:type any:parameter ...) ...) -> path-state
sp-segment :: integer procedure -> (vector . states)
sp-sine :: time freq -> number
```

(sph sp generate sequencer)
```
seq :: integer list -> integer/vector:sample-data list:state
seq-default-mixer :: output ->
seq-event :: name f optional ...
seq-event-custom :: a ->
seq-event-f :: a ->
seq-event-groups :: a ->
seq-event-list->events :: a ->
seq-event-name :: a ->
seq-event-new :: procedure #:key integer (symbol ...) any -> vector
seq-event-start :: a ->
seq-event-update :: a #:f #:start #:name #:groups #:custom ->
seq-events-merge :: events:target events:source -> events
seq-index-data :: a ->
seq-index-end :: a ->
seq-index-events :: a ->
seq-index-f :: a ->
seq-index-f-new :: number seq-state -> procedure
seq-index-i-f :: a ->
seq-index-i-f-new :: vector number number -> procedure
seq-index-i-next :: index time ->
seq-index-new :: data end events f i-f start ->
seq-index-next :: index time state ->
seq-index-start :: a ->
seq-index-update :: a #:data #:end #:events #:f #:i-f #:start ->
seq-output :: symbol integer/vector/any seq-state list list:alist -> list
seq-output-new :: name data custom event ->
seq-state-add-events :: seq-state seq-event-list/seq-events ... -> state
seq-state-custom :: a ->
seq-state-events-custom :: a ->
seq-state-index :: a ->
seq-state-index-i :: a ->
seq-state-input :: a ->
seq-state-new :: procedure [#:event-f-list list #:custom alist] -> seq-state
seq-state-options :: a ->
seq-state-output :: a ->
seq-state-update :: a #:custom #:events-f #:events-custom #:index #:index-i #:input #:mixer #:options #:output -> state
```

# c usage
```
#include <sph-sp.h>
```

## error handling
routines return an object of type `status_t`, which is a `struct {.id, .group}`.
status.id = 0 is success, but it is easier to use status_* and sp_status_* bindings to check for that

## types
```c
status_i_t
sp_port_t struct
  b32 sample_rate;
  b32 channel_count;
  boolean closed_p;
  b8 flags;
  b8 type;
  size_t position;
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
  sp_float_t freq;
  sp_float_t transition;
status_t struct
  status_i_t id;
  b8 group;
```

## enum
```c
sp_status_id_undefined, sp_status_id_input_type, sp_status_id_not_implemented,
sp_status_id_memory, sp_status_id_file_incompatible, sp_status_id_file_encoding,
sp_status_id_file_header, sp_status_id_port_closed, sp_status_id_port_position,
sp_status_id_file_channel_mismatch, sp_status_id_file_incomplete, sp_status_id_port_type,
sp_status_group_sp, sp_status_group_libc, sp_status_group_alsa
```

## routines
```c
b0 sp_convolve(sp_sample_t* result, sp_sample_t* a, size_t a_len, sp_sample_t* b, size_t b_len, sp_sample_t* carryover, size_t carryover_len)
b0 sp_convolve_one(sp_sample_t* result, sp_sample_t* a, size_t a_len, sp_sample_t* b, size_t b_len)
b0 sp_sine(sp_sample_t* data, b32 len, sp_float_t sample_duration, sp_float_t freq, sp_float_t phase, sp_float_t amp)
b0 sp_sine_lq(sp_sample_t* data, b32 len, sp_float_t sample_duration, sp_float_t freq, sp_float_t phase, sp_float_t amp)
b0 sp_spectral_inversion_ir(sp_sample_t* a, size_t a_len)
b0 sp_spectral_reversal_ir(sp_sample_t* a, size_t a_len)
b0 sp_windowed_sinc_ir(sp_sample_t** result, size_t* result_len, b32 sample_rate, sp_float_t freq, sp_float_t transition)
b0 sp_windowed_sinc_state_destroy(sp_windowed_sinc_state_t* state)
b8 sp_windowed_sinc_state_create(b32 sample_rate, sp_float_t freq, sp_float_t transition, sp_windowed_sinc_state_t** state)
b8* sp_status_description(status_t a)
b8* sp_status_name(status_t a)
size_t sp_windowed_sinc_ir_length(sp_float_t transition)
sp_float_t sp_sin_lq(sp_float_t a)
sp_float_t sp_sinc(sp_float_t a)
sp_float_t sp_window_blackman(sp_float_t a, size_t width)
sp_sample_t** sp_alloc_channel_array(b32 channel_count, b32 sample_count)
status_i_t sp_moving_average(sp_sample_t* result, sp_sample_t* source, b32 source_len, sp_sample_t* prev, b32 prev_len, sp_sample_t* next, b32 next_len, b32 start, b32 end, b32 distance)
status_i_t sp_windowed_sinc(sp_sample_t* result, sp_sample_t* source, size_t source_len, b32 sample_rate, sp_float_t freq, sp_float_t transition, sp_windowed_sinc_state_t** state)
status_t sp_alsa_open(sp_port_t* result, b8* device_name, boolean input_p, b32 channel_count, b32 sample_rate, b32_s latency)
status_t sp_fft(sp_sample_t* result, b32 result_len, sp_sample_t* source, b32 source_len)
status_t sp_file_open(sp_port_t* result, b8* path, b32 channel_count, b32 sample_rate)
status_t sp_ifft(sp_sample_t* result, b32 result_len, sp_sample_t* source, b32 source_len)
status_t sp_port_close(sp_port_t* a)
status_t sp_port_position(size_t* result, sp_port_t* port)
status_t sp_port_read(sp_sample_t** result, sp_port_t* port, b32 sample_count)
status_t sp_port_set_position(sp_port_t* port, size_t sample_index)
status_t sp_port_write(sp_port_t* port, size_t sample_count, sp_sample_t** channel_data)
```

## macros
```c
duration_to_sample_count(seconds, sample_rate)
sample_count_to_duration(sample_count, sample_rate)
sp_alloc_define(id, type, octet_count)
sp_alloc_define_samples(id, sample_count)
sp_alloc_define_samples_zero(id, sample_count)
sp_alloc_define_zero(id, type, octet_count)
sp_alloc_require(a)
sp_alloc_set(a, octet_count)
sp_alloc_set_samples(a, sample_count)
sp_alloc_set_samples_zero(a, sample_count)
sp_alloc_set_zero(a, octet_count)
sp_alsa_status_require_x(expression)
sp_default_alsa_enable_soft_resample
sp_default_alsa_latency
sp_default_channel_count
sp_default_sample_rate
sp_float_t
sp_octets_to_samples(a)
sp_port_bit_input
sp_port_bit_output
sp_port_bit_position
sp_port_type_alsa
sp_port_type_file
sp_sample_type
sp_sample_type_f32
sp_sample_type_f64
sp_samples_to_octets(a)
sp_status_init
sp_status_require_alloc(a)
sp_system_status_require_id(id)
sp_system_status_require_x(expression)
sp_windowed_sinc_cutoff(freq, sample_rate)
status_failure_p
status_goto
status_group_undefined
status_i_require_x(expression)
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