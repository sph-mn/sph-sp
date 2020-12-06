(sc-include-once "./sc-macros")

(define (sp-event-sort-swap a b c) (void void* ssize-t ssize-t)
  (declare d sp-event-t)
  (set
    d (array-get (convert-type a sp-event-t*) b)
    (array-get (convert-type a sp-event-t*) b) (array-get (convert-type a sp-event-t*) c)
    (array-get (convert-type a sp-event-t*) c) d))

(define (sp-event-sort-less? a b c) (uint8-t void* ssize-t ssize-t)
  (return
    (< (struct-get (array-get (convert-type a sp-event-t*) b) start)
      (struct-get (array-get (convert-type a sp-event-t*) c) start))))

(define (sp-seq-events-prepare data size) (void sp-event-t* sp-time-t)
  (quicksort sp-event-sort-less? sp-event-sort-swap data 0 (- size 1)))

(define (sp-seq start end out events size)
  (void sp-time-t sp-time-t sp-block-t sp-event-t* sp-time-t)
  "event arrays must have been prepared/sorted with sp_seq_event_prepare for seq to work correctly.
   event functions receive event relative start/end time, and output blocks begin at event start.
   as for paths, start is inclusive, end is exclusive, so that 0..100 and 100..200 attach seamless.
   $size is the number of events"
  (declare e-out-start sp-time-t e sp-event-t e-start sp-time-t e-end sp-time-t i sp-time-t)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set e (array-get events i))
    (cond ((<= e.end start) continue) ((<= end e.start) break)
      (else
        (set
          e-out-start (if* (> e.start start) (- e.start start) 0)
          e-start (if* (> start e.start) (- start e.start) 0)
          e-end (- (sp-min end e.end) e.start))
        (e.f e-start e-end (if* e-out-start (sp-block-with-offset out e-out-start) out) &e)))))

(define (sp-events-array-free events size) (void sp-event-t* sp-time-t)
  (declare i sp-time-t event-free (function-pointer void (struct sp-event-t*)))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set event-free (: (+ events i) free))
    (if event-free (event-free (+ events i)))))

(declare sp-seq-future-t
  (type
    (struct
      (start sp-time-t)
      (end sp-time-t)
      (out-start sp-time-t)
      (out sp-block-t)
      (event sp-event-t*)
      (future future-t))))

(define (sp-seq-parallel-future-f data) (void* void*)
  (define a sp-seq-future-t* data)
  (a:event:f a:start a:end a:out a:event))

(define (sp-seq-parallel start end out events size)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t* sp-time-t)
  "like sp_seq but evaluates events in parallel"
  status-declare
  (declare
    e-out-start sp-time-t
    e sp-event-t
    e-start sp-time-t
    e-end sp-time-t
    chn-i sp-channel-count-t
    seq-futures sp-seq-future-t*
    sf sp-seq-future-t*
    i sp-time-t
    ftr-i sp-time-t
    active-count sp-time-t)
  (set seq-futures 0 active-count 0)
  (sc-comment "count events to allocate future object memory")
  (for ((set i 0) (< i size) (set+ i 1))
    (set e (array-get events i))
    (cond ((<= e.end start) continue) ((<= end e.start) break) (else (set+ active-count 1))))
  (status-require (sph-helper-malloc (* active-count (sizeof sp-seq-future-t)) &seq-futures))
  (sc-comment "parallelise")
  (set ftr-i 0)
  (for ((set i 0) (< i size) (set+ i 1))
    (set e (array-get events i))
    (cond ((<= e.end start) continue) ((<= end e.start) break)
      (else
        (set
          sf (+ seq-futures ftr-i)
          e-out-start (if* (> e.start start) (- e.start start) 0)
          e-start (if* (> start e.start) (- start e.start) 0)
          e-end (- (sp-min end e.end) e.start))
        (set+ ftr-i 1) (status-require (sp-block-new out.channels (- e-end e-start) &sf:out))
        (set sf:start e-start sf:end e-end sf:out-start e-out-start sf:event (+ i events))
        (future-new sp-seq-parallel-future-f sf &sf:future))))
  (sc-comment "merge")
  (for ((set ftr-i 0) (< ftr-i active-count) (set+ ftr-i 1))
    (set sf (+ ftr-i seq-futures))
    (touch &sf:future)
    (for ((set chn-i 0) (< chn-i out.channels) (set chn-i (+ 1 chn-i)))
      (for ((set i 0) (< i sf:out.size) (set i (+ 1 i)))
        (set+ (array-get (array-get out.samples chn-i) (+ sf:out-start i))
          (array-get (array-get sf:out.samples chn-i) i))))
    (sp-block-free sf:out))
  (label exit (free seq-futures) status-return))

