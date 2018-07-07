(pre-define
  kiss-fft-scalar sp-sample-t
  (sp-windowed-sinc-cutoff freq sample-rate)
  (begin
    "sp-float-t integer -> sp-float-t
    radians-per-second samples-per-second -> cutoff-value"
    (/ (* 2 M_PI freq) sample-rate)))

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
      (sample-rate b32)
      (freq sp-float-t)
      (transition sp-float-t))))

(define (sp-windowed-sinc-ir-length transition) (size-t sp-float-t))

(define (sp-windowed-sinc-ir result result-len sample-rate freq transition)
  (b0 sp-sample-t** size-t* b32 sp-float-t sp-float-t))

(define (sp-windowed-sinc-state-destroy state) (b0 sp-windowed-sinc-state-t*))

(define (sp-windowed-sinc-state-create sample-rate freq transition state)
  (b8 b32 sp-float-t sp-float-t sp-windowed-sinc-state-t**))

(define (sp-windowed-sinc result source source-len sample-rate freq transition state)
  (status-i-t sp-sample-t* sp-sample-t* size-t b32 sp-float-t sp-float-t sp-windowed-sinc-state-t**))

(define (sp-window-blackman a width) (sp-float-t sp-float-t size-t))
(define (sp-spectral-inversion-ir a a-len) (b0 sp-sample-t* size-t))
(define (sp-spectral-reversal-ir a a-len) (b0 sp-sample-t* size-t))
(define (sp-fft result result-len source source-len) (status-t sp-sample-t* b32 sp-sample-t* b32))
(define (sp-ifft result result-len source source-len) (status-t sp-sample-t* b32 sp-sample-t* b32))

(define (sp-moving-average result source source-len prev prev-len next next-len start end distance)
  (status-i-t sp-sample-t* sp-sample-t* b32 sp-sample-t* b32 sp-sample-t* b32 b32 b32 b32))

(define (sp-convolve-one result a a-len b b-len)
  (b0 sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t))

(define (sp-convolve result a a-len b b-len carryover carryover-len)
  (b0 sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t sp-sample-t* size-t))