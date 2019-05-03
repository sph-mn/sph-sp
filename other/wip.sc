sp-default-random-state
MTRand

(define (sp-seq-parallel time offset size output events)
  (void sp-count-t sp-count-t sp-count-t sp-block-t sp-events-t)
  (define (merge results rest-events)
    (let
      loop
      ((results results) (events null))
      (if (null? results) (append (reverse events) rest-events)
        (apply
          (l
            (offset event-output-size event-output event)
            (each
              (l
                (output event-output)
                (each-integer
                  event-output-size
                  (l
                    (sample-index)
                    (sp-samples-set!
                      output
                      (+ offset sample-index)
                      (float-sum
                        (sp-samples-ref output (+ offset sample-index))
                        (sp-samples-ref event-output sample-index))))))
              output event-output)
            (loop (tail results) (pair event events)))
          (touch (first results))))))
  (let
    ((time-end (+ time size)) (channels (length output)))
    (let
      loop
      ((results null) (rest events))
      (if (null? rest) (merge results rest)
        (let*
          ( (event (first rest))
            (data (seq-event-data event))
            (start (seq-event-data-start data)) (end (seq-event-data-end data)))
          (if (< time-end start) (merge results rest)
            (if (> time end) (loop results (tail rest))
              (loop
                (pair
                  (future
                    (seq-event-f-arguments
                      time
                      time-end
                      start
                      end
                      offset
                      size
                      (l
                        (time offset size)
                        (let
                          (output (sp-block-new channels size))
                          (list
                            offset
                            size output ((seq-event-data-f data) time offset size output event))))))
                  results)
                (tail rest)))))))))

(define (sp-random-real random-state) (double MTRand)
  (set sp-default-random-state (seedRand 1337))
  (genRand &random-state))

(define (sp-noise-uniform random-state) (sp-sample-t MTRand) (- (* 2 (genRand &random-state)) 1))
;
; # todo
; sp-noise
; block->file
; blocks->file
; cached-event
; cheap-noise-event
; event-group
; events->block
; noise-event
; seq-parallel
; sp-event-with-resolution
; sp-path-new*
