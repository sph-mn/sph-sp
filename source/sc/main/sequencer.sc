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

(define (sp-seq-events-prepare events) (void sp-events-t*)
  (quicksort sp-event-sort-less? sp-event-sort-swap events:data 0 (- events:used 1)))

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
    (cond ((<= e.end start) (if e.free (e.free ep)) (array4-forward *events))
      ((<= end e.start) break)
      ( (> e.end start)
        (if e.prepare (begin (status-require (e.prepare ep)) (set ep:prepare 0 e.state ep:state)))
        (status-require
          (e.generate (if* (> start e.start) (- start e.start) 0) (- (sp-min end e.end) e.start)
            (if* (> e.start start) (sp-block-with-offset out (- e.start start)) out) e.state)))))
  (label exit status-return))

(define (sp-seq-events-free events) (void sp-events-t*)
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
  (set a:status (a:event:generate a:start a:end a:out a:event:state))
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
  (set sf-array 0 sf-i 0 active 0 allocated 0)
  (for ((set i events:current) (< i events:used) (set+ i 1))
    (set ep (+ events:data i))
    (cond ((<= end ep:start) break) ((> ep:end start) (set+ active 1))))
  (status-require (sp-malloc-type active sp-seq-future-t &sf-array))
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

(define (sp-group-event-free a) (void sp-event-t*)
  (define sp sp-events-t* a:state)
  (sp-seq-events-free sp)
  (array4-free *sp)
  (free sp)
  (sp-event-memory-free a))

(define (sp-group-prepare event) (status-t sp-event-t*)
  status-declare
  (sp-seq-events-prepare (convert-type event:state sp-events-t*))
  status-return)

(define (sp-group-free event) (void sp-event-t*)
  (if (not event:state) return)
  (define events sp-events-t* event:state)
  (sp-seq-events-free events)
  (array4-free *events)
  (free events))

(define (sp-group-new start event-size out) (status-t sp-time-t sp-group-size-t sp-event-t*)
  status-declare
  (declare s sp-events-t*)
  (set s 0)
  (status-require (sp-malloc-type 1 sp-events-t &s))
  (if (sp-events-new event-size s) sp-memory-error)
  (struct-set *out
    state s
    start start
    end start
    prepare sp-group-prepare
    generate (convert-type sp-seq sp-event-generate-t)
    free sp-group-free)
  (label exit (if status-is-failure (if s (free s))) status-return))

(define (sp-group-append a event) (void sp-event-t* sp-event-t)
  (set+ event.start a:end event.end a:end)
  (sp-group-add *a event))

(define (sp-channel-config mute delay phs amp amod)
  (sp-channel-config-t boolean sp-time-t sp-time-t sp-sample-t sp-sample-t*)
  (declare a sp-channel-config-t)
  (struct-set a use #t mute mute delay delay phs phs amp amp amod amod)
  (return a))

(define (sp-wave-event-free a) (void sp-event-t*) (free a:state))

(define (sp-wave-event-generate start end out state)
  (status-t sp-time-t sp-time-t sp-block-t void*)
  status-declare
  (declare i sp-time-t sp sp-wave-event-state-t* s sp-wave-event-state-t)
  (set sp state s *sp)
  (for ((set i start) (< i end) (set+ i 1))
    (set+ (array-get out.samples s.channel (- i start))
      (* s.amp (array-get s.amod i) (array-get s.wvf s.phs)) s.phs (sp-array-or-fixed s.fmod s.frq i))
    (if (>= s.phs s.wvf-size) (set s.phs (modulo s.phs s.wvf-size))))
  (set sp:phs s.phs)
  status-return)

(define (sp-wave-event-channel duration config channel out)
  (status-t sp-time-t sp-wave-event-config-t sp-channel-count-t sp-event-t*)
  status-declare
  (declare state sp-wave-event-state-t* channel-config sp-channel-config-t)
  (sp-declare-event event)
  (set state 0 channel-config (array-get config.channel-config channel))
  (status-require (sp-malloc-type 1 sp-wave-event-state-t &state))
  (struct-set *state
    wvf config.wvf
    wvf-size config.wvf-size
    phs (if* channel-config.use channel-config.phs config.phs)
    frq config.frq
    fmod config.fmod
    amp (if* channel-config.use channel-config.amp config.amp)
    amod (if* (and channel-config.use channel-config.amod) channel-config.amod config.amod)
    channel channel)
  (struct-set event
    state state
    start (if* channel-config.use channel-config.delay 0)
    end (if* channel-config.use (+ duration channel-config.delay) duration)
    prepare 0
    generate sp-wave-event-generate
    free sp-wave-event-free)
  (set *out event)
  (label exit (if status-is-failure (if state (free state))) status-return))

(define (sp-wave-event start end config out)
  (status-t sp-time-t sp-time-t sp-wave-event-config-t sp-event-t*)
  "create an event playing waveforms from an array.
   config should have been declared with defaults using sp-declare-wave-event-config.
   event end will be longer if channel config delay is used.
   config:
   * frq (frequency): fixed frequency, only used if fmod is null
   * fmod (frequency): array with hertz values
   * wvf (waveform): array with waveform samples
   * wvf-size: count of waveform samples
   * phs (phase): initial phase offset
   * amp (amplitude): multiplied with amod
   * amod (amplitude): array with sample values
   channel-config (array with one element per channel):
   * use: if zero, config for this channel will not be applied
   * mute: non-zero for silencing this channel
   * delay: integer number of samples
   * amod: uses main amod if zero
   * amp: multiplied with amod"
  status-declare
  (declare event sp-event-t)
  (sp-declare-event group)
  (status-require (sp-group-new 0 config.channels &group))
  (for-each-index ci config.channels
    (if (struct-get (array-get config.channel-config ci) mute) continue)
    (status-require (sp-wave-event-channel (- end start) config ci &event))
    (sp-group-add group event))
  (set group.memory out:memory *out group)
  (label exit (if status-is-failure (sp-group-free &group)) status-return))

(declare sp-noise-event-state-t
  (type
    (struct
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (cutl sp-sample-t)
      (cuth sp-sample-t)
      (trnl sp-sample-t)
      (trnh sp-sample-t)
      (cutl-mod sp-sample-t*)
      (cuth-mod sp-sample-t*)
      (resolution sp-time-t)
      (is-reject uint8-t)
      (random-state sp-random-state-t)
      (channel sp-channel-count-t)
      (filter-state sp-convolution-filter-state-t*)
      (noise sp-sample-t*)
      (temp sp-sample-t*))))

(define (sp-noise-event-free a) (void sp-event-t*)
  (define s sp-noise-event-state-t* a:state)
  (sp-convolution-filter-state-free s:filter-state)
  (free a:state)
  (sp-event-memory-free a))

(define (sp-noise-event-generate start end out state)
  (status-t sp-time-t sp-time-t sp-block-t void*)
  "updates filter arguments only every resolution number of samples"
  status-declare
  (declare
    block-count sp-time-t
    block-i sp-time-t
    block-offset sp-time-t
    block-rest sp-time-t
    duration sp-time-t
    i sp-time-t
    s sp-noise-event-state-t
    sp sp-noise-event-state-t*
    t sp-time-t)
  (set
    sp state
    s *sp
    duration (- end start)
    block-count (if* (= duration s.resolution) 1 (sp-cheap-floor-positive (/ duration s.resolution)))
    block-rest (modulo duration s.resolution))
  (for ((set block-i 0) (<= block-i block-count) (set+ block-i 1))
    (set
      block-offset (* s.resolution block-i)
      t (+ start block-offset)
      duration (if* (= block-count block-i) block-rest s.resolution))
    (sp-samples-random &s.random-state duration s.noise)
    (sp-windowed-sinc-bp-br s.noise duration
      (sp-array-or-fixed s.cutl-mod s.cutl t) (sp-array-or-fixed s.cuth-mod s.cuth t) s.trnl
      s.trnh s.is-reject &s.filter-state s.temp)
    (for ((set i 0) (< i duration) (set+ i 1))
      (set+ (array-get out.samples s.channel (+ block-offset i))
        (* s.amp (array-get s.amod (+ t i)) (array-get s.temp i)))))
  (set sp:random-state s.random-state)
  status-return)

(define (sp-noise-event-filter-state cutl cuth trnl trnh is-reject rs out)
  (status-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t boolean sp-random-state-t* sp-convolution-filter-state-t**)
  "the result shows a small delay, for example, circa 40 samples for transition 0.07. the size seems to be related to ir-len.
   the filter state is initialised with one unused call to skip the delay."
  status-declare
  (declare ir-len sp-time-t temp sp-sample-t* noise sp-sample-t*)
  (memreg-init 2)
  (set ir-len (sp-windowed-sinc-lp-hp-ir-length (sp-min trnl trnh)))
  (sp-malloc-type ir-len sp-sample-t &noise)
  (memreg-add noise)
  (sp-malloc-type ir-len sp-sample-t &temp)
  (memreg-add temp)
  (sp-samples-random rs ir-len noise)
  (status-require (sp-windowed-sinc-bp-br noise ir-len cutl cuth trnl trnh is-reject out temp))
  (label exit memreg-free status-return))

(define (sp-noise-event-channel duration config channel rs state-noise state-temp out)
  (status-t sp-time-t sp-noise-event-config-t sp-channel-count-t sp-random-state-t sp-sample-t* sp-sample-t* sp-event-t*)
  status-declare
  (declare
    state sp-noise-event-state-t*
    channel-config sp-channel-config-t
    filter-state sp-convolution-filter-state-t*)
  (sp-declare-event event)
  (set state 0 filter-state 0 channel-config (array-get config.channel-config channel))
  (status-require
    (sp-noise-event-filter-state (sp-array-or-fixed config.cutl-mod config.cutl 0)
      (sp-array-or-fixed config.cuth-mod config.cuth 0) config.trnl config.trnh
      config.is-reject &rs &filter-state))
  (status-require (sp-malloc-type 1 sp-noise-event-state-t &state))
  (struct-set *state
    amp (if* channel-config.use channel-config.amp config.amp)
    amod (if* (and channel-config.use channel-config.amod) channel-config.amod config.amod)
    cutl config.cutl
    cuth config.cuth
    trnl config.trnl
    trnh config.trnh
    cutl-mod config.cutl-mod
    cuth-mod config.cuth-mod
    resolution config.resolution
    is-reject config.is-reject
    random-state rs
    channel channel
    filter-state filter-state
    noise state-noise
    temp state-temp)
  (struct-set event
    state state
    start (if* channel-config.use channel-config.delay 0)
    end (if* channel-config.use (+ duration channel-config.delay) duration)
    prepare 0
    generate sp-noise-event-generate
    free sp-noise-event-free)
  (set *out event)
  (label exit
    (if status-is-failure
      (begin
        (if state (free state))
        (if filter-state (sp-convolution-filter-state-free filter-state))))
    status-return))

(define (sp-noise-event start end config out)
  (status-t sp-time-t sp-time-t sp-noise-event-config-t sp-event-t*)
  "an event for noise filtered by a windowed-sinc filter.
   very processing intensive if parameters change with low resolution.
   memory for event.state will be allocated and then owned by the caller.
   all channels use the same initial random-state"
  status-declare
  (declare
    duration sp-time-t
    event sp-event-t
    rs sp-random-state-t
    state-noise sp-sample-t*
    state-temp sp-sample-t*)
  (sp-declare-event group)
  (set
    duration (- end start)
    config.resolution (if* config.resolution config.resolution 96)
    config.resolution (sp-min config.resolution duration)
    rs (sp-random-state-new (sp-time-random &sp-default-random-state)))
  (status-require (sp-group-new start config.channels &group))
  (status-require (sp-event-memory-init &group 2))
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-noise))
  (sp-event-memory-add1 &group state-noise)
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-temp))
  (sp-event-memory-add1 &group state-temp)
  (for-each-index ci config.channels
    (if (struct-get (array-get config.channel-config ci) mute) continue)
    (status-require (sp-noise-event-channel duration config ci rs state-noise state-temp &event))
    (sp-group-add group event))
  (set group.memory out:memory *out group)
  (label exit (if status-is-failure (sp-group-free &group)) status-return))

