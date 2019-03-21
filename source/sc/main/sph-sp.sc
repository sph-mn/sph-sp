(pre-include "byteswap.h" "math.h" "inttypes.h")

(pre-if-not-defined
  sp-config-is-set
  (pre-define
    sp-channel-count-t uint8-t
    sp-file-format (bit-or SF-FORMAT-WAV SF-FORMAT-FLOAT)
    sp-float-t double
    sp-sample-count-t size-t
    sp-sample-rate-t uint32-t
    sp-sample-sum f64-sum
    sp-sample-t double
    sp-sf-read sf-readf-double
    sp-sf-write sf-writef-double
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
  (sp-windowed-sinc-ir-cutoff freq sample-rate)
  (begin
    "sp-float-t integer -> sp-float-t
    radians-per-second samples-per-second -> cutoff-value"
    (/ (* 2 M_PI freq) sample-rate))
  sp-windowed-sinc-ir-transition sp-windowed-sinc-ir-cutoff)

(sc-include "../foreign/sph" "../foreign/sph/status")

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

(declare
  sp-file-t
  (type
    (struct
      (flags uint8-t)
      (sample-rate sp-sample-rate-t)
      (channel-count sp-channel-count-t)
      (data void*)))
  sp-convolution-filter-ir-f-t
  (type (function-pointer status-t void* sp-sample-t** sp-sample-count-t*))
  sp-convolution-filter-state-t
  (type
    (struct
      (carryover sp-sample-t*)
      (carryover-len sp-sample-count-t)
      (carryover-alloc-len sp-sample-count-t)
      (ir sp-sample-t*)
      (ir-f sp-convolution-filter-ir-f-t)
      (ir-f-arguments void*)
      (ir-f-arguments-len uint8-t)
      (ir-len sp-sample-count-t)))
  (sp-file-read file sample-count result-channel-data result-sample-count)
  (status-t sp-file-t* sp-sample-count-t sp-sample-t** sp-sample-count-t*)
  (sp-file-write file channel-data sample-count result-sample-count)
  (status-t sp-file-t* sp-sample-t** sp-sample-count-t sp-sample-count-t*)
  (sp-file-position file result-position) (status-t sp-file-t* sp-sample-count-t*)
  (sp-file-position-set file sample-offset) (status-t sp-file-t* size-t)
  (sp-file-open path mode channel-count sample-rate result-file)
  (status-t uint8-t* int sp-channel-count-t sp-sample-rate-t sp-file-t*) (sp-file-close a)
  (status-t sp-file-t*) (sp-alloc-channel-array channel-count sample-count result-array)
  (status-t sp-channel-count-t sp-sample-count-t sp-sample-t***) (sp-status-description a)
  (uint8-t* status-t) (sp-status-name a)
  (uint8-t* status-t) (sp-sin-lq a)
  (sp-sample-t sp-float-t) (sp-sinc a)
  (sp-float-t sp-float-t) (sp-windowed-sinc-lp-hp-ir-length transition)
  (sp-sample-count-t sp-float-t) (sp-windowed-sinc-ir cutoff transition result-len result-ir)
  (status-t sp-float-t sp-float-t sp-sample-count-t* sp-sample-t**)
  (sp-convolution-filter-state-free state) (void sp-convolution-filter-state-t*)
  (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-sample-count-t*)
  (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)
  (status-t sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-sample-t** sp-sample-count-t*)
  (sp-windowed-sinc-lp-hp-ir-f arguments out-ir out-len)
  (status-t void* sp-sample-t** sp-sample-count-t*)
  (sp-windowed-sinc-bp-br-ir-f arguments out-ir out-len)
  (status-t void* sp-sample-t** sp-sample-count-t*)
  (sp-windowed-sinc-lp-hp in in-len cutoff transition is-high-pass out-state out-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-bp-br
    in in-len cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t
    sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  (sp-window-blackman a width) (sp-float-t sp-float-t sp-sample-count-t)
  (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-sample-count-t)
  (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-sample-count-t)
  (sp-fft input-len input/output-real input/output-imag)
  (status-t sp-sample-count-t sp-sample-t* sp-sample-t*)
  (sp-ffti input-len input/output-real input/output-imag)
  (status-t sp-sample-count-t sp-sample-t* sp-sample-t*)
  (sp-moving-average source source-len prev prev-len next next-len radius start end result-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t
    sp-sample-t*
    sp-sample-count-t
    sp-sample-t* sp-sample-count-t sp-sample-count-t sp-sample-count-t sp-sample-count-t sp-sample-t*)
  (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-t*)
  (sp-convolve a a-len b b-len result-carryover-len result-carryover result-samples)
  (void
    sp-sample-t*
    sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-count-t sp-sample-t* sp-sample-t*)
  (sp-channel-data-free a channel-count) (void sp-sample-t** sp-channel-count-t)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-sample-count-t*)
  (sp-null-ir out-ir out-len) (status-t sp-sample-t** sp-sample-count-t*)
  (sp-passthrough-ir out-ir out-len) (status-t sp-sample-t** sp-sample-count-t*))