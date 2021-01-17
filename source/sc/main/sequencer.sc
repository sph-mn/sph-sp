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

(define (sp-seq start end out events) (status-t sp-time-t sp-time-t sp-block-t sp-events-t*)
  "event arrays must have been prepared/sorted with sp_seq_event_prepare for seq to work correctly.
   like for paths, start is inclusive, end is exclusive, so that 0..100 and 100..200 attach seamless.
   events can have three function pointers: prepare, generate and free.
   generate receives event relative start/end times, and output blocks that start at the current block.
   prepare will only be called once and the function pointer will be set to zero afterwards.
   if prepare fails, sp_seq returns, and events.current will be the event that produced the error.
   events.current is the first index after past events"
  status-declare
  (declare e sp-event-t ep sp-event-t* i sp-time-t)
  (for ((set i events:current) (< i events:used) (set+ i 1))
    (set ep (+ events:data i) e *ep)
    (cond ((<= e.end start) (e.free ep) (array4-forward *events)) ((<= end e.start) break)
      ( (> e.end start)
        (if e.prepare
          (begin (status-require-return (e.prepare ep)) (set ep:prepare 0 e.state ep:state)))
        (e.generate (if* (> start e.start) (- start e.start) 0) (- (sp-min end e.end) e.start)
          (if* (> e.start start) (sp-block-with-offset out (- e.start start)) out) e.state))))
  status-return)

(define (sp-events-free events) (void sp-events-t*)
  "free all events starting from the current event. only needed if sp_seq will not further process, and thereby free, the events"
  (declare a sp-events-t f (function-pointer void sp-event-t*))
  (set a *events)
  (while (array4-in-range a)
    (set f (struct-get (array4-get a) free))
    (if f (f (+ a.data a.current)))
    (array4-forward a)))

(declare sp-seq-future-t
  (type
    (struct
      (start sp-time-t)
      (end sp-time-t)
      (out-start sp-time-t)
      (out sp-block-t)
      (event sp-event-t*)
      (status status-t)
      (future future-t))))

(define (sp-seq-parallel-future-f data) (void* void*)
  (define a sp-seq-future-t* data)
  (if a:event:prepare
    (begin
      (set a:status (a:event:prepare a:event))
      (if (= status-id-success a:status.id) (set a:event:prepare 0) (return 0))))
  (a:event:generate a:start a:end a:out a:event:state)
  (return 0))

(define (sp-seq-parallel start end out events)
  (status-t sp-time-t sp-time-t sp-block-t sp-events-t*)
  "like sp_seq but evaluates events with multiple threads in parallel.
   there is some overhead, as each event gets its own output block"
  status-declare
  (declare
    active sp-time-t
    allocated sp-time-t
    ci sp-channel-count-t
    e-end sp-time-t
    ep sp-event-t*
    e sp-event-t
    e-start sp-time-t
    i sp-time-t
    sf-array sp-seq-future-t*
    sf-i sp-time-t
    sf sp-seq-future-t*)
  (set sf-array 0 sf-i 0 active 0)
  (for ((set i events:current) (< i events:used) (set+ i 1))
    (set ep (+ events:data i))
    (cond ((<= end ep:start) break) ((> ep:end start) (set+ active 1))))
  (status-require (sph-helper-malloc (* active (sizeof sp-seq-future-t)) &sf-array))
  (sc-comment "distribute")
  (for ((set i events:current) (< i events:used) (set+ i 1))
    (set ep (+ events:data i) e *ep)
    (cond ((<= e.end start) (if e.free (e.free ep)) (array4-forward *events))
      ((<= end e.start) break)
      ( (> e.end start)
        (set
          sf (+ sf-array sf-i)
          e-start (if* (> start e.start) (- start e.start) 0)
          e-end (- (sp-min end e.end) e.start)
          sf:start e-start
          sf:end e-end
          sf:out-start (if* (> e.start start) (- e.start start) 0)
          sf:event (+ events:data i)
          sf:status.id status-id-success)
        (status-require (sp-block-new out.channels (- e-end e-start) &sf:out))
        (set+ allocated 1 sf-i 1) (future-new sp-seq-parallel-future-f sf &sf:future))))
  (sc-comment "merge")
  (for ((set sf-i 0) (< sf-i active) (set+ sf-i 1))
    (set sf (+ sf-array sf-i))
    (touch &sf:future)
    (status-require sf:status)
    (for ((set ci 0) (< ci out.channels) (set+ ci 1))
      (for ((set i 0) (< i sf:out.size) (set+ i 1))
        (set+ (array-get (array-get out.samples ci) (+ sf:out-start i))
          (array-get (array-get sf:out.samples ci) i)))))
  (label exit
    (if sf-array
      (begin
        (for ((set i 0) (< i allocated) (set+ i 1))
          (sp-block-free (struct-get (array-get sf-array i) out)))
        (free sf-array)))
    status-return))