(declare sp-cheap-noise-event-state-t
  (type
    (struct
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (cut sp-sample-t)
      (cut-mod sp-sample-t*)
      (q-factor sp-sample-t)
      (passes sp-time-t)
      (type sp-state-variable-filter-t)
      (random-state sp-random-state-t)
      (resolution sp-time-t)
      (channel sp-channel-count-t)
      (filter-state sp-cheap-filter-state-t)
      (noise sp-sample-t*)
      (temp sp-sample-t*))))

(define (sp-cheap-noise-event-free a) (void sp-event-t*)
  (define s sp-cheap-noise-event-state-t* a:state)
  (sp-cheap-filter-state-free &s:filter-state)
  (free a:state)
  (sp-event-memory-free a))

(define (sp-cheap-noise-event-generate start end out state)
  (status-t sp-time-t sp-time-t sp-block-t void*)
  status-declare
  (declare
    block-count sp-time-t
    block-i sp-time-t
    block-offset sp-time-t
    block-rest sp-time-t
    duration sp-time-t
    i sp-time-t
    s sp-cheap-noise-event-state-t
    sp sp-cheap-noise-event-state-t*
    t sp-time-t)
  (sc-comment "update filter arguments only every resolution number of samples")
  (set
    sp state
    s *sp
    duration (- end start)
    block-count (if* (= duration s.resolution) 1 (sp-cheap-floor-positive (/ duration s.resolution)))
    block-rest (modulo duration s.resolution))
  (sc-comment "total block count is block-count plus rest-block")
  (for ((set block-i 0) (<= block-i block-count) (set+ block-i 1))
    (set
      block-offset (* s.resolution block-i)
      t (+ start block-offset)
      duration (if* (= block-count block-i) block-rest s.resolution))
    (sp-samples-random &s.random-state duration s.noise)
    (sp-cheap-filter s.type s.noise
      duration (sp-array-or-fixed s.cut-mod s.cut t) s.passes s.q-factor &s.filter-state s.temp)
    (for ((set i 0) (< i duration) (set+ i 1))
      (set+ (array-get out.samples s.channel (+ block-offset i))
        (* s.amp (array-get s.amod (+ t i)) (array-get s.temp i)))))
  (set sp:random-state s.random-state)
  status-return)

