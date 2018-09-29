(pre-include
  "byteswap.h"
  "math.h"
  "inttypes.h"
  "../foreign/sph.c"
  "../foreign/sph/status.c" "../foreign/sph/types.c" "../foreign/sph/float.c" "./config.c")

(pre-define
  sp-port-type-alsa 0
  sp-port-type-file 1
  sp-port-bit-input 1
  sp-port-bit-output 2
  sp-port-bit-position 4
  sp-port-bit-closed 8
  sp-sample-format-f64 1
  sp-sample-format-f32 2
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
  (duration->sample-count seconds sample-rate) (* seconds sample-rate)
  (sample-count->duration sample-count sample-rate) (/ sample-count sample-rate)
  kiss-fft-scalar sp-sample-t
  (sp-windowed-sinc-cutoff freq sample-rate)
  (begin
    "sp-float-t integer -> sp-float-t
    radians-per-second samples-per-second -> cutoff-value"
    (/ (* 2 M_PI freq) sample-rate)))

(pre-if
  (= sp-sample-format sp-sample-format-f64)
  (pre-define
    sp-sample-t double
    sample-reverse-endian __bswap_64
    sp-sample-sum f64-sum
    sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT64_LE)
  (pre-if
    (= sp-sample-format-f32 sp-sample-format)
    (pre-define
      sp-sample-t float
      sample-reverse-endian __bswap_32
      sp-sample-sum f32-sum
      sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT_LE)))

(enum
  (sp-status-id-file-channel-mismatch
    sp-status-id-file-encoding
    sp-status-id-file-header
    sp-status-id-file-incompatible
    sp-status-id-file-incomplete
    sp-status-id-eof
    sp-status-id-input-type
    sp-status-id-memory
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
  sp-windowed-sinc-state-t
  (type
    (struct
      (carryover sp-sample-t*)
      (carryover-len sp-sample-count-t)
      (carryover-alloc-len sp-sample-count-t)
      (freq sp-float-t)
      (ir sp-sample-t*)
      (ir-len sp-sample-count-t)
      (sample-rate sp-sample-rate-t)
      (transition sp-float-t)))
  (sp-port-read port sample-count result-samples)
  (status-t sp-port-t* sp-sample-count-t sp-sample-t**)
  (sp-port-write port sample-count channel-data)
  (status-t sp-port-t* sp-sample-count-t sp-sample-t**) (sp-port-position port result-position)
  (status-t sp-port-t* sp-sample-count-t*) (sp-port-set-position port sample-index)
  (status-t sp-port-t* sp-sample-count-t) (sp-file-open path channel-count sample-rate result-port)
  (status-t uint8-t* sp-channel-count-t sp-sample-count-t sp-port-t*)
  (sp-alsa-open device-name is-input channel-count sample-rate latency result-port)
  (status-t uint8-t* boolean sp-channel-count-t sp-sample-rate-t int32-t sp-port-t*)
  (sp-port-close a) (status-t sp-port-t*)
  (sp-alloc-channel-array channel-count sample-count result-array)
  (status-t sp-channel-count-t sp-sample-count-t sp-sample-t***) (sp-status-description a)
  (uint8-t* status-t) (sp-status-name a)
  (uint8-t* status-t) (sp-sine len sample-duration freq phase amp result-samples)
  (void sp-sample-count-t sp-float-t sp-float-t sp-float-t sp-float-t sp-sample-t*)
  (sp-sine-lq len sample-duration freq phase amp result-samples)
  (void sp-sample-count-t sp-float-t sp-float-t sp-float-t sp-float-t sp-sample-t*) (sp-sinc a)
  (sp-float-t sp-float-t) (sp-windowed-sinc-ir-length transition)
  (sp-sample-count-t sp-float-t)
  (sp-windowed-sinc-ir sample-rate freq transition result-len result-ir)
  (status-t sp-sample-rate-t sp-float-t sp-float-t sp-sample-count-t* sp-sample-t**)
  (sp-windowed-sinc-state-free state) (void sp-windowed-sinc-state-t*)
  (sp-windowed-sinc-state-create sample-rate freq transition result-state)
  (status-t sp-sample-rate-t sp-float-t sp-float-t sp-windowed-sinc-state-t**)
  (sp-windowed-sinc source source-len sample-rate freq transition result-state result-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t sp-sample-rate-t sp-float-t sp-float-t sp-windowed-sinc-state-t** sp-sample-t*)
  (sp-window-blackman a width) (sp-float-t sp-float-t sp-sample-count-t)
  (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-sample-count-t)
  (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-sample-count-t)
  (sp-fft len source source-len result-samples)
  (status-t sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-t*)
  (sp-ifft len source source-len result-samples)
  (status-t sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-t*)
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
    sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-count-t sp-sample-t* sp-sample-t*))