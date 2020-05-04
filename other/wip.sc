(declare sp-random-discrete-t
  (type
    (struct
      (cuprob sp-sample-t)
      (cuprob-size sp-count-t)
      (random-state sp-random-state-t)
      (sum sp-sample-t))))

(define (sp-random-discrete-new probabilities size random-state out)
  (void sp-sample-t* sp-count-t sp-random-state-t sp-random-discrete-t*)
  (f64-cusum probabilities size out:cuprob)
  (set out:sum (array-get out:cuprob (- size 1))))

(define (random-discrete state) (sp-sample-t sp-random-discrete-t*)
  (declare a sp-sample-t deviate sp-sample-t)
  (set a 0 deviate (sp-random state:random-state))
  (for ((set i 0) (< i state:size) (set i (+ 1 i)))
    (if (< deviate (array-get cuprob i)) (return a)))
  (return a))

(define (f64-cusum in in-size out) (void double* double*)
  "calculate cumulative sums from the given numbers.
   (a b c ...) -> (a (+ a b) (+ a b c) ...).
   every sum is calculated from all input values with floating point error compensation"
  (declare i double)
  (for ((set i 0) (< i in-size) (set i (+ 1 i))) (set (array-get out i) (f64-sum in i))))

(define (sp-event-group-f start end out event) (void sp-count-t sp-count-t sp-block-t sp-event-t*)
  (sp-seq start end out (pointer-get (convert-type event:state sp-events-t*))))

(define (sp-event-group start end events out) (void sp-count-t sp-count-t sp-events-t* sp-event-t*)
  "an event that calls seq to process other events"
  (set out:start start out:end end out:f sp-event-group-f out:state events))
