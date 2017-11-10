(pre-define kiss-fft-scalar sp-sample-t)

(pre-define (sp-windowed-sinc-cutoff freq sample-rate)
  "f32-s integer -> f32-s
  radians-per-second samples-per-second -> cutoff-value"
  (/ (* 2 M_PI freq) sample-rate))

(define-type sp-windowed-sinc-state-t
  ; stores impulse response, parameters to create the current impulse response,
  ; and data needed for the next call
  (struct
    (data sp-sample-t*)
    ; allocated len
    (data-len size-t)
    (ir-len-prev size-t)
    (ir sp-sample-t*)
    (ir-len size-t)
    (sample-rate b32)
    (freq f32-s)
    (transition f32-s)))

(define (sp-windowed-sinc-ir-length transition) (size-t f32-s))

(define (sp-windowed-sinc-ir result result-len sample-rate freq transition)
  (b0 sp-sample-t** size-t* b32 f32-s f32-s))

(define (sp-windowed-sinc-state-destroy state) (b0 sp-windowed-sinc-state-t*))

(define (sp-windowed-sinc-state-create sample-rate freq transition state)
  (b8 b32 f32-s f32-s sp-windowed-sinc-state-t**))

(define (sp-windowed-sinc result source source-len sample-rate freq transition state)
  (status-i-t sp-sample-t* sp-sample-t* size-t b32 f32-s f32-s sp-windowed-sinc-state-t**))

(define (sp-window-blackman a width) (f32-s f32-s size-t))
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
