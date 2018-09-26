(pre-include
  "byteswap.h"
  "math.h" "./foreign/sph.c" "./foreign/sph/status.c" "./foreign/sph/float.c" "./main/config.c")

(pre-define
  sp-port-type-alsa 0
  sp-port-type-file 1
  sp-port-bit-input 1
  sp-port-bit-output 2
  sp-port-bit-position 4
  sp-port-bit-closed 8
  sp-sample-format-f64 1
  sp-sample-format-f32 2
  (sp-octets->samples a) (/ a (sizeof sp-sample-t))
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
    sp-sample-t f64
    sample-reverse-endian __bswap_64
    sp-sample-sum f64-sum
    sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT64_LE)
  (pre-if
    (= sp-sample-format-f32 sp-sample-format)
    (pre-define
      sp-sample-t f32
      sample-reverse-endian __bswap_32
      sp-sample-sum f32-sum
      sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT_LE)))

(enum
  (sp-status-group-alsa
    sp-status-group-libc
    sp-status-group-sndfile
    sp-status-group-sp
    sp-status-id-file-channel-mismatch
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
      (sample-rate uint32-t)
      (channel-count uint32-t)
      (data void*)))
  (sp-port-read result port sample-count) (status-t sp-sample-t** sp-port-t* uint32-t)
  (sp-port-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**)
  (sp-port-position result port) (status-t size-t* sp-port-t*)
  (sp-port-set-position port sample-index) (status-t sp-port-t* size-t)
  (sp-file-open result path channel-count sample-rate)
  (status-t sp-port-t* uint8-t* uint32-t uint32-t)
  (sp-alsa-open result device-name input? channel-count sample-rate latency)
  (status-t sp-port-t* uint8-t* boolean uint32-t uint32-t int32-t) (sp-port-close a)
  (status-t sp-port-t*) (sp-alloc-channel-array channel-count sample-count)
  (sp-sample-t** uint32-t uint32-t) (sp-status-description a)
  (uint8-t* status-t) (sp-status-name a)
  (uint8-t* status-t) (sp-sin-lq a)
  (sp-float-t sp-float-t) (sp-sinc a)
  (sp-float-t sp-float-t) (sp-sine data len sample-duration freq phase amp)
  (void sp-sample-t* uint32-t sp-float-t sp-float-t sp-float-t sp-float-t)
  (sp-sine-lq data len sample-duration freq phase amp)
  (void sp-sample-t* uint32-t sp-float-t sp-float-t sp-float-t sp-float-t) sp-windowed-sinc-state-t
  (type
    (struct
      (data sp-sample-t*)
      ; allocated len
      (data-len size-t)
      (ir-len-prev size-t)
      (ir sp-sample-t*)
      (ir-len size-t)
      (sample-rate uint32-t)
      (freq sp-float-t)
      (transition sp-float-t)))
  (sp-windowed-sinc-ir-length transition) (size-t sp-float-t)
  (sp-windowed-sinc-ir result result-len sample-rate freq transition)
  (void sp-sample-t** size-t* uint32-t sp-float-t sp-float-t) (sp-windowed-sinc-state-destroy state)
  (void sp-windowed-sinc-state-t*) (sp-windowed-sinc-state-create sample-rate freq transition state)
  (uint8-t uint32-t sp-float-t sp-float-t sp-windowed-sinc-state-t**)
  (sp-windowed-sinc result source source-len sample-rate freq transition state)
  (status-i-t
    sp-sample-t* sp-sample-t* size-t uint32-t sp-float-t sp-float-t sp-windowed-sinc-state-t**)
  (sp-window-blackman a width) (sp-float-t sp-float-t size-t)
  (sp-spectral-inversion-ir a a-len) (void sp-sample-t* size-t)
  (sp-spectral-reversal-ir a a-len) (void sp-sample-t* size-t)
  (sp-fft result result-len source source-len) (status-t sp-sample-t* uint32-t sp-sample-t* uint32-t)
  (sp-ifft result result-len source source-len)
  (status-t sp-sample-t* uint32-t sp-sample-t* uint32-t)
  (sp-moving-average result source source-len prev prev-len next next-len start end distance)
  (status-i-t
    sp-sample-t*
    sp-sample-t* uint32-t sp-sample-t* uint32-t sp-sample-t* uint32-t uint32-t uint32-t uint32-t)
  (sp-convolve-one result a a-len b b-len)
  (void sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t)
  (sp-convolve result a a-len b b-len carryover carryover-len)
  (void sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t sp-sample-t* size-t))