(pre-define-if-not-defined __USE_POSIX199309)
(pre-include "byteswap.h" "inttypes.h" "string.h")
(sc-comment "configuration")

(pre-define-if-not-defined
  sp-channel-limit 2
  sp-channels-t uint8-t
  sp-file-format (bit-or SF-FORMAT-WAV SF-FORMAT-FLOAT)
  sp-float-t double
  spline-path-time-t sp-time-t
  spline-path-value-t sp-sample-t
  sp-sample-rate-t uint32-t
  sp-samples-sum f64-sum
  sp-sample-t double
  sp-sf-read sf-readf-double
  sp-sf-write sf-writef-double
  sp-synth-count-t uint16-t
  sp-synth-partial-limit 64
  sp-synth-sine sp-sine-96
  sp-time-t uint64-t)

(pre-include "sph/status.c" "sph/spline-path.h"
  "sph/types.c" "sph/random.c" "sph/i-array.c" "sph/memreg-heap.c" "sph/float.c")

(sc-comment "main")

(pre-define
  boolean uint8-t
  f64 double
  sp-file-bit-input 1
  sp-file-bit-output 2
  sp-file-bit-position 4
  sp-file-mode-read 1
  sp-file-mode-write 2
  sp-file-mode-read-write 3
  sp-s-group-libc "libc"
  sp-s-group-sndfile "sndfile"
  sp-s-group-sp "sp"
  sp-s-group-sph "sph"
  sp-random-state-t sph-random-state-t
  sp-random-state-new sph-random-state-new
  sp-s-id-undefined 1
  sp-s-id-file-channel-mismatch 2
  sp-s-id-file-encoding 3
  sp-s-id-file-header 4
  sp-s-id-file-incompatible 5
  sp-s-id-file-incomplete 6
  sp-s-id-eof 7
  sp-s-id-input-type 8
  sp-s-id-memory 9
  sp-s-id-invalid-argument 10
  sp-s-id-not-implemented 11
  sp-s-id-file-closed 11
  sp-s-id-file-position 12
  sp-s-id-file-type 13
  (sp-octets->samples a) (begin "sample count to bit octets count" (/ a (sizeof sp-sample-t)))
  (sp-samples->octets a) (* a (sizeof sp-sample-t))
  (sp-cheap-round-positive a) (convert-type (+ 0.5 a) sp-time-t)
  (sp-cheap-floor-positive a) (convert-type a sp-time-t)
  (sp-cheap-ceiling-positive a) (+ (convert-type a sp-time-t) (< (convert-type a sp-time-t) a))
  (sp-block-zero a) (set a.channels 0)
  (sp-samples-zero a size) (memset a 0 (* size (sizeof sp-sample-t))))

