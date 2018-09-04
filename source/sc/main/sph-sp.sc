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
  sp-sample-type-f64 1
  sp-sample-type-f32 2
  (sp-octets->samples a) (/ a (sizeof sp-sample-t))
  (sp-samples->octets a) (* a (sizeof sp-sample-t))
  (duration->sample-count seconds sample-rate) (* seconds sample-rate)
  (sample-count->duration sample-count sample-rate) (/ sample-count sample-rate)
  ; memory
  (sp-alloc-require a)
  (begin
    "set sph/status object to group sp and id memory-error and goto \"exit\" label if \"a\" is 0"
    (if (not a) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))
  (sp-alloc-set a octet-count)
  (begin
    (set a (malloc octet-count))
    (sp-alloc-require a))
  (sp-alloc-set-zero a octet-count)
  (begin
    (set a (calloc octet-count 1))
    (sp-alloc-require a))
  (sp-alloc-define id type octet-count)
  (begin
    (declare id type)
    (sp-alloc-set id octet-count))
  (sp-alloc-define-zero id type octet-count)
  (begin
    (declare id type)
    (sp-alloc-set-zero id octet-count))
  (sp-alloc-define-samples id sample-count)
  (sp-alloc-define id sp-sample-t* (* sample-count (sizeof sp-sample-t)))
  (sp-alloc-set-samples a sample-count) (sp-alloc-set a (* sample-count (sizeof sp-sample-t)))
  (sp-alloc-define-samples-zero id sample-count)
  (sp-alloc-define-zero id sp-sample-t* (sp-samples->octets (sizeof sp-sample-t)))
  (sp-alloc-set-samples-zero a sample-count) (sp-alloc-set-zero a (sp-samples->octets sample-count))
  ; transformators
  kiss-fft-scalar sp-sample-t
  (sp-windowed-sinc-cutoff freq sample-rate)
  (begin
    "sp-float-t integer -> sp-float-t
    radians-per-second samples-per-second -> cutoff-value"
    (/ (* 2 M_PI freq) sample-rate)))

(pre-if
  (= sp-sample-type sp-sample-type-f64)
  (pre-define
    sp-sample-t f64-s
    sample-reverse-endian __bswap_64
    sp-sample-sum f64-sum
    sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT64_LE)
  (pre-if
    (= sp-sample-type-f32 sp-sample-type)
    (pre-define
      sp-sample-t f32-s
      sample-reverse-endian __bswap_32
      sp-sample-sum f32-sum
      sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT_LE)))

(declare sp-port-t
  (type
    (struct
      (type uint8-t)
      (flags uint8-t)
      (sample-rate uint32-t)
      (channel-count uint32-t)
      (data void*))))

(define (sp-port-read result port sample-count) (status-t sp-sample-t** sp-port-t* uint32-t))
(define (sp-port-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**))
(define (sp-port-position result port) (status-t size-t* sp-port-t*))
(define (sp-port-set-position port sample-index) (status-t sp-port-t* size-t))
(define (sp-file-open result path channel-count sample-rate) (status-t sp-port-t* uint8-t* uint32-t uint32-t))

(define (sp-alsa-open result device-name input? channel-count sample-rate latency)
  (status-t sp-port-t* uint8-t* boolean uint32-t uint32-t int32-t))

(define (sp-port-close a) (status-t sp-port-t*))
(pre-include "generic/base/io.h")
(define (sp-alloc-channel-array channel-count sample-count) (sp-sample-t** uint32-t uint32-t))
(define (sp-status-description a) (uint8-t* status-t))
(define (sp-status-name a) (uint8-t* status-t))
(define (sp-sin-lq a) (sp-float-t sp-float-t))
(define (sp-sinc a) (sp-float-t sp-float-t))

(define (sp-sine data len sample-duration freq phase amp)
  (void sp-sample-t* uint32-t sp-float-t sp-float-t sp-float-t sp-float-t))

(define (sp-sine-lq data len sample-duration freq phase amp)
  (void sp-sample-t* uint32-t sp-float-t sp-float-t sp-float-t sp-float-t))

(declare
  ; stores impulse response, parameters to create the current impulse response,
  ; and data needed for the next call
  sp-windowed-sinc-state-t
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
      (transition sp-float-t))))

(define (sp-windowed-sinc-ir-length transition) (size-t sp-float-t))

(define (sp-windowed-sinc-ir result result-len sample-rate freq transition)
  (void sp-sample-t** size-t* uint32-t sp-float-t sp-float-t))

(define (sp-windowed-sinc-state-destroy state) (void sp-windowed-sinc-state-t*))

(define (sp-windowed-sinc-state-create sample-rate freq transition state)
  (uint8-t uint32-t sp-float-t sp-float-t sp-windowed-sinc-state-t**))

(define (sp-windowed-sinc result source source-len sample-rate freq transition state)
  (status-i-t sp-sample-t* sp-sample-t* size-t uint32-t sp-float-t sp-float-t sp-windowed-sinc-state-t**))

(define (sp-window-blackman a width) (sp-float-t sp-float-t size-t))
(define (sp-spectral-inversion-ir a a-len) (void sp-sample-t* size-t))
(define (sp-spectral-reversal-ir a a-len) (void sp-sample-t* size-t))
(define (sp-fft result result-len source source-len) (status-t sp-sample-t* uint32-t sp-sample-t* uint32-t))
(define (sp-ifft result result-len source source-len) (status-t sp-sample-t* uint32-t sp-sample-t* uint32-t))

(define (sp-moving-average result source source-len prev prev-len next next-len start end distance)
  (status-i-t sp-sample-t* sp-sample-t* uint32-t sp-sample-t* uint32-t sp-sample-t* uint32-t uint32-t uint32-t uint32-t))

(define (sp-convolve-one result a a-len b b-len)
  (void sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t))

(define (sp-convolve result a a-len b b-len carryover carryover-len)
  (void sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t sp-sample-t* size-t))