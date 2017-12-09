(define-test-module (test module sph sp generate sequencer)
  (import
    (sph sp generate sequencer))

  (test-execute-procedures-lambda
    (seq-event-list->events (()) () (((#()) (#()))) ((#()) (#())) ((() ())) (() ()))
    (seq-events-merge
      ; equal length
      (((#(a) #(b)) (#(c) #(d))) ((#(e) #(f)) (#(g) #(h))))
      ((#(a) #(b) #(e) #(f)) (#(c) #(d) #(g) #(h)))
      ; shorter longer
      (((#(a) #(b)) (#(c) #(d))) ((#(e) #(f)))) ((#(a) #(b) #(e) #(f)) (#(c) #(d)))
      ; longer shorter
      (((#(e) #(f))) ((#(a) #(b)) (#(c) #(d)))) ((#(e) #(f) #(a) #(b)) (#(c) #(d))))
    (seq-default-mixer
      ; default mixer should work with numbers and number lists
      ( (unquote
          (list (seq-output-new (q noname) (list 0.02 0.02 0.03) #f #f)
            (seq-output-new (q noname) (list 0.04 0.05 0.06) #f #f)
            (seq-output-new (q noname) (list 0.07 0.08 0.09) #f #f))))
      (0.13 0.15 0.18)
      ( (unquote
          (list (seq-output-new (q noname) 0.02 #f #f) (seq-output-new (q noname) 0.04 #f #f)
            (seq-output-new (q noname) 0.07 #f #f))))
      (0.13))))
