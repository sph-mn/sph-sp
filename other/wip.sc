(pre-define sp-count-t sp-sample-count-t)

(declare
  sp-block-t
  (type
    (struct
      (channels sp-channel-count-t)
      (size sp-count-t)
      (samples sp-sample-t*)))
  db-event-t struct
  sp-event-f-t
  (type
    (function-pointer
      sp-event-t sp-count-t sp-count-t sp-count-t sp-block-t
      (struct
        sp-event-t)))
  sp-event-t
  (type
    (struct
      (state void*)
      (start sp-count-t)
      (end sp-count-t)
      (f sp-event-f-t)))
  sp-events-t
  (type
    (struct
      (size sp-count-t)
      (data sp-event-t*)))
  sp-default-random-state MTRand)

(define (sp-seq time offset size output events)
  (void sp-count-t sp-count-t sp-count-t sp-block-t sp-events-t)
  (declare
    time-end sp-count-t
    e sp-event-t
    e-time sp-event-t
    e-offset sp-event-t
    e-offset-right sp-event-t
    i sp-count-t)
  (set time-end (+ time size))
  (for ((set i 0) (< i events.size) (set i (+ 1 i)))
    (set e (array-get events.data i))
    (cond
      ((< time-end e.start) break)
      ((<= end time) continue)
      (else
        (set
          e-time
          (if (< start time) (- time start)
            0)
          e-offset
          (if (> start time) (- start time)
            0)
          e-offset-right
          (if (> end time-end) 0
            (- time-end end)))
        (e.f e-time (+ offset e-offset) (- size e-offset e-offset-right) output e)))))

(define*
  (sp-seq-parallel time offset size output events)
  "integer integer integer (samples:channel ...) seq-events -> seq-events
     calls one or multiple functions that add to the given output block in parallel at event-defined times and sums result samples.
     write to output after given offset. output samples length must be equal or greater than offset + size.
     the output block to fill will be of specified size.
     the returned object is are the events with finished events removed and can be passed to the next call to seq-parallel or seq.
     seq-parallel can be nested, but unless more cpu cores are available then using seq will be more efficient.
     events are created with seq-events-new and seq-event-new.
     # example
     (let
       ( (result (sp-block-new 1 96000))
         (events
           (seq-events-new*
             (seq-event-new 10000 96000
               (lambda (time offset size output event)
                 (for-each
                   (lambda (output)
                     (each-integer size
                       (l (index)
                         (sp-samples-set! output (+ offset index)
                           (sp-sample-sum (sp-samples-ref output (+ offset index))
                             (sp-sine~ (+ index time)))))))
                   output)
                 event)))))
       (seq-parallel 0 0 96000 result events)
       (sp-plot-samples (first result)))"
  ; take sorted event objects that have a start/end time and a function that is called with a time offset and an output array.
  ; each block, filter events that write into the block, allocate arrays for the portions they write to, call the events in parallel,
  ; merge the event blocks into the given output block. return a list of events with finished events removed.
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

(define (sp-triangle t a b) (sp-sample-t sp-count-t sp-count-t)
  "return a sample for a triangular wave with center offsets a left and b right.
   creates sawtooth waves if either a or b is 0"
  (declare remainder sp-count-t)
  (set remainder (modulo t (+ a b)))
  (if (< remainder a) (* remainder (/ 2 (convert-type a sp-sample-t)))
    (*
      (- (convert-type b sp-sample-t) (- 2 (convert-type a sp-sample-t)))
      (/ 2 (convert-type b sp-sample-t)))))

(define (sp-square-96 t) (sp-sample-t sp-count-t)
  (if (< (modulo (* 2 t) (* 2 96000)) 96000) -1
    1))

(define (sp-random-real random-state) (double MTRand)
  (set sp-default-random-state (seedRand 1337))
  (genRand &random-state))

(define (sp-noise-uniform random-state) (sp-sample-t MTRand) (- (* 2 (genRand &random-state)) 1))

(define (sp-fftr a) "samples -> #(complex ...)"
  (let*
    ( (b (sp-fft (list->vector (map (l (a) (make-rectangular a 0)) (sp-samples->list a)))))
      (c-len (+ 1 (/ (vector-length b) 2))) (c (make-vector c-len 0)))
    (let
      loop
      ((i 0))
      (if (< i c-len)
        (begin
          (vector-set! c i (vector-ref b i))
          (loop (+ 1 i)))))
    c))

(define (sp-fftri a) "#(complex ...) -> samples"
  (sp-samples-from-list (map real-part (vector->list (sp-ffti a)))))

(define (sp-spectrum a) "samples -> #(real ...)"
  (vector-map (l (b) (* 2 (/ (magnitude b) (sp-samples-length a)))) (sp-fftr a)))

(define*
  (sp-plot-samples-display-file file-path #:key (type (q lines)) (color "blue"))
  "string #:type symbol:lines/points #:color string -> unspecified
     type and color correspond to gnuplot options"
  (system*
    "gnuplot"
    "--persist"
    "-e"
    (string-append
      "set key off; set size ratio 0.5; plot \""
      file-path
      "\""
      " with "
      (if (string? type) type
        (case type ((points) "points pointtype 5 ps 0.3")
          (else "lines")))
      " lc rgb \"" color "\"")))

(define (sp-plot-samples->file a path)
  (call-with-output-file path (l (file) (each (l (a) (display-line a file)) (sp-samples->list a)))))

(define (sp-plot-samples a . display-args)
  "samples [#:type #:color] -> unspecified
     for display-args see sp-plot-samples-display-file"
  (let
    (path (tmpnam))
    (sp-plot-samples->file a path) (apply sp-plot-samples-display-file path display-args)))

(define (sp-plot-spectrum-display-file path) (sp-plot-samples-display-file path #:type "histeps"))

(define (sp-plot-spectrum->file a path)
  "apply sp-spectrum on \"a\" and write the result to file at path"
  (call-with-output-file
    path (l (file) (vector-each (l (a) (display-line a file)) (sp-spectrum a)))))

(define (sp-plot-spectrum a)
  (let (path (tmpnam)) (sp-plot-spectrum->file a path) (sp-plot-spectrum-display-file path)))

; seq-parallel
; plot-samples
; plot-spectrum
; how to update event state
; how to update event list
; -- next
; noise-event
; cheap-noise-event
; event-group
; cached-event
; fm-synth-event
; asynth-event
; events->block
; block->file
; blocks->file
; sp-path-new*
; sp-event-with-resolution
