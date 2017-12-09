# sph-sp-guile

library for basic sound processing with guile scheme.
multi-dimensional sound generator

# features
* [sph-sp](https://github.com/sph-mn/sph-sp) features
* mapping function that for a duration maps time to sample or sample arrays
* sequencer for event functions with shared state for cross modulation plus automatic event indexing for improved performance
* path creation with custom line, bezier curve, elliptical arc and gap segments
* sine and noise generator

# dependencies
* run-time
  * guile >= 2.2
  * [sph-sp](https://github.com/sph-mn/sph-sp)
* quick build
  * gcc and shell for the provided compile script
* development build
  * [sph-sc](https://github.com/sph-mn/sph-sc)

# installation
```
./exe/compile-c
./exe/install
```

# usage
```
(import (sph sp))

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

# exports
```
float-nearly-equal? :: a b c ->
sp-alsa-open :: device-name input? channel-count sample-rate latency -> sp-port
sp-duration->sample-count :: seconds sample-rate ->
sp-file-open :: path channel-count sample-rate -> sp-port
sp-pi
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
sp-segments->alsa :: (vector ...) string ->
sp-segments->file :: (vector ...) string ->
sp-segments->plot :: (vector ...) string ->
sp-sine! :: data len sample-duration freq phase amp -> unspecified
sp-sine-lq! :: data len sample-duration freq phase amp -> unspecifiedsp-fold-integers :: start end f states ... ->
sp-generate :: sample-rate start duration segment-f sample-f states ... ->
sp-line :: time rise duration ->
sp-noise :: integer [{random-state -> real} random-state] -> f64vector
sp-segment :: size f states ... ->
sp-sine :: time freq ->
```