(declare
  sp-block-t
  (type
    (struct
      (channels sp-channels-t)
      (size sp-time-t)
      (samples (array sp-sample-t* sp-channel-limit))))
  sp-file-t
  (type
    (struct
      (flags uint8-t)
      (sample-rate sp-sample-rate-t)
      (channel-count sp-channels-t)
      (data void*)))
  sp-cpu-count uint32-t
  sp-default-random-state sp-random-state-t
  (sp-file-read file sample-count result-block result-sample-count)
  (status-t sp-file-t* sp-time-t sp-sample-t** sp-time-t*)
  (sp-file-write file block sample-count result-sample-count)
  (status-t sp-file-t* sp-sample-t** sp-time-t sp-time-t*)
  (sp-file-position file result-position) (status-t sp-file-t* sp-time-t*)
  (sp-file-position-set file sample-offset) (status-t sp-file-t* sp-time-t)
  (sp-file-open path mode channel-count sample-rate result-file)
  (status-t uint8-t* int sp-channels-t sp-sample-rate-t sp-file-t*)
  (sp-file-close a) (status-t sp-file-t*)
  (sp-block->file block path rate) (status-t sp-block-t uint8-t* sp-time-t)
  (sp-block-new channel-count sample-count out-block) (status-t sp-channels-t sp-time-t sp-block-t*)
  (sp-status-description a) (uint8-t* status-t)
  (sp-status-name a) (uint8-t* status-t)
  (sp-sin-lq a) (sp-sample-t sp-float-t)
  (sp-sinc a) (sp-float-t sp-float-t)
  (sp-window-blackman a width) (sp-float-t sp-float-t sp-time-t)
  (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-time-t)
  (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-time-t)
  (sp-fft input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  (sp-ffti input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-convolve a a-len b b-len result-carryover-len result-carryover result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-time-t sp-sample-t* sp-sample-t*)
  (sp-block-free a) (void sp-block-t)
  (sp-block-with-offset a offset) (sp-block-t sp-block-t sp-time-t)
  (sp-null-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (sp-passthrough-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (sp-initialise cpu-count) (status-t uint16-t)
  (sp-random-samples state size out) (void sp-random-state-t* sp-time-t sp-sample-t*)
  (sp-random-times state size out) (void sp-random-state-t* sp-time-t sp-time-t*)
  (sp-samples-new size out) (status-t sp-time-t sp-sample-t**)
  (sp-times-new size out) (status-t sp-time-t sp-time-t**)
  (sp-samples->time in in-size out) (void sp-sample-t* sp-time-t sp-time-t*))

(sc-comment "filter")

(pre-define
  sp-filter-state-t sp-convolution-filter-state-t
  sp-filter-state-free sp-convolution-filter-state-free
  sp-cheap-filter-passes-limit 8
  (sp-cheap-filter-lp ...) (sp-cheap-filter sp-state-variable-filter-lp __VA_ARGS__)
  (sp-cheap-filter-hp ...) (sp-cheap-filter sp-state-variable-filter-hp __VA_ARGS__)
  (sp-cheap-filter-bp ...) (sp-cheap-filter sp-state-variable-filter-bp __VA_ARGS__)
  (sp-cheap-filter-br ...) (sp-cheap-filter sp-state-variable-filter-br __VA_ARGS__))

(declare
  sp-convolution-filter-ir-f-t (type (function-pointer status-t void* sp-sample-t** sp-time-t*))
  sp-convolution-filter-state-t
  (type
    (struct
      (carryover sp-sample-t*)
      (carryover-len sp-time-t)
      (carryover-alloc-len sp-time-t)
      (ir sp-sample-t*)
      (ir-f sp-convolution-filter-ir-f-t)
      (ir-f-arguments void*)
      (ir-f-arguments-len uint8-t)
      (ir-len sp-time-t)))
  sp-cheap-filter-state-t
  (type
    (struct
      (in-temp sp-sample-t*)
      (out-temp sp-sample-t*)
      (svf-state (array sp-sample-t ((* 2 sp-cheap-filter-passes-limit))))))
  sp-state-variable-filter-t
  (type
    (function-pointer void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*))
  (sp-moving-average in in-end in-window in-window-end prev prev-end next next-end radius out)
  (status-t sp-sample-t* sp-sample-t*
    sp-sample-t* sp-sample-t* sp-sample-t*
    sp-sample-t* sp-sample-t* sp-sample-t* sp-time-t sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir-length transition) (sp-time-t sp-float-t)
  (sp-windowed-sinc-ir cutoff transition result-len result-ir)
  (status-t sp-float-t sp-float-t sp-time-t* sp-sample-t**)
  (sp-convolution-filter-state-free state) (void sp-convolution-filter-state-t*)
  (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)
  (status-t sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-lp-hp-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-bp-br-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-lp-hp in in-len cutoff transition is-high-pass out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-bp-br in in-len
    cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-time-t*)
  (sp-state-variable-filter-lp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-hp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-bp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-br out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-peak out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-state-variable-filter-all out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
  (sp-cheap-filter type in in-size cutoff passes q-factor unity-gain state out)
  (void sp-state-variable-filter-t sp-sample-t*
    sp-time-t sp-float-t sp-time-t sp-float-t uint8-t sp-cheap-filter-state-t* sp-sample-t*)
  (sp-cheap-filter-state-free a) (void sp-cheap-filter-state-t*)
  (sp-cheap-filter-state-new max-size max-passes out-state)
  (status-t sp-time-t sp-time-t sp-cheap-filter-state-t*)
  (sp-filter in in-size cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-filter-state-t** sp-sample-t*))

(sc-comment "plot")

(declare
  (sp-plot-samples a a-size) (void sp-sample-t* sp-time-t)
  (sp-plot-times a a-size) (void sp-time-t* sp-time-t)
  (sp-plot-samples->file a a-size path) (void sp-sample-t* sp-time-t uint8-t*)
  (sp-plot-times->file a a-size path) (void sp-time-t* sp-time-t uint8-t*)
  (sp-plot-samples-file path use-steps) (void uint8-t* uint8-t)
  (sp-plot-spectrum->file a a-size path) (void sp-sample-t* sp-time-t uint8-t*)
  (sp-plot-spectrum-file path) (void uint8-t*)
  (sp-plot-spectrum a a-size) (void sp-sample-t* sp-time-t))

(sc-comment "synthesiser")
(pre-define (sp-sine-96 t) (begin "t must be between 0 and 95999" (array-get sp-sine-96-table t)))

(declare
  sp-synth-partial-t
  (type
    (struct
      (start sp-time-t)
      (end sp-time-t)
      (modifies sp-synth-count-t)
      (amp (array sp-sample-t* sp-channel-limit))
      (wvl (array sp-time-t* sp-channel-limit))
      (phs (array sp-time-t sp-channel-limit))))
  sp-sine-96-table sp-sample-t*
  (sp-sine-table-new out size) (status-t sp-sample-t** sp-time-t)
  (sp-phase-96 current change) (sp-time-t sp-time-t sp-time-t)
  (sp-phase-96-float current change) (sp-time-t sp-time-t double)
  (sp-synth out start duration config-len config phases)
  (status-t sp-block-t sp-time-t sp-time-t sp-synth-count-t sp-synth-partial-t* sp-time-t*)
  (sp-synth-state-new channel-count config-len config out-state)
  (status-t sp-time-t sp-synth-count-t sp-synth-partial-t* sp-time-t**)
  (sp-synth-partial-1 start end modifies amp wvl phs)
  (sp-synth-partial-t sp-time-t sp-time-t sp-synth-count-t sp-sample-t* sp-time-t* sp-time-t)
  (sp-synth-partial-2 start end modifies amp1 amp2 wvl1 wvl2 phs1 phs2)
  (sp-synth-partial-t sp-time-t sp-time-t
    sp-synth-count-t sp-sample-t* sp-sample-t* sp-time-t* sp-time-t* sp-time-t sp-time-t)
  (sp-square-96 t) (sp-sample-t sp-time-t)
  (sp-triangle t a b) (sp-sample-t sp-time-t sp-time-t sp-time-t)
  (sp-triangle-96 t) (sp-sample-t sp-time-t))

(sc-comment "sequencer")

(pre-define
  (sp-event-duration a) (- a.end a.start)
  (sp-event-duration-set a duration) (set a.end (+ a.start duration))
  (sp-event-move a start) (set a.end (+ start (- a.end a.start)) a.start start)
  (sp-cheap-noise-event-lp start end amp ...)
  (sp-cheap-noise-event start end amp sp-state-variable-filter-lp __VA_ARGS__)
  (sp-cheap-noise-event-hp start end amp ...)
  (sp-cheap-noise-event start end amp sp-state-variable-filter-hp __VA_ARGS__)
  (sp-cheap-noise-event-bp start end amp ...)
  (sp-cheap-noise-event start end amp sp-state-variable-filter-bp __VA_ARGS__)
  (sp-cheap-noise-event-br start end amp ...)
  (sp-cheap-noise-event start end amp sp-state-variable-filter-br __VA_ARGS__)
  sp-group-size-t uint32-t
  (sp-group-events a) (: (convert-type a.state sp-group-event-state-t*) events)
  (sp-group-memory a) (: (convert-type a.state sp-group-event-state-t*) memory)
  (sp-group-memory-add a pointer)
  (if (i-array-space (sp-group-memory a)) (i-array-add (sp-group-memory a) pointer))
  (sp-group-add a event)
  (if (i-array-space (sp-group-events a))
    (begin (i-array-add (sp-group-events a) event) (if (< a.end event.end) (set a.end event.end))))
  (sp-group-prepare a)
  (sp-seq-events-prepare (struct-get (sp-group-events a) start)
    (i-array-length (sp-group-events a)))
  (sp-event-declare variable) (begin (declare variable sp-event-t) (set variable.state 0))
  sp-group-declare sp-event-declare
  (sp-group-free a) (if a.state (a.free &a)))

(declare
  sp-event-t struct
  sp-event-t
  (type
    (struct
      sp-event-t
      (state void*)
      (start sp-time-t)
      (end sp-time-t)
      (f (function-pointer void sp-time-t sp-time-t sp-block-t (struct sp-event-t*)))
      (free (function-pointer void (struct sp-event-t*)))))
  sp-event-f-t (type (function-pointer void sp-time-t sp-time-t sp-block-t sp-event-t*))
  sp-synth-event-state-t
  (type
    (struct
      (config-len sp-synth-count-t)
      (config (array sp-synth-partial-t sp-synth-partial-limit))
      (state sp-time-t*)))
  (sp-seq-events-prepare data size) (void sp-event-t* sp-time-t)
  (sp-seq start end out events size) (void sp-time-t sp-time-t sp-block-t sp-event-t* sp-time-t)
  (sp-seq-parallel start end out events size)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t* sp-time-t)
  (sp-synth-event start end channel-count config-len config out-event)
  (status-t sp-time-t sp-time-t sp-channels-t sp-time-t sp-synth-partial-t* sp-event-t*)
  (sp-noise-event start end amp cut-l cut-h trn-l trn-h is-reject resolution random-state out-event)
  (status-t sp-time-t sp-time-t
    sp-sample-t** sp-sample-t* sp-sample-t*
    sp-sample-t* sp-sample-t* uint8-t sp-time-t sp-random-state-t sp-event-t*)
  (sp-events-free events size) (void sp-event-t* sp-time-t)
  (sp-cheap-noise-event start end amp type cut passes q-factor resolution random-state out-event)
  (status-t sp-time-t sp-time-t
    sp-sample-t** sp-state-variable-filter-t sp-sample-t*
    sp-time-t sp-sample-t sp-time-t sp-random-state-t sp-event-t*))

(i-array-declare-type sp-events sp-event-t)
(declare sp-group-event-state-t (type (struct (events sp-events-t) (memory memreg-register-t))))

(declare
  (sp-group-new start event-size memory-size out)
  (status-t sp-time-t sp-group-size-t sp-group-size-t sp-event-t*)
  (sp-group-append a event) (void sp-event-t* sp-event-t)
  (sp-group-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-event-free a) (void sp-event-t*))

(sc-comment "path")

(pre-define
  path-move spline-path-move
  sp-path-line spline-path-line
  sp-path-bezier spline-path-bezier
  sp-path-constant spline-path-constant
  sp-path-path spline-path-path)

(sc-comment "main 2")

(pre-define
  (declare-render-config name)
  (define name sp-render-config-t (struct-literal (channels 2) (rate 96000) (block-size 96000)))
  (rt n d)
  (begin
    "returns n/d fractions of the sample rate. for example, 1/2 is half the sample rate.
     this macro references a local variable named rate which must exists and contain the current sample rate"
    (convert-type (* (/ rate d) n) sp-time-t)))

(declare
  sp-render-config-t (type (struct (rate sp-time-t) (block-size sp-time-t) (channels sp-channels-t)))
  (sp-render-file event start duration config path)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t uint8-t*))

(declare
  (spline-path-new-get-2 out duration s1 s2)
  (int sp-sample-t* sp-time-t spline-path-segment-t spline-path-segment-t)
  (spline-path-new-get-3 out duration s1 s2 s3)
  (int sp-sample-t* sp-time-t spline-path-segment-t spline-path-segment-t spline-path-segment-t)
  (spline-path-new-get-4 out duration s1 s2 s3 s4)
  (int sp-sample-t* sp-time-t
    spline-path-segment-t spline-path-segment-t spline-path-segment-t spline-path-segment-t)
  (sp-path-samples segment-count segments size out)
  (status-t spline-path-segment-count-t spline-path-segment-t* spline-path-time-t sp-sample-t**)
  (sp-path-times segment-count segments size out)
  (status-t spline-path-segment-count-t spline-path-segment-t* spline-path-time-t sp-time-t**)
  (sp-path-samples-2 out size s1 s2)
  (status-t sp-sample-t** spline-path-time-t spline-path-segment-t spline-path-segment-t)
  (sp-path-samples-3 out size s1 s2 s3)
  (status-t sp-sample-t** spline-path-time-t
    spline-path-segment-t spline-path-segment-t spline-path-segment-t)
  (sp-path-samples-4 out size s1 s2 s3 s4)
  (status-t sp-sample-t** spline-path-time-t
    spline-path-segment-t spline-path-segment-t spline-path-segment-t spline-path-segment-t)
  (sp-path-times-2 out size s1 s2)
  (status-t sp-time-t** spline-path-time-t spline-path-segment-t spline-path-segment-t)
  (sp-path-times-3 out size s1 s2 s3)
  (status-t sp-time-t** spline-path-time-t
    spline-path-segment-t spline-path-segment-t spline-path-segment-t)
  (sp-path-times-4 out size s1 s2 s3 s4)
  (status-t sp-time-t** spline-path-time-t
    spline-path-segment-t spline-path-segment-t spline-path-segment-t spline-path-segment-t))