(define (sp-wave-event-free a) (void sp-event-t*) (free a:state) (sp-event-memory-free a))

(define (sp-wave-event-generate start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (declare i sp-time-t state-pointer sp-wave-event-state-t* state sp-wave-event-state-t)
  (set state-pointer (convert-type event:state sp-wave-event-state-t*) state *state-pointer)
  (for ((set i 0) (< i (- end start)) (set+ i 1))
    (set+ (array-get out state.config.channel-index i)
      (* (array-get state.config.amod (+ start i)) (array-get state.config.wvf state.config.phs))
      state.config.phs (sp-modvalue state.config.frq state.config.fmod mod-index))
    (if (>= state.config.phs state.config.wvf-size)
      (set state.config.phs (modulo state.config.phs state.config.wvf-size))))
  (set state-pointer:config.phs state.config.phs))

(define (sp-wave-event start end config out)
  (status-t sp-time-t sp-time-t sp-wave-event-config-t sp-event-t*)
  "create an event playing waveforms from an array.
   channel config:
   * settings override the main settings if not 0
   * amod multiplies main amod
   * event end will be longer if delay is used
   if modulation arrays are null, then the fixed value will be used (frq, channel amp)"
  status-declare
  (declare state sp-wave-event-state-t*)
  (status-require (sph-helper-calloc (sizeof sp-wave-event-state-t) &state))
  (set
    state:config config
    out:start start
    out:end delayed-end
    out:f sp-wave-event-generate
    out:free sp-wave-event-free
    out:state state)
  (label exit status-return))

(declare sp-noise-event-state-t
  (type
    (struct
      (config sp-noise-event-config-t)
      (filter-state sp-convolution-filter-state-t*)
      (noise sp-sample-t*)
      (temp sp-sample-t*))))

(define (sp-noise-event-free a) (void sp-event-t*)
  (define s sp-noise-event-state-t* a:state)
  (free s:noise)
  (free s:temp)
  (sp-convolution-filter-state-free s:filter-state)
  (free a:state)
  (sp-event-memory-free a))

(define (sp-noise-event-generate start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (declare
    amod sp-sample-t*
    block-count sp-time-t
    block-i sp-time-t
    block-offset sp-time-t
    block-rest sp-time-t
    chn-i sp-time-t
    duration sp-time-t
    i sp-time-t
    s sp-noise-event-state-t
    t sp-time-t)
  (sc-comment "update filter arguments only every resolution number of samples")
  (set
    s (pointer-get (convert-type event:state sp-noise-event-state-t*))
    duration (- end start)
    block-count
    (if* (= duration s.config.resolution) 1
      (sp-cheap-floor-positive (/ duration s.config.resolution)))
    block-rest (modulo duration s.config.resolution))
  (for ((set block-i 0) (<= block-i block-count) (set block-i (+ 1 block-i)))
    (set
      block-offset (* s.config.resolution block-i)
      t (+ start block-offset)
      duration (if* (= block-count block-i) block-rest s.config.resolution))
    (sp-samples-random &s.config.random-state duration s.noise)
    (sp-windowed-sinc-bp-br s.noise duration
      (sp-modvalue s.config.cutl s.config.cutl-mod t) (sp-modvalue s.config.cuth s.config.cuth-mod t)
      (sp-modvalue s.config.trnl s.config.trnl-mod t) (sp-modvalue s.config.trnh s.config.trnh-mod t)
      s.config.is-reject &s.filter-state s.temp)
    (for ((set chn-i 0) (< chn-i out.channels) (set+ chn-i 1))
      (set amod (array-get s.config.amod chn-i))
      (if (not amod) continue)
      (for ((set i 0) (< i duration) (set+ i 1))
        (set+ (array-get out.samples chn-i (+ block-offset i))
          (* (array-get amod (+ t i)) (array-get s.temp i)))))))

(define (sp-noise-event start end config out-event)
  (status-t sp-time-t sp-time-t sp-noise-event-config-t sp-event-t*)
  "an event for noise filtered by a windowed-sinc filter.
   very processing intensive if parameters change with low resolution.
   memory for event.state will be allocated and then owned by the caller"
  status-declare
  (declare
    temp sp-sample-t*
    temp-noise sp-sample-t*
    ir-len sp-time-t
    state sp-noise-event-state-t*
    trnl sp-sample-t
    trnh sp-sample-t)
  (sp-declare-event event)
  (set
    config.resolution (sp-min config.resolution (- end start))
    trnl (sp-modvalue config.trnl config.trnl-mod 0)
    trnh (sp-modvalue config.trnh config.trnh-mod 0))
  (status-require (sph-helper-malloc (sizeof sp-noise-event-state-t) &state))
  (status-require (sph-helper-malloc (* config.resolution (sizeof sp-sample-t)) &state:noise))
  (status-require (sph-helper-malloc (* config.resolution (sizeof sp-sample-t)) &state:temp))
  (sc-comment
    "the result shows a small delay, circa 40 samples for transition 0.07. the size seems to be related to ir-len."
    "the state is initialised with one unused call to skip the delay."
    "an added benefit is that the filter-state setup malloc is checked")
  (set ir-len (sp-windowed-sinc-lp-hp-ir-length (sp-min trnl trnh)) state:filter-state 0)
  (status-require (sph-helper-malloc (* ir-len (sizeof sp-sample-t)) &temp))
  (status-require (sph-helper-malloc (* ir-len (sizeof sp-sample-t)) &temp-noise))
  (sp-samples-random &config.random-state ir-len temp-noise)
  (status-require
    (sp-windowed-sinc-bp-br temp-noise ir-len
      (sp-modvalue config.cutl config.cutl-mod 0) (sp-modvalue config.cuth config.cuth-mod 0) trnl
      trnh config.is-reject &state:filter-state temp))
  (free temp)
  (free temp-noise)
  (set
    state:config config
    event.start start
    event.end end
    event.generate sp-noise-event-generate
    event.free sp-noise-event-free
    event.state state
    *out-event event)
  (label exit status-return))

(declare sp-cheap-noise-event-state-t
  (type
    (struct
      (config sp-cheap-noise-event-config-t)
      (filter-state sp-cheap-filter-state-t)
      (noise sp-sample-t*)
      (temp sp-sample-t*))))

(define (sp-cheap-noise-event-generate start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (declare
    amod sp-sample-t*
    block-count sp-time-t
    block-i sp-time-t
    block-offset sp-time-t
    block-rest sp-time-t
    chn-i sp-time-t
    duration sp-time-t
    i sp-time-t
    s sp-cheap-noise-event-state-t
    t sp-time-t)
  (sc-comment "update filter arguments only every resolution number of samples")
  (set s (pointer-get (convert-type event:state sp-cheap-noise-event-state-t*)))
  (if (not s.config.resolution) (exit 1))
  (set
    duration (- end start)
    block-count
    (if* (= duration s.config.resolution) 1
      (sp-cheap-floor-positive (/ duration s.config.resolution)))
    block-rest (modulo duration s.config.resolution))
  (sc-comment "total block count is block-count plus rest-block")
  (for ((set block-i 0) (<= block-i block-count) (set block-i (+ 1 block-i)))
    (set
      block-offset (* s.config.resolution block-i)
      t (+ start block-offset)
      duration (if* (= block-count block-i) block-rest s.config.resolution))
    (sp-samples-random &s.config.random-state duration s.noise)
    (sp-cheap-filter s.config.type s.noise
      duration (sp-modvalue s.config.cut s.config.cut-mod t) s.config.passes
      s.config.q-factor &s.filter-state s.temp)
    (for ((set chn-i 0) (< chn-i out.channels) (set chn-i (+ 1 chn-i)))
      (set amod (array-get s.config.amod chn-i))
      (if (not amod) continue)
      (for ((set i 0) (< i duration) (set i (+ 1 i)))
        (set+ (array-get out.samples chn-i (+ block-offset i))
          (* (array-get amod (+ t i)) (array-get s.temp i)))))))

(define (sp-cheap-noise-event-free a) (void sp-event-t*)
  (define s sp-cheap-noise-event-state-t* a:state)
  (free s:noise)
  (free s:temp)
  (sp-cheap-filter-state-free &s:filter-state)
  (free a:state)
  (sp-event-memory-free a))

(define (sp-cheap-noise-event start end config out-event)
  (status-t sp-time-t sp-time-t sp-cheap-noise-event-config-t sp-event-t*)
  "an event for noise filtered by a state-variable filter.
   lower processing costs even when parameters change with high resolution.
   multiple passes almost multiply performance costs.
   memory for event.state will be allocated and then owned by the caller"
  status-declare
  (declare state sp-cheap-noise-event-state-t*)
  (sp-declare-event event)
  (set config.resolution (sp-min config.resolution (- end start)))
  (status-require (sph-helper-malloc (sizeof sp-cheap-noise-event-state-t) &state))
  (status-require (sph-helper-malloc (* config.resolution (sizeof sp-sample-t)) &state:noise))
  (status-require (sph-helper-malloc (* config.resolution (sizeof sp-sample-t)) &state:temp))
  (status-require (sp-cheap-filter-state-new config.resolution config.passes &state:filter-state))
  (set
    state:config config
    event.start start
    event.end end
    event.generate sp-cheap-noise-event-generate
    event.free sp-cheap-noise-event-free
    event.state state)
  (set *out-event event)
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

(define (sp-group-event-generate start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  "free past events early, they might be sub-group trees"
  (define s sp-events-t* event:state)
  (sp-seq start end out (array4-get-address *s) (array4-right-size *s))
  (sp-group-event-free-events *s end))

(define (sp-group-event-parallel-generate start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  "can be used in place of sp-group-event-generate.
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
  (struct-set *out state s start start end start f sp-group-event-generate free sp-group-event-free)
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
  sp-map-event-generate-t
  (type (function-pointer void sp-time-t sp-time-t sp-block-t sp-block-t sp-event-t* void*))
  sp-map-event-state-t
  (type (struct (event sp-event-t) (generate sp-map-event-generate-t) (state void*))))

(define (sp-map-event-free a) (void sp-event-t*)
  (declare s sp-map-event-state-t*)
  (set s a:state)
  (if s:event.free (s:event.free &s:event))
  (free a:state))

(define (sp-map-event-generate start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  "creates temporary output, lets event write to it, and passes the result to a user function"
  (declare s sp-map-event-state-t*)
  (set s event:state)
  (s:event.generate start end out &s:event)
  (s:f start end out out &s:event s:state))

(define (sp-map-event-isolated-generate start end out event)
  (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  "creates temporary output, lets event write to it, and passes the result to a user function"
  (declare s sp-map-event-state-t* temp-out sp-block-t)
  status-declare
  (set status (sp-block-new out.channels out.size &temp-out))
  (if status-is-failure return)
  (set s event:state)
  (s:event.generate start end temp-out &s:event)
  (s:generate start end temp-out out &s:event s:state)
  (sp-block-free temp-out))

(define (sp-map-event event f state isolate out)
  (status-t sp-event-t sp-map-event-generate-t void* uint8-t sp-event-t*)
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
    s:generate f
    e.state s
    e.start event.start
    e.end event.end
    e.generate (if* isolate sp-map-event-isolated-generate sp-map-event-generate)
    e.free sp-map-event-free)
  (set *out e)
  (label exit status-return))