(define (sp-wave-event-state-1 wave-state) (sp-wave-event-state-t sp-wave-state-t)
  (declare a sp-wave-event-state-t)
  (set a.channels 1)
  (array-set* a.wave-states wave-state)
  (return a))

(define (sp-wave-event-state-2 wave-state-1 wave-state-2)
  (sp-wave-event-state-t sp-wave-state-t sp-wave-state-t)
  (declare a sp-wave-event-state-t)
  (set a.channels 2)
  (array-set* a.wave-states wave-state-1 wave-state-2)
  (return a))

(define (sp-wave-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (declare chn-i sp-channel-count-t state sp-wave-event-state-t* wave-state sp-wave-state-t*)
  (set state (convert-type event:state sp-wave-event-state-t*))
  (for ((set chn-i 0) (< chn-i state:channels) (set chn-i (+ 1 chn-i)))
    (set wave-state (+ state:wave-states chn-i))
    (if (not wave-state:amp) continue)
    (sp-wave start (- end start) wave-state (array-get out.samples chn-i))))

(define (sp-wave-event-free a) (void sp-event-t*) (free a:state) (sp-event-memory-free a))

(define (sp-wave-event start end state out)
  (status-t sp-time-t sp-time-t sp-wave-event-state-t sp-event-t*)
  "in wave_event_state, unset wave states or wave states with amp set to null will generate no output.
   the state struct is copied and freed with event.free so that stack declared structs can be used.
   sp_wave_event, sp_wave_event_f and sp_wave_event_free are a good example for custom events"
  status-declare
  (declare event-state sp-wave-state-t*)
  (set event-state 0)
  (status-require (sph-helper-calloc (sizeof sp-wave-event-state-t) &event-state))
  (memcpy event-state &state (sizeof sp-wave-event-state-t))
  (set
    out:start start
    out:end end
    out:f sp-wave-event-f
    out:free sp-wave-event-free
    out:state event-state)
  (label exit (if status-is-failure (free event-state)) status-return))

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
      (resolution sp-time-t)
      (temp sp-sample-t*)
      (trn-h sp-sample-t*)
      (trn-l sp-sample-t*))))

(define (sp-noise-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (declare
    s sp-noise-event-state-t*
    block-count sp-time-t
    i sp-time-t
    block-i sp-time-t
    chn-i sp-time-t
    amp sp-sample-t**
    t sp-time-t
    block-offset sp-time-t
    duration sp-time-t
    block-rest sp-time-t)
  (sc-comment "update filter arguments only every resolution number of samples")
  (set
    s event:state
    duration (- end start)
    block-count (if* (= duration s:resolution) 1 (sp-cheap-floor-positive (/ duration s:resolution)))
    block-rest (modulo duration s:resolution))
  (for ((set block-i 0) (<= block-i block-count) (set block-i (+ 1 block-i)))
    (set
      block-offset (* s:resolution block-i)
      t (+ start block-offset)
      duration (if* (= block-count block-i) block-rest s:resolution))
    (sp-samples-random &s:random-state duration s:noise)
    (sp-windowed-sinc-bp-br s:noise duration
      (array-get s:cut-l t) (array-get s:cut-h t) (array-get s:trn-l t)
      (array-get s:trn-h t) s:is-reject &s:filter-state s:temp)
    (for ((set chn-i 0) (< chn-i out.channels) (set chn-i (+ 1 chn-i)))
      (set amp (+ s:amp chn-i))
      (if (not amp) continue)
      (for ((set i 0) (< i duration) (set i (+ 1 i)))
        (set+ (array-get out.samples chn-i (+ block-offset i))
          (* (array-get *amp (+ t i)) (array-get s:temp i)))))))

(define (sp-noise-event-free a) (void sp-event-t*)
  (define s sp-noise-event-state-t* a:state)
  (free s:noise)
  (free s:temp)
  (sp-convolution-filter-state-free s:filter-state)
  (free a:state)
  (sp-event-memory-free a))

(define
  (sp-noise-event start end amp cut-l cut-h trn-l trn-h is-reject resolution random-state out-event)
  (status-t sp-time-t sp-time-t sp-sample-t** sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* uint8-t sp-time-t sp-random-state-t sp-event-t*)
  "an event for noise filtered by a windowed-sinc filter.
   very processing intensive if parameters change with low resolution.
   memory for event.state will be allocated and then owned by the caller"
  status-declare
  (declare temp sp-sample-t* temp-noise sp-sample-t* ir-len sp-time-t s sp-noise-event-state-t*)
  (sp-declare-event e)
  (set resolution (if* resolution (sp-min resolution (- end start)) 96))
  (status-require (sph-helper-malloc (sizeof sp-noise-event-state-t) &s))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:noise))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:temp))
  (sc-comment
    "the result shows a small delay, circa 40 samples for transition 0.07. the size seems to be related to ir-len."
    "the state is initialised with one unused call to skip the delay."
    "an added benefit is that the filter-state setup malloc is checked")
  (set ir-len (sp-windowed-sinc-lp-hp-ir-length (sp-min *trn-l *trn-h)) s:filter-state 0)
  (status-require (sph-helper-malloc (* ir-len (sizeof sp-sample-t)) &temp))
  (status-require (sph-helper-malloc (* ir-len (sizeof sp-sample-t)) &temp-noise))
  (sp-samples-random &random-state ir-len temp-noise)
  (status-require
    (sp-windowed-sinc-bp-br temp-noise ir-len
      *cut-l *cut-h *trn-l *trn-h is-reject &s:filter-state temp))
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
  (label exit status-return))