(define (sp-cheap-noise-event-channel duration config channel rs state-noise state-temp out)
  (status-t sp-time-t sp-cheap-noise-event-config-t sp-channel-count-t sp-random-state-t sp-sample-t* sp-sample-t* sp-event-t*)
  status-declare
  (declare state sp-cheap-noise-event-state-t* channel-config sp-channel-config-t)
  (sp-declare-event event)
  (sp-declare-cheap-filter-state filter-state)
  (set state 0 channel-config (array-get config.channel-config channel))
  (status-require (sp-malloc-type 1 sp-cheap-noise-event-state-t &state))
  (status-require (sp-cheap-filter-state-new config.resolution config.passes &filter-state))
  (struct-set *state
    amp (if* channel-config.use channel-config.amp config.amp)
    amod (if* (and channel-config.use channel-config.amod) channel-config.amod config.amod)
    cut config.cut
    cut-mod config.cut-mod
    type config.type
    passes config.passes
    q-factor config.q-factor
    resolution config.resolution
    random-state rs
    channel channel
    filter-state filter-state
    noise state-noise
    temp state-temp)
  (struct-set event
    state state
    start (if* channel-config.use channel-config.delay 0)
    end (if* channel-config.use (+ duration channel-config.delay) duration)
    prepare 0
    generate sp-cheap-noise-event-generate
    free sp-cheap-noise-event-free)
  (set *out event)
  (label exit
    (if status-is-failure
      (begin (if state (free state)) (sp-cheap-filter-state-free &filter-state)))
    status-return))

