(import (sph base) (sph sp) (rnrs bytevectors))

;what about a variable for sample-size, or what about channel-count so the interleaved data can actually be written, and bv-u32-map, and setting sample-size
;u32-bv-index-offet and frequency-time index are things that may benefit from abstraction, and loudness inside the osc loop?
;how about max loudness for clipping? inside osc?

(define loudness 394967295)


(define (osc-sine~ buffer time)
  (let loop ((bv-index 0) (time time))
    (if (< bv-index (bytevector-length buffer))
      (begin
        (bytevector-u32-native-set! buffer bv-index
          (inexact->exact (round (* (+ 1 (sin time)) loudness))))
        (bytevector-u32-native-set! buffer (+ bv-index 4)
          (inexact->exact (round (* (+ 1 (sin time)) loudness))))
        (loop (+ bv-index 8) (+ 0.01 time)))
      buffer)))

(sp-use-alsa () (sp-loop-alsa (l (buffer time) (osc-sine~ buffer time))))