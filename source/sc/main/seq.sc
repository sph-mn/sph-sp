(define (sp-event-sort-swap a b) (void void* void*)
  (declare c sp-event-t)
  (set
    c (pointer-get (convert-type a sp-event-t*))
    (pointer-get (convert-type b sp-event-t*)) (pointer-get (convert-type a sp-event-t*))
    (pointer-get (convert-type b sp-event-t*)) c))

(define (sp-event-sort-less? a b) (uint8-t void* void*)
  (return (< (: (convert-type a sp-event-t*) start) (: (convert-type b sp-event-t*) start))))

(define (sp-seq-events-prepare a size) (void sp-event-t* sp-count-t)
  (quicksort sp-event-sort-less? sp-event-sort-swap (sizeof sp-event-t) a size))

(define (sp-seq start end out out-start events events-size)
  (void sp-count-t sp-count-t sp-block-t sp-count-t sp-event-t* sp-count-t)
  "event arrays must have been prepared/sorted with sp-seq-event-prepare for seq to work correctly"
  (declare
    e-out-start sp-count-t
    e sp-event-t
    e-start sp-count-t
    e-end sp-count-t
    i sp-count-t)
  (if out-start (set out (sp-block-with-offset out out-start)))
  (for ((set i 0) (< i events-size) (set i (+ 1 i)))
    (set e (array-get events i))
    (cond
      ((<= e.end start) continue)
      ((<= end e.start) break)
      (else
        (set
          e-out-start
          (if* (> e.start start) (- e.start start)
            0)
          e-start
          (if* (> start e.start) (- start e.start)
            0)
          e-end
          (-
            (if* (< e.end end) e.end
              end)
            e.start))
        (e.f
          e-start e-end
          (if* e-out-start (sp-block-with-offset out e-out-start)
            out)
          &e)))))

(declare sp-seq-future-t
  (type
    (struct
      (start sp-count-t)
      (end sp-count-t)
      (out-start sp-count-t)
      (out sp-block-t)
      (event sp-event-t*)
      (future future-t))))

(define (sp-seq-parallel-future-f data) (void* void*)
  (define a sp-seq-future-t* data)
  (a:event:f a:start a:end a:out a:event))

(define (sp-seq-parallel start end out out-start events events-size)
  (status-t sp-count-t sp-count-t sp-block-t sp-count-t sp-event-t* sp-count-t)
  "like sp_seq but evaluates events in parallel"
  status-declare
  (declare
    e-out-start sp-count-t
    e sp-event-t
    e-start sp-count-t
    e-end sp-count-t
    channel-i sp-channel-count-t
    events-start sp-count-t
    events-count sp-count-t
    seq-futures sp-seq-future-t*
    sf sp-seq-future-t*
    i sp-count-t
    e-i sp-count-t)
  (set seq-futures 0)
  (if out-start (set out (sp-block-with-offset out out-start)))
  (sc-comment "select active events")
  (for
    ( (set
        i 0
        events-start 0
        events-count 0)
      (< i events-size) (set i (+ 1 i)))
    (set e (array-get events i))
    (cond
      ((<= e.end start) (set events-start (+ 1 events-start)))
      ((<= end e.start) break) (else (set events-count (+ 1 events-count)))))
  (status-require (sph-helper-malloc (* events-count (sizeof sp-seq-future-t)) &seq-futures))
  (sc-comment "parallelise")
  (for ((set i 0) (< i events-count) (set i (+ 1 i)))
    (set
      e (array-get events (+ events-start i))
      sf (+ i seq-futures))
    (set
      e-out-start
      (if* (> e.start start) (- e.start start)
        0)
      e-start
      (if* (> start e.start) (- start e.start)
        0)
      e-end
      (-
        (if* (< e.end end) e.end
          end)
        e.start))
    (status-require (sp-block-new out.channels (- e-end e-start) &sf:out))
    (set
      sf:start e-start
      sf:end e-end
      sf:out-start e-out-start
      sf:event (+ events-start i events))
    (future-new sp-seq-parallel-future-f sf &sf:future))
  (sc-comment "merge")
  (for ((set e-i 0) (< e-i events-count) (set e-i (+ 1 e-i)))
    (set sf (+ e-i seq-futures))
    (touch &sf:future)
    (for ((set channel-i 0) (< channel-i out.channels) (set channel-i (+ 1 channel-i)))
      (for ((set i 0) (< i sf:out.size) (set i (+ 1 i)))
        (set (array-get (array-get out.samples channel-i) (+ sf:out-start i))
          (+
            (array-get (array-get out.samples channel-i) (+ sf:out-start i))
            (array-get (array-get sf:out.samples channel-i) i)))))
    (sp-block-free sf:out))
  (label exit
    (free seq-futures)
    (return status)))

