(define (sp-sin-lq a) (sp-sample-t sp-sample-t)
  "lower precision version of sin() that is faster to compute"
  (define b sp-sample-t (/ 4 M_PI))
  (define c sp-sample-t (/ -4 (* M_PI M_PI)))
  (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-sinc a) (sp-sample-t sp-sample-t)
  "the normalised sinc function"
  (return
    (if* (= 0 a) 1
      (/ (sin (* M_PI a)) (* M_PI a)))))

(sc-comment
  "write samples for a sine wave into dest.
   sample-duration: in seconds
   freq: radian frequency
   phase: phase offset
   amp: amplitude. 0..1
   defines sp-sine, sp-sine-lq")

(pre-define
  (define-sp-sine id sin)
  ; todo: assumes that sin() returns sp-sample-t
  (define (id dest len sample-duration freq phase amp)
    (b0 sp-sample-t* b32 sp-float-t sp-float-t sp-float-t sp-float-t)
    (define index b32 0)
    (while (<= index len)
      (set (array-get dest index) (* amp (sin (* freq phase sample-duration))))
      (inc phase)
      (inc index))))

(define-sp-sine sp-sine sin)
(define-sp-sine sp-sine-lq sp-sin-lq)