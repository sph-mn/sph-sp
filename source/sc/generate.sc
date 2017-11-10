(define (sp-sin-lq a) (f32-s f32-s)
  "lower precision version of sin() that is faster to compute"
  (define b f32-s (/ 4 M_PI))
  (define c f32-s (/ -4 (* M_PI M_PI)))
  (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-sinc a) (f32-s f32-s)
  "the normalised sinc function"
  (return (if* (= 0 a) 1 (/ (sin (* M_PI a)) (* M_PI a)))))

(sc-comment
  "write samples for a sine wave into data between start at end.
   defines sp-sine, sp-sine-lq")

(pre-define (define-sp-sine id sin)
  (define (id data start end sample-duration freq phase amp)
    (b0 sp-sample-t* b32 b32 f32_s f32_s f32_s f32_s)
    (while (<= start end)
      (set (deref data start) (* amp (sin (* freq phase sample-duration))))
      (inc phase)
      (inc start))))

(define-sp-sine sp-sine sin)
(define-sp-sine sp-sine-lq sp-sin-lq)
