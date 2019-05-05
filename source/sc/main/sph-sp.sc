(pre-include "byteswap.h" "math.h" "inttypes.h" "string.h")

(pre-if-not-defined
  sp-config-is-set
  (pre-define
    sp-channel-count-t uint8-t
    sp-file-format (bit-or SF-FORMAT-WAV SF-FORMAT-FLOAT)
    sp-float-t double
    sp-count-t uint32-t
    sp-sample-count-max UINT32_MAX
    sp-sample-rate-t uint32-t
    sp-sample-sum f64-sum
    sp-sample-t double
    sp-sf-read sf-readf-double
    sp-sf-write sf-writef-double
    sp-synth-count-t uint16-t
    sp-synth-sine sp-sine-96
    sp-synth-channel-limit 16
    sp-synth-partial-limit 64
    sp-config-is-set #t)
  ; f32
  #;(pre-define
    sp-channel-count-t uint8-t
    sp-file-format (bit-or SF-FORMAT-WAV SF-FORMAT-FLOAT)
    sp-float-t float
    sp-count-t size-t
    sp-sample-rate-t uint32-t
    sp-sample-sum f32-sum
    sp-sample-t float
    sp-sf-read sf-readf-float
    sp-sf-write sf-writef-float
    sp-config-is-set #t))

(pre-define
  spline-path-time-t sp-count-t
  spline-path-value-t sp-sample-t
  boolean uint8-t
  sp-file-bit-input 1
  sp-file-bit-output 2
  sp-file-bit-position 4
  sp-file-bit-closed 8
  sp-file-mode-read 1
  sp-file-mode-write 2
  sp-file-mode-read-write 3
  sp-status-group-libc "libc"
  sp-status-group-sndfile "sndfile"
  sp-status-group-sp "sp"
  sp-status-group-sph "sph"
  (sp-octets->samples a)
  (begin
    "sample count to bit octets count"
    (/ a (sizeof sp-sample-t)))
  (sp-samples->octets a) (* a (sizeof sp-sample-t))
  (sp-sine-96 t)
  (begin
    "t must be between 0 and 95999"
    (array-get sp-sine-96-table t))
  (sp-cheap-round-positive a) (convert-type (+ 0.5 a) sp-count-t)
  (sp-cheap-floor-positive a) (convert-type a sp-count-t)
  (sp-cheap-ceiling-positive a) (+ (convert-type a sp-count-t) (< (convert-type a sp-count-t) a))
  (sp-block-set-null a) (set a.channels 0))

(sc-include "../foreign/sph" "../foreign/sph/status" "../foreign/sph/spline-path")

(enum
  (sp-status-id-file-channel-mismatch
    sp-status-id-file-encoding
    sp-status-id-file-header
    sp-status-id-file-incompatible
    sp-status-id-file-incomplete
    sp-status-id-eof
    sp-status-id-input-type
    sp-status-id-memory
    sp-status-id-invalid-argument
    sp-status-id-not-implemented
    sp-status-id-file-closed sp-status-id-file-position sp-status-id-file-type sp-status-id-undefined))

(pre-define sp-channel-limit 2)

(declare
  sp-block-t
  (type
    (struct
      (channels sp-channel-count-t)
      (size sp-count-t)
      (samples (array sp-sample-t* sp-channel-limit))))
  sp-file-t
  (type
    (struct
      (flags uint8-t)
      (sample-rate sp-sample-rate-t)
      (channel-count sp-channel-count-t)
      (data void*)))
  sp-convolution-filter-ir-f-t (type (function-pointer status-t void* sp-sample-t** sp-count-t*))
  sp-convolution-filter-state-t
  (type
    (struct
      (carryover sp-sample-t*)
      (carryover-len sp-count-t)
      (carryover-alloc-len sp-count-t)
      (ir sp-sample-t*)
      (ir-f sp-convolution-filter-ir-f-t)
      (ir-f-arguments void*)
      (ir-f-arguments-len uint8-t)
      (ir-len sp-count-t)))
  sp-synth-partial-t
  (type
    (struct
      (start sp-count-t)
      (end sp-count-t)
      (modifies sp-synth-count-t)
      (amp (array sp-sample-t* sp-synth-channel-limit))
      (wvl (array sp-count-t* sp-synth-channel-limit))
      (phs (array sp-count-t sp-synth-channel-limit))))
  (sp-file-read file sample-count result-block result-sample-count)
  (status-t sp-file-t* sp-count-t sp-sample-t** sp-count-t*)
  (sp-file-write file block sample-count result-sample-count)
  (status-t sp-file-t* sp-sample-t** sp-count-t sp-count-t*) (sp-file-position file result-position)
  (status-t sp-file-t* sp-count-t*) (sp-file-position-set file sample-offset)
  (status-t sp-file-t* sp-count-t) (sp-file-open path mode channel-count sample-rate result-file)
  (status-t uint8-t* int sp-channel-count-t sp-sample-rate-t sp-file-t*) (sp-file-close a)
  (status-t sp-file-t*) (sp-block-new channel-count sample-count out-block)
  (status-t sp-channel-count-t sp-count-t sp-block-t*) (sp-status-description a)
  (uint8-t* status-t) (sp-status-name a)
  (uint8-t* status-t) (sp-sin-lq a)
  (sp-sample-t sp-float-t) (sp-sinc a)
  (sp-float-t sp-float-t) (sp-windowed-sinc-lp-hp-ir-length transition)
  (sp-count-t sp-float-t) (sp-windowed-sinc-ir cutoff transition result-len result-ir)
  (status-t sp-float-t sp-float-t sp-count-t* sp-sample-t**) (sp-convolution-filter-state-free state)
  (void sp-convolution-filter-state-t*)
  (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t
    sp-sample-t*
    sp-count-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-count-t*)
  (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)
  (status-t sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-sample-t** sp-count-t*)
  (sp-windowed-sinc-lp-hp-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-count-t*)
  (sp-windowed-sinc-bp-br-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-count-t*)
  (sp-windowed-sinc-lp-hp in in-len cutoff transition is-high-pass out-state out-samples)
  (status-t
    sp-sample-t*
    sp-count-t sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-bp-br
    in in-len cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t
    sp-sample-t*
    sp-count-t
    sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  (sp-window-blackman a width) (sp-float-t sp-float-t sp-count-t)
  (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-count-t)
  (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-count-t)
  (sp-fft input-len input/output-real input/output-imag) (status-id-t sp-count-t double* double*)
  (sp-ffti input-len input/output-real input/output-imag) (status-id-t sp-count-t double* double*)
  (sp-moving-average in in-end in-window in-window-end prev prev-end next next-end radius out)
  (status-t
    sp-sample-t*
    sp-sample-t*
    sp-sample-t*
    sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-count-t sp-sample-t*)
  (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-count-t sp-sample-t* sp-count-t sp-sample-t*)
  (sp-convolve a a-len b b-len result-carryover-len result-carryover result-samples)
  (void sp-sample-t* sp-count-t sp-sample-t* sp-count-t sp-count-t sp-sample-t* sp-sample-t*)
  (sp-block-free a) (void sp-block-t)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-count-t*) (sp-null-ir out-ir out-len)
  (status-t sp-sample-t** sp-count-t*) (sp-passthrough-ir out-ir out-len)
  (status-t sp-sample-t** sp-count-t*) (sp-sine-table-new out size)
  (status-t sp-sample-t** sp-count-t) sp-sine-96-table
  sp-sample-t* (sp-initialise cpu-count)
  (status-t uint16-t) (sp-phase-96 current change)
  (sp-count-t sp-count-t sp-count-t) (sp-phase-96-float current change)
  (sp-count-t sp-count-t double) (sp-synth out start duration config-len config phases)
  (status-t sp-block-t sp-count-t sp-count-t sp-synth-count-t sp-synth-partial-t* sp-count-t*)
  (sp-synth-state-new channel-count config-len config out-state)
  (status-t sp-count-t sp-synth-count-t sp-synth-partial-t* sp-count-t**)
  (sp-state-variable-filter-lp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-count-t sp-sample-t*)
  (sp-state-variable-filter-hp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-count-t sp-sample-t*)
  (sp-state-variable-filter-bp out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-count-t sp-sample-t*)
  (sp-state-variable-filter-br out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-count-t sp-sample-t*)
  (sp-state-variable-filter-peak out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-count-t sp-sample-t*)
  (sp-state-variable-filter-all out in in-count cutoff q-factor state)
  (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-count-t sp-sample-t*))

(declare
  sp-event-t struct
  sp-event-t
  (type
    (struct
      sp-event-t
      (state void*)
      (start sp-count-t)
      (end sp-count-t)
      (f
        (function-pointer void sp-count-t sp-count-t sp-block-t
          (struct
            sp-event-t*)))))
  sp-event-f-t (type (function-pointer void sp-count-t sp-count-t sp-block-t sp-event-t*))
  sp-synth-event-state-t
  (type
    (struct
      (config-len sp-synth-count-t)
      (config (array sp-synth-partial-t sp-synth-partial-limit))
      (state sp-count-t*))))

(declare
  (sp-seq-events-prepare a a-size) (void sp-event-t* sp-count-t)
  (sp-seq start end out out-start events events-size)
  (void sp-count-t sp-count-t sp-block-t sp-count-t sp-event-t* sp-count-t)
  (sp-seq-parallel start end out out-start events events-size)
  (status-t sp-count-t sp-count-t sp-block-t sp-count-t sp-event-t* sp-count-t)
  (sp-synth-event start end channel-count config-len config out-event)
  (status-t sp-count-t sp-count-t sp-count-t sp-count-t sp-synth-partial-t* sp-event-t*)
  (sp-plot-samples a a-size) (void sp-sample-t* sp-count-t)
  (sp-plot-counts a a-size) (void sp-count-t* sp-count-t)
  (sp-plot-samples->file a a-size path) (void sp-sample-t* sp-count-t uint8-t*)
  (sp-plot-counts->file a a-size path) (void sp-count-t* sp-count-t uint8-t*)
  (sp-plot-samples-file path use-steps) (void uint8-t* uint8-t)
  (sp-plot-spectrum->file a a-size path) (void sp-sample-t* sp-count-t uint8-t*)
  (sp-plot-spectrum-file path) (void uint8-t*)
  (sp-plot-spectrum a a-size) (void sp-sample-t* sp-count-t)
  (sp-square-96 t) (sp-sample-t sp-count-t)
  (sp-triangle t a b) (sp-sample-t sp-count-t sp-count-t sp-count-t)
  (sp-triangle-96 t) (sp-sample-t sp-count-t)
  sp-random-state-t
  (type
    (struct
      (data (array uint64-t 4))))
  (sp-random state size out) (sp-random-state-t sp-random-state-t sp-count-t sp-sample-t*)
  (sp-random-state-new seed) (sp-random-state-t uint64-t)
  (sp-samples-new size out) (status-t sp-count-t sp-sample-t**)
  (sp-counts-new size out) (status-t sp-count-t sp-count-t**)
  (sp-synth-partial-1 start end modifies amp wvl phs)
  (sp-synth-partial-t sp-count-t sp-count-t sp-synth-count-t sp-sample-t* sp-count-t* sp-count-t)
  (sp-synth-partial-2 start end modifies amp1 amp2 wvl1 wvl2 phs1 phs2)
  (sp-synth-partial-t
    sp-count-t
    sp-count-t
    sp-synth-count-t sp-sample-t* sp-sample-t* sp-count-t* sp-count-t* sp-count-t sp-count-t))