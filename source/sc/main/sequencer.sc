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

(define (sp-events-free events events-count) (void sp-event-t* sp-count-t)
  (declare i sp-count-t)
  (for ((set i 0) (< i events-count) (set i (+ 1 i)))
    (: (+ events i) free)))

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

(define (sp-synth-event-free a) (void sp-event-t*)
  (free (: (convert-type a:state sp-synth-event-state-t*) state))
  (free a:state))

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
  (set state 0)
  (status-require (sph-helper-malloc (* channel-count (sizeof sp-synth-event-state-t)) &state))
  (status-require (sp-synth-state-new channel-count config-len config &state:state))
  (memcpy state:config config (* config-len (sizeof sp-synth-partial-t)))
  (set
    state:config-len config-len
    e.start start
    e.end end
    e.f sp-synth-event-f
    e.free sp-synth-event-free
    e.state state)
  (set *out-event e)
  (label exit
    (if status-is-failure (free state))
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
  (declare
    s sp-noise-event-state-t*
    block-count sp-count-t
    i sp-count-t
    block-i sp-count-t
    channel-i sp-count-t
    t sp-count-t
    block-offset sp-count-t
    duration sp-count-t
    block-rest sp-count-t)
  (sc-comment "update filter arguments only every resolution number of samples")
  (set
    s event:state
    duration (- end start)
    block-count
    (if* (= duration s:resolution) 1
      (sp-cheap-floor-positive (/ duration s:resolution)))
    block-rest (modulo duration s:resolution))
  (sc-comment "total block count is block-count plus rest-block")
  (for ((set block-i 0) (<= block-i block-count) (set block-i (+ 1 block-i)))
    (set
      block-offset (* s:resolution block-i)
      t (+ start block-offset)
      duration
      (if* (= block-count block-i) block-rest
        s:resolution)
      s:random-state (sp-random s:random-state duration s:noise))
    (sp-windowed-sinc-bp-br
      s:noise
      duration
      (array-get s:cut-l t)
      (array-get s:cut-h t)
      (array-get s:trn-l t) (array-get s:trn-h t) s:is-reject &s:filter-state s:temp)
    (for ((set channel-i 0) (< channel-i out.channels) (set channel-i (+ 1 channel-i)))
      (for ((set i 0) (< i duration) (set i (+ 1 i)))
        (set (array-get (array-get out.samples channel-i) (+ block-offset i))
          (+
            (array-get (array-get out.samples channel-i) (+ block-offset i))
            (* (array-get (array-get s:amp channel-i) (+ block-offset i)) (array-get s:temp i))))))))

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
  "an event for noise filtered by a windowed-sinc filter.
  very processing intensive when parameters change with low resolution.
  memory for event.state will be allocated and then owned by the caller"
  status-declare
  (declare
    temp sp-sample-t*
    temp-noise sp-sample-t*
    ir-len sp-count-t
    e sp-event-t
    s sp-noise-event-state-t*)
  (set resolution
    (if* resolution (min resolution (- end start))
      96))
  (status-require (sph-helper-malloc (sizeof sp-noise-event-state-t) &s))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:noise))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:temp))
  (sc-comment
    "the result shows a small delay, circa 40 samples for transition 0.07. the size seems to be related to ir-len."
    "the state is initialised with one unused call to skip the delay."
    "an added benefit is that the filter-state setup malloc is checked")
  (set ir-len (sp-windowed-sinc-lp-hp-ir-length (min *trn-l *trn-h)))
  (status-require (sph-helper-malloc (* ir-len (sizeof sp-sample-t)) &temp))
  (status-require (sph-helper-malloc (* ir-len (sizeof sp-sample-t)) &temp-noise))
  (set
    random-state (sp-random random-state ir-len temp-noise)
    s:filter-state 0)
  (status-require
    (sp-windowed-sinc-bp-br
      temp-noise ir-len *cut-l *cut-h *trn-l *trn-h is-reject &s:filter-state temp))
  (free temp)
  (free temp-noise)
  (set
    s:cut-l cut-l
    s:cut-h cut-h
    s:trn-l trn-l
    s:trn-h trn-h
    s:is-reject is-reject
    s:resolution resolution
    s:random-state random-state
    s:amp amp
    e.start start
    e.end end
    e.f sp-noise-event-f
    e.free sp-noise-event-free
    e.state s)
  (set *out-event e)
  (label exit
    (return status)))