(define (sp-cheap-noise-event start end config out)
  (status-t sp-time-t sp-time-t sp-cheap-noise-event-config-t sp-event-t*)
  "an event for noise filtered by a state-variable filter.
   lower processing costs even when parameters change with high resolution.
   multiple passes almost multiply performance costs.
   memory for event.state will be allocated and then owned by the caller"
  status-declare
  (declare event sp-event-t rs sp-random-state-t state-noise sp-sample-t* state-temp sp-sample-t*)
  (sp-declare-event group)
  (set
    config.passes (if* config.passes config.passes 1)
    config.resolution (if* config.resolution config.resolution 96)
    config.resolution (sp-min config.resolution (- end start))
    rs (sp-random-state-new (sp-time-random &sp-default-random-state)))
  (status-require (sp-group-new start config.channels &group))
  (status-require (sp-event-memory-init &group 2))
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-noise))
  (sp-event-memory-add1 &group state-noise)
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-temp))
  (sp-event-memory-add1 &group state-temp)
  (for-each-index ci config.channels
    (if (struct-get (array-get config.channel-config ci) mute) continue)
    (status-require
      (sp-cheap-noise-event-channel (- end start) config ci rs state-noise state-temp &event))
    (sp-group-add group event))
  (set group.memory out:memory *out group)
  (label exit (if status-is-failure (sp-group-free &group)) status-return))

