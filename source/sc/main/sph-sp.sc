(pre-include "byteswap.h" "math.h" "inttypes.h")

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

(sc-include "../foreign/sph" "../foreign/sph/status" "../foreign/sph/i-array")

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
  (sp-fft input-len input/output-real input/output-imag) (status-t sp-count-t double* double*)
  (sp-ffti input-len input/output-real input/output-imag) (status-t sp-count-t double* double*)
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
  sp-sample-t* (sp-initialise)
  status-t (sp-cheap-phase-96 current change)
  (sp-count-t sp-count-t sp-count-t) (sp-cheap-phase-96-float current change)
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

(pre-define sp-events-new i-array-allocate-sp-events-t)

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

(i-array-declare-type sp-events-t sp-event-t)

(declare
  (sp-seq-events-prepare a) (void sp-events-t)
  (sp-seq time offset size output events)
  (void sp-count-t sp-count-t sp-count-t sp-block-t sp-events-t)
  (sp-synth-event start end channel-count config-len config out-event)
  (status-t sp-count-t sp-count-t sp-count-t sp-count-t sp-synth-partial-t* sp-event-t*)
  (sp-plot-samples a) (void sp-sample-t*))