(declare sp-cheap-noise-event-state-t
  (type
    (struct
      (amp sp-sample-t**)
      (cut sp-sample-t*)
      (q-factor sp-sample-t)
      (passes sp-count-t)
      (filter-state (array sp-sample-t 2))
      (filter sp-state-variable-filter-t)
      (noise sp-sample-t*)
      (random-state sp-random-state-t)
      (resolution sp-count-t)
      (temp sp-sample-t*))))

(define (sp-cheap-noise-event-f start end out event)
  (void sp-count-t sp-count-t sp-block-t sp-event-t*)
  (declare
    s sp-cheap-noise-event-state-t*
    block-count sp-count-t
    i sp-count-t
    block-i sp-count-t
    channel-i sp-count-t
    t sp-count-t
    block-offset sp-count-t
    duration sp-count-t
    block-rest sp-count-t)
  (sc-comment "update filter arguments only every resolution number of samples")
  (set
    s event:state
    duration (- end start)
    block-count
    (if* (= duration s:resolution) 1
      (sp-cheap-floor-positive (/ duration s:resolution)))
    block-rest (modulo duration s:resolution))
  (sc-comment "total block count is block-count plus rest-block")
  (for ((set block-i 0) (<= block-i block-count) (set block-i (+ 1 block-i)))
    ;(out in in-count cutoff q-factor state)
    (set
      block-offset (* s:resolution block-i)
      t (+ start block-offset)
      duration
      (if* (= block-count block-i) block-rest
        s:resolution)
      s:random-state (sp-random s:random-state duration s:noise))
    (s:filter s:temp s:noise duration (array-get s:cut t) s:q-factor s:filter-state)
    (for ((set channel-i 0) (< channel-i out.channels) (set channel-i (+ 1 channel-i)))
      (for ((set i 0) (< i duration) (set i (+ 1 i)))
        (set (array-get (array-get out.samples channel-i) (+ block-offset i))
          (+
            (array-get (array-get out.samples channel-i) (+ block-offset i))
            (* (array-get (array-get s:amp channel-i) (+ block-offset i)) (array-get s:temp i))))))))

(define (sp-cheap-noise-event-free a) (void sp-event-t*)
  (define s sp-cheap-noise-event-state-t* a:state)
  (free s:noise)
  (free s:temp)
  (free a:state))

(define
  (sp-cheap-noise-event start end amp filter cut passes q-factor resolution random-state out-event)
  (status-t
    sp-count-t
    sp-count-t
    sp-sample-t**
    sp-state-variable-filter-t
    sp-sample-t* sp-count-t sp-sample-t sp-count-t sp-random-state-t sp-event-t*)
  "an event for noise filtered by a state-variable filter. multiple passes currently not implemented.
  lower processing costs even when parameters change with high resolution.
  multiple passes almost multiply performance costs.
  memory for event.state will be allocated and then owned by the caller"
  status-declare
  (declare
    e sp-event-t
    s sp-cheap-noise-event-state-t*)
  (set resolution
    (if* resolution (min resolution (- end start))
      96))
  (status-require (sph-helper-malloc (sizeof sp-cheap-noise-event-state-t) &s))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:noise))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:temp))
  (set
    (array-get s:filter-state 0) 0
    (array-get s:filter-state 1) 0
    s:cut cut
    s:q-factor q-factor
    s:passes passes
    s:resolution resolution
    s:random-state random-state
    s:amp amp
    s:filter filter
    e.start start
    e.end end
    e.f sp-cheap-noise-event-f
    e.free sp-cheap-noise-event-free
    e.state s)
  (set *out-event e)
  (label exit
    (return status)))