(define (sp-event-memory-init a additional-size) (status-t sp-event-t* sp-time-t)
  "ensures that event memory is initialized and can take $additional_size more elements"
  status-declare
  (if a:memory.data
    (if
      (and (> additional-size (array3-unused-size a:memory))
        (sp-memory-resize &a:memory
          (+ (array3-max-size a:memory) (- additional-size (array3-unused-size a:memory)))))
      sp-memory-error)
    (if (sp-memory-new additional-size &a:memory) sp-memory-error))
  (label exit status-return))

(define (sp-event-memory-add a address handler) (void sp-event-t* void* sp-memory-free-t)
  (declare m memreg2-t)
  (struct-set m address address handler handler)
  (sp-memory-add a:memory m))

(define (sp-event-memory-free a) (void sp-event-t*)
  "free all registered memory and unitialize the event-memory register"
  (declare m memreg2-t)
  (for-each-index i (array3-size a:memory)
    (set m (array3-get a:memory i)) (m.handler m.address))
  (array3-free a:memory))

(define (sp-map-event-free a) (void sp-event-t*)
  (declare s sp-map-event-state-t*)
  (set s a:state)
  (if s:event.free (s:event.free &s:event))
  (free s)
  (sp-event-memory-free a))

(define (sp-map-event-generate start end out state) (status-t sp-time-t sp-time-t sp-block-t void*)
  status-declare
  (declare s sp-map-event-state-t*)
  (set s state)
  (status-require-return (s:event.generate start end out &s:event))
  (return (s:generate start end out out s:state)))

(define (sp-map-event-isolated-generate start end out state)
  (status-t sp-time-t sp-time-t sp-block-t void*)
  "creates temporary output, lets event write to it, and passes the temporary output to a user function"
  status-declare
  (declare s sp-map-event-state-t* temp-out sp-block-t)
  (status-require-return (sp-block-new out.channels out.size &temp-out))
  (set s state)
  (status-require (s:event.generate start end temp-out s:event.state))
  (status-require (s:generate start end temp-out out s:state))
  (label exit (sp-block-free temp-out) status-return))

(define (sp-map-event event f state isolate out)
  (status-t sp-event-t sp-map-event-generate-t void* uint8-t sp-event-t*)
  "f: map function (start end sp_block_t:in sp_block_t:out void*:state)
   state: custom state value passed to f.
   the wrapped event will be freed with the map-event.
   isolate: use a dedicated output buffer for event
     events can be wrapped in multiple sp_map_event with an isolated sp_map_event on top that
     finally writes to main out to mix with other events
   use cases: filter chains, post processing"
  status-declare
  (declare s sp-map-event-state-t*)
  (sp-declare-event e)
  (status-require (sp-malloc-type 1 sp-map-event-state-t &s))
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