(define (sp-synth-event-f start end out event) (void sp-count-t sp-count-t sp-block-t sp-event-t*)
  (define s sp-synth-event-state-t* event:state)
  (sp-synth out start (- end start) s:config-len s:config s:state))

(define (sp-synth-event start end channel-count config-len config out-event)
  (status-t sp-count-t sp-count-t sp-count-t sp-count-t sp-synth-partial-t* sp-event-t*)
  "memory for event.state will be allocated and then owned by the caller.
  config is copied into event.state"
  status-declare
  (declare
    e sp-event-t
    state sp-synth-event-state-t*)
  (status-require (sph-helper-malloc (* channel-count (sizeof sp-synth-event-state-t)) &state))
  (status-require (sp-synth-state-new channel-count config-len config &state:state))
  (memcpy state:config config (* config-len (sizeof sp-synth-partial-t)))
  (set
    state:config-len config-len
    e.start start
    e.end end
    e.f sp-synth-event-f
    e.state state)
  (set *out-event e)
  (label exit
    (return status)))

(declare sp-noise-event-state-t
  (type
    (struct
      (amp sp-sample-t**)
      (cut-h sp-sample-t*)
      (cut-l sp-sample-t*)
      (filter-state sp-convolution-filter-state-t*)
      (is-reject uint8-t)
      (noise sp-sample-t*)
      (random-state sp-random-state-t)
      (resolution sp-count-t)
      (temp sp-sample-t*)
      (trn-h sp-sample-t*)
      (trn-l sp-sample-t*))))

(define (sp-noise-event-f start end out event) (void sp-count-t sp-count-t sp-block-t sp-event-t*)
  (define s sp-noise-event-state-t* event:state)
  (set s:random-state (sp-random s:random-state (- end start) s:noise))
  ; todo: map with resolution, apply amps when copying into out
  (sp-windowed-sinc-bp-br
    s:noise s:resolution *s:cut-l *s:cut-h *s:trn-l *s:trn-h s:is-reject &s:filter-state s:temp))

(define (sp-noise-event-free a) (void sp-event-t*)
  (define s sp-noise-event-state-t* a:state)
  (free s:noise)
  (free s:temp)
  (sp-convolution-filter-state-free s:filter-state)
  (free a:state))

(define
  (sp-noise-event start end amp cut-l cut-h trn-l trn-h is-reject resolution random-state out-event)
  (status-t
    sp-count-t
    sp-count-t
    sp-sample-t**
    sp-sample-t*
    sp-sample-t* sp-sample-t* sp-sample-t* uint8-t sp-count-t sp-random-state-t sp-event-t*)
  "memory for event.state will be allocated and then owned by the caller"
  status-declare
  (declare
    e sp-event-t
    s sp-noise-event-state-t*)
  (status-require (sph-helper-malloc (sizeof sp-noise-event-state-t) &s))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:noise))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:temp))
  (set
    s:cut-l cut-l
    s:cut-h cut-h
    s:trn-l trn-l
    s:trn-h trn-h
    s:is-reject is-reject
    s:resolution resolution
    s:filter-state 0
    s:random-state random-state
    s:amp amp
    e.start start
    e.end end
    e.f sp-noise-event-f
    e.state s)
  (set *out-event e)
  (label exit
    (return status)))