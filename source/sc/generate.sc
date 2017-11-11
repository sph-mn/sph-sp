(define (sp-sin-lq a) (f32-s f32-s)
  "lower precision version of sin() that is faster to compute"
  (define b f32-s (/ 4 M_PI))
  (define c f32-s (/ -4 (* M_PI M_PI)))
  (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-sinc a) (f32-s f32-s)
  "the normalised sinc function"
  (return (if* (= 0 a) 1 (/ (sin (* M_PI a)) (* M_PI a)))))

(sc-comment
  "write samples for a sine wave into dest.
   sample-duration: in seconds
   freq: radian frequency
   phase: phase offset
   amp: amplitude. 0..1
   defines sp-sine, sp-sine-lq")

(pre-define (define-sp-sine id sin)
  (define (id dest len sample-duration freq phase amp)
    (b0 sp-sample-t* b32 f32_s f32_s f32_s f32_s)
    (define index b32 0)
    (while (<= index len)
      (set (deref dest index) (* amp (sin (* freq phase sample-duration))))
      (inc phase)
      (inc index))))

(define-sp-sine sp-sine sin)
(define-sp-sine sp-sine-lq sp-sin-lq)