(declare sp-cheap-noise-event-state-t
  (type
    (struct
      (amp sp-sample-t**)
      (cut sp-sample-t*)
      (q-factor sp-sample-t)
      (passes sp-time-t)
      (filter-state sp-cheap-filter-state-t)
      (type sp-state-variable-filter-t)
      (noise sp-sample-t*)
      (random-state sp-random-state-t)
      (resolution sp-time-t)
      (temp sp-sample-t*))))

(define (sp-cheap-noise-event-f start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (declare
    amp sp-sample-t**
    block-count sp-time-t
    block-i sp-time-t
    block-offset sp-time-t
    block-rest sp-time-t
    chn-i sp-time-t
    duration sp-time-t
    i sp-time-t
    s sp-cheap-noise-event-state-t*
    t sp-time-t)
  (sc-comment "update filter arguments only every resolution number of samples")
  (set
    s event:state
    duration (- end start)
    block-count (if* (= duration s:resolution) 1 (sp-cheap-floor-positive (/ duration s:resolution)))
    block-rest (modulo duration s:resolution))
  (sc-comment "total block count is block-count plus rest-block")
  (for ((set block-i 0) (<= block-i block-count) (set block-i (+ 1 block-i)))
    (set
      block-offset (* s:resolution block-i)
      t (+ start block-offset)
      duration (if* (= block-count block-i) block-rest s:resolution))
    (sp-samples-random &s:random-state duration s:noise)
    (sp-cheap-filter s:type s:noise
      duration (array-get s:cut t) s:passes s:q-factor &s:filter-state s:temp)
    (for ((set chn-i 0) (< chn-i out.channels) (set chn-i (+ 1 chn-i)))
      (set amp (+ s:amp chn-i))
      (if (not amp) continue)
      (for ((set i 0) (< i duration) (set i (+ 1 i)))
        (set+ (array-get out.samples chn-i (+ block-offset i))
          (* (array-get *amp (+ t i)) (array-get s:temp i)))))))

(define (sp-cheap-noise-event-free a) (void sp-event-t*)
  (define s sp-cheap-noise-event-state-t* a:state)
  (free s:noise)
  (free s:temp)
  (sp-cheap-filter-state-free &s:filter-state)
  (free a:state)
  (sp-event-memory-free a))

(define
  (sp-cheap-noise-event start end amp type cut passes q-factor resolution random-state out-event)
  (status-t sp-time-t sp-time-t sp-sample-t** sp-state-variable-filter-t sp-sample-t* sp-time-t sp-sample-t sp-time-t sp-random-state-t sp-event-t*)
  "an event for noise filtered by a state-variable filter.
   lower processing costs even when parameters change with high resolution.
   multiple passes almost multiply performance costs.
   memory for event.state will be allocated and then owned by the caller"
  status-declare
  (declare s sp-cheap-noise-event-state-t*)
  (sp-declare-event e)
  (set resolution (if* resolution (sp-min resolution (- end start)) 96))
  (status-require (sph-helper-malloc (sizeof sp-cheap-noise-event-state-t) &s))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:noise))
  (status-require (sph-helper-malloc (* resolution (sizeof sp-sample-t)) &s:temp))
  (status-require (sp-cheap-filter-state-new resolution passes &s:filter-state))
  (set
    s:cut cut
    s:q-factor q-factor
    s:passes passes
    s:resolution resolution
    s:random-state random-state
    s:amp amp
    s:type type
    e.start start
    e.end end
    e.f sp-cheap-noise-event-f
    e.free sp-cheap-noise-event-free
    e.state s)
  (set *out-event e)
  (label exit status-return))

