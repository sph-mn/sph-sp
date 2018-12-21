(pre-include "byteswap.h" "math.h" "inttypes.h")
(sc-include "../foreign/sph" "../foreign/sph/status")
(pre-if-not-defined sp-sample-format (sc-include "config"))

(pre-define
  f32 float
  f64 double
  boolean uint8-t
  sp-port-type-alsa 0
  sp-port-type-file 1
  sp-port-bit-input 1
  sp-port-bit-output 2
  sp-port-bit-position 4
  sp-port-bit-closed 8
  sp-port-mode-read 1
  sp-port-mode-write 2
  sp-port-mode-read-write 3
  sp-sample-format-f64 1
  sp-sample-format-f32 2
  sp-sample-format-int32 3
  sp-sample-format-int16 4
  sp-sample-format-int8 5
  sp-status-group-libc "libc"
  sp-status-group-sndfile "sndfile"
  sp-status-group-sp "sp"
  sp-status-group-sph "sph"
  sp-status-group-alsa "alsa"
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
  sp-windowed-sinc-ir-transition sp-windowed-sinc-ir-cutoff
  (sp-fftr-output-len input-len) (+ 1 (/ input-len 2))
  (sp-fftri-output-len input-len) (* 2 (- input-len 1)))

(pre-cond
  ( (= sp-sample-format-f64 sp-sample-format)
    (pre-define
      sp-sample-t double
      sp-sample-sum f64-sum
      sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT64
      sp-sf-write sf-writef-double
      sp-sf-read sf-readf-double
      kiss-fft-scalar sp-sample-t))
  ( (= sp-sample-format-f32 sp-sample-format)
    (pre-define
      sp-sample-t float
      sp-sample-sum f32-sum
      sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT
      sp-sf-write sf-writef-float
      sp-sf-read sf-readf-float
      kiss-fft-scalar sp-sample-t))
  ( (= sp-sample-format-int32 sp-sample-format)
    (pre-define
      sp-sample-t int32-t
      (sp-sample-sum a b) (+ a b)
      sp-alsa-snd-pcm-format SND_PCM_FORMAT_S32
      sp-sf-write sf-writef-int
      sp-sf-read sf-readf-int
      FIXED_POINT 32))
  ( (= sp-sample-format-int16 sp-sample-format)
    (pre-define
      sp-sample-t int16-t
      (sp-sample-sum a b) (+ a b)
      sp-alsa-snd-pcm-format SND_PCM_FORMAT_S16
      sp-sf-write sf-writef-short
      sp-sf-read sf-readf-short
      FIXED_POINT 16))
  ( (= sp-sample-format-int8 sp-sample-format)
    (pre-define
      sp-sample-t int8-t
      (sp-sample-sum a b) (+ a b)
      sp-alsa-snd-pcm-format SND_PCM_FORMAT_S8
      sp-sf-write sf-writef-short
      sp-sf-read sf-readf-short
      FIXED_POINT 8)))

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
    sp-status-id-port-closed sp-status-id-port-position sp-status-id-port-type sp-status-id-undefined))

(declare
  sp-port-t
  (type
    (struct
      (type uint8-t)
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
      (ir-len sp-sample-count-t)))
  (sp-port-read port sample-count result-channel-data result-sample-count)
  (status-t sp-port-t* sp-sample-count-t sp-sample-t** sp-sample-count-t*)
  (sp-port-write port channel-data sample-count result-sample-count)
  (status-t sp-port-t* sp-sample-t** sp-sample-count-t sp-sample-count-t*)
  (sp-port-position port result-position) (status-t sp-port-t* sp-sample-count-t*)
  (sp-port-position-set port sample-offset) (status-t sp-port-t* size-t)
  (sp-file-open path mode channel-count sample-rate result-port)
  (status-t uint8-t* int sp-channel-count-t sp-sample-rate-t sp-port-t*)
  (sp-alsa-open device-name mode channel-count sample-rate latency result-port)
  (status-t uint8-t* int sp-channel-count-t sp-sample-rate-t int32-t sp-port-t*) (sp-port-close a)
  (status-t sp-port-t*) (sp-alloc-channel-array channel-count sample-count result-array)
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
  (sp-fftr input input-len output) (status-t sp-sample-t* sp-sample-count-t sp-sample-t*)
  (sp-fftri input input-len output) (status-t sp-sample-t* sp-sample-count-t sp-sample-t*)
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