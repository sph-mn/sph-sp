(pre-include-once alsa-asoundlib-h "alsa/asoundlib.h")
(sc-include "foreign/sph" "foreign/sph/status" "config" "status")
(pre-define (sp-octets->samples a) (/ a (sizeof sp-sample-t)))
(pre-define (sp-samples->octets a) (* a (sizeof sp-sample-t)))
(define (sp-alloc-channel-data channel-count sample-count) (sp-sample-t** b32 b32))
(define (sp-sin-lq a) (f32-s f32-s))
(define (sp-sinc a) (f32-s f32-s))
(define (sp-blackman a width) (f32-s f32-s size-t))
(define (sp-spectral-inversion-ir a a-len) (b0 sp-sample-t* size-t))
(define (sp-spectral-reversal-ir a a-len) (b0 sp-sample-t* size-t))
(define (sp-fft result result-len source source-len) (status-t sp-sample-t* b32 sp-sample-t* b32))

(define (sp-fft-inverse result result-len source source-len)
  (status-t sp-sample-t* b32 sp-sample-t* b32))

(define (sp-moving-average result source source-len prev prev-len next next-len start end distance)
  (boolean sp-sample-t* sp-sample-t* b32 sp-sample-t* b32 sp-sample-t* b32 b32 b32 b32))

(define (sp-convolve-one result a a-len b b-len)
  (b0 sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t))

(define (sp-convolve result a a-len b b-len carryover carryover-len)
  (b0 sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t sp-sample-t* size-t))

(define (sp-sine data start end sample-duration freq phase amp)
  (b0 sp-sample-t* b32 b32 f32_s f32_s f32_s f32_s))

(define (sp-sine-lq data start end sample-duration freq phase amp)
  (b0 sp-sample-t* b32 b32 f32_s f32_s f32_s f32_s))

(pre-define (sp-windowed-sinc-cutoff freq sample-rate)
  "f32-s integer -> f32-s
  radians-per-second samples-per-second -> cutoff-value"
  (/ (* 2 M_PI freq) sample-rate))

(define-type sp-windowed-sinc-state-t
  ; stores impulse response, parameters to create the current impulse response,
  ; and data needed for the next call
  (struct (data sp-sample-t*)
    ; allocated len
    (data-len size-t) (ir-len-prev size-t)
    (ir sp-sample-t*) (ir-len size-t) (sample-rate b32) (freq f32-s) (transition f32-s)))

(define (sp-windowed-sinc-ir-length transition) (size-t f32-s))

(define (sp-windowed-sinc-ir result result-len sample-rate freq transition)
  (b0 sp-sample-t** size-t* b32 f32-s f32-s))

(define (sp-windowed-sinc-state-destroy state) (b0 sp-windowed-sinc-state-t*))

(define (sp-windowed-sinc-state-create sample-rate freq transition state)
  (b8 b32 f32-s f32-s sp-windowed-sinc-state-t**))

(define (sp-windowed-sinc result source source-len sample-rate freq transition state)
  (status-i-t sp-sample-t* sp-sample-t* size-t b32 f32-s f32-s sp-windowed-sinc-state-t**))

(define-type sp-port-t
  ; generic input/output port handle.
  ; type: any of sp-port-type-* value
  ; position?: true if the port supports random access
  ; position-offset: header length
  (struct (sample-rate b32) (channel-count b32)
    (closed? boolean) (flags b8)
    (type b8) (position b64) (position-offset b16) (data b0*) (data-int int)))

(pre-define sp-port-type-alsa 0 sp-port-type-file 1 sp-port-bit-input 1 sp-port-bit-position 2)
(define (sp-port-read result port sample-count) (status-t sp-sample-t** sp-port-t* b32))
(define (sp-port-write port sample-count channel-data) (status-t sp-port-t* b32 sp-sample-t**))

(define (sp-file-open result path input? channel-count sample-rate)
  (status-t sp-port-t* b8* boolean b32-s b32-s))

(define (sp-alsa-open result device-name input? channel-count sample-rate latency)
  (status-t sp-port-t* b8* boolean b32-s b32-s b32-s))

(define (sp-port-close a) (status-t sp-port-t*))