(pre-define (sp-group-event-free-events events end)
  (begin
    "to free events while the group itself is not finished"
    (while (and (array4-in-range events) (<= (struct-get (array4-get events) end) end))
      (if (struct-get (array4-get events) free)
        ((struct-get (array4-get events) free) (array4-get-address events)))
      (array4-forward events))))

(define (sp-group-event-free a) (void sp-event-t*)
  "events.current is used to track freed events"
  (define s sp-events-t (pointer-get (convert-type a:state sp-events-t*)))
  (while (array4-in-range s)
    (if (struct-get (array4-get s) free) ((struct-get (array4-get s) free) (array4-get-address s)))
    (array4-forward s))
  (array4-free s)
  (free a:state)
  (sp-event-memory-free a))

(define (sp-group-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  "free past events early, they might be sub-group trees"
  (define s sp-events-t* event:state)
  (sp-seq start end out (array4-get-address *s) (array4-right-size *s))
  (sp-group-event-free-events *s end))

(define (sp-group-event-parallel-f start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  "can be used in place of sp-group-event-f.
   seq-parallel can fail if there is not enough memory, but this is ignored currently"
  (define s sp-events-t* event:state)
  (sp-seq-parallel start end out (array4-get-address *s) (array4-right-size *s))
  (sp-group-event-free-events *s end))

(define (sp-group-new start event-size out) (status-t sp-time-t sp-group-size-t sp-event-t*)
  status-declare
  (declare s sp-events-t*)
  (memreg-init 2)
  (status-require (sph-helper-malloc (sizeof sp-events-t) &s))
  (memreg-add s)
  (if (sp-events-new event-size s) sp-memory-error)
  (memreg-add s:data)
  (struct-set *out state s start start end start f sp-group-event-f free sp-group-event-free)
  (label exit (if status-is-failure memreg-free) (return status)))

(define (sp-group-append a event) (void sp-event-t* sp-event-t)
  (set+ event.start a:end event.end a:end)
  (sp-group-add *a event))

(define (sp-event-memory-free event) (void sp-event-t*)
  (declare i sp-time-half-t m sp-memory-t)
  (for ((set i 0) (< i event:memory-used) (set+ i 1))
    (set m (array-get event:memory i))
    (m.free m.data))
  (free event:memory))

(define (sp-event-array-set-null a size) (void sp-event-t* sp-time-t)
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1)) (sp-event-set-null (array-get a i))))

(declare
  sp-map-event-f-t
  (type (function-pointer void sp-time-t sp-time-t sp-block-t sp-block-t sp-event-t* void*))
  sp-map-event-state-t (type (struct (event sp-event-t) (f sp-map-event-f-t) (state void*))))

(define (sp-map-event-free a) (void sp-event-t*)
  (declare s sp-map-event-state-t*)
  (set s a:state)
  (if s:event.free (s:event.free &s:event))
  (free a:state))

(define (sp-map-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  "creates temporary output, lets event write to it, and passes the result to a user function"
  (declare s sp-map-event-state-t*)
  (set s event:state)
  (s:event.f start end out &s:event)
  (s:f start end out out &s:event s:state))

(define (sp-map-event-isolated-f start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  "creates temporary output, lets event write to it, and passes the result to a user function"
  (declare s sp-map-event-state-t* temp-out sp-block-t)
  status-declare
  (set status (sp-block-new out.channels out.size &temp-out))
  (if status-is-failure return)
  (set s event:state)
  (s:event.f start end temp-out &s:event)
  (s:f start end temp-out out &s:event s:state)
  (sp-block-free temp-out))

(define (sp-map-event event f state isolate out)
  (status-t sp-event-t sp-map-event-f-t void* uint8-t sp-event-t*)
  "f: function(start end sp_block_t:in sp_block_t:out sp_event_t*:event void*:state)
   isolate: use a dedicated output buffer for event
     events can be wrapped in multiple sp_map_event with an isolated sp_map_event on top that
     finally writes to main out to mix with other events
   use cases: filter chains, post processing"
  status-declare
  (declare s sp-map-event-state-t*)
  (sp-declare-event e)
  (status-require (sph-helper-malloc (sizeof sp-map-event-state-t) &s))
  (set
    s:event event
    s:state state
    s:f f
    e.state s
    e.start event.start
    e.end event.end
    e.f (if* isolate sp-map-event-isolated-f sp-map-event-f)
    e.free sp-map-event-free)
  (set *out e)
  (label exit status-return))