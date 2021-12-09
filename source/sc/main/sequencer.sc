(sc-include-once "./sc-macros")

(define (sp-event-list-display-element a) (void sp-event-list-t*)
  (printf "%lu %lu %lu %lu %lu\n" a:previous a a:next a:event.start a:event.end))

(define (sp-event-list-display a) (void sp-event-list-t*)
  (while a (sp-event-list-display-element a) (set a a:next)))

(define (sp-event-list-reverse a) (void sp-event-list-t**)
  (declare current sp-event-list-t* next sp-event-list-t*)
  (set next *a)
  (while next (set current next next next:next current:next current:previous current:previous next))
  (set *a current))

(define (sp-event-list-find-duplicate a b) (uint8-t sp-event-list-t* sp-event-list-t*)
  (define i sp-time-t 0 count sp-time-t 0)
  (while a
    (if (= a b)
      (if (= 1 count) (begin (printf "duplicate list entry i%lu %lu\n" i a) (exit 1))
        (set+ count 1)))
    (set+ i 1)
    (set a a:next)))

(define (sp-event-list-validate a) (void sp-event-list-t*)
  (define i sp-time-t 0 b sp-event-list-t* a c sp-event-list-t* 0)
  (while b
    (if (not (= c b:previous))
      (begin (printf "link to previous is invalid at index %lu, element %lu\n" i b) (exit 1)))
    (if (and (= b:next b:previous) (not (= 0 b:next)))
      (begin (printf "circular list entry at index %lu, element %lu\n" i b) (exit 1)))
    (sp-event-list-find-duplicate a b)
    (set+ i 1)
    (set c b b b:next)))

(define (sp-event-list-remove a element) (void sp-event-list-t** sp-event-list-t*)
  "removes the list element and frees the event, without having to search in the list.
   updates the head of the list if element is the first element"
  (if element:previous (set element:previous:next element:next) (set *a element:next))
  (if element:next (set element:next:previous element:previous))
  (if element:event.free (element:event.free &element:event))
  (free element))

(define (sp-event-list-add a event) (status-t sp-event-list-t** sp-event-t)
  "insert sorted by start time descending. event-list might get updated with a new head element"
  status-declare
  (declare current sp-event-list-t* new sp-event-list-t*)
  (status-require (sp-malloc-type 1 sp-event-list-t &new))
  (set new:event event current *a)
  (if (not current) (begin (sc-comment "empty") (set new:next 0 new:previous 0 *a new) (goto exit)))
  (if (<= current:event.start event.start)
    (begin (sc-comment "first") (set new:previous 0 new:next current current:previous new *a new))
    (begin
      (while current:next
        (set current current:next)
        (if (<= current:event.start event.start)
          (begin
            (sc-comment "-- middle")
            (set
              new:previous current:previous
              new:next current
              current:previous:next new
              current:previous new)
            (goto exit))))
      (sc-comment "-- last")
      (set new:next 0 new:previous current current:next new)))
  (label exit status-return))

(define (sp-event-list-free events) (void sp-event-list-t**)
  "free all list elements and the associated events. update the list head so it becomes the empty list.
   needed if sp_seq will not further process and thereby free further past events"
  (declare current sp-event-list-t* temp sp-event-list-t*)
  (set current *events)
  (while current (sp-event-free current:event) (set temp current current current:next) (free temp))
  (set events 0))

(define (sp-seq start end out events) (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  "event arrays must have been prepared/sorted once with sp_seq_event_prepare for seq to work correctly.
   like for paths, start is inclusive, end is exclusive, so that 0..100 and 100..200 attach seamless.
   events can have three function pointers: prepare, generate and free.
   generate receives event relative start/end times, and output blocks that start at the current block.
   prepare will only be called once and the function pointer will be set to zero afterwards.
   if prepare fails, sp_seq returns immediately.
   past events including the event list elements are freed when processing the following block.
   on error, all events and the event list are freed"
  status-declare
  (declare e sp-event-t ep sp-event-t* current sp-event-list-t* next sp-event-list-t*)
  (set current *events)
  (while current
    (set ep &current:event e *ep)
    (cond
      ( (<= e.end start) (set next current:next) (sp-event-list-remove events current)
        (set current next) continue)
      ((<= end e.start) break)
      ( (> e.end start)
        (if e.prepare (begin (status-require (e.prepare ep)) (set ep:prepare 0 e *ep)))
        (status-require
          (e.generate (if* (> start e.start) (- start e.start) 0) (- (sp-min end e.end) e.start)
            (if* (> e.start start) (sp-block-with-offset out (- e.start start)) out) ep))))
    (set current current:next))
  (label exit (if status-is-failure (sp-event-list-free events)) status-return))

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
  status-declare
  (declare a sp-seq-future-t* ep sp-event-t*)
  (set a data ep a:event)
  (if ep:prepare (begin (status-require (ep:prepare ep)) (set ep:prepare 0)))
  (status-require (ep:generate a:start a:end a:out ep))
  (label exit (set a:status status) (if status-is-failure (sp-event-pointer-free ep)) (return 0)))

(define (sp-seq-parallel start end out events)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  "like sp_seq but evaluates events with multiple threads in parallel.
   there is some overhead, as each event gets its own output block"
  status-declare
  (declare
    current sp-event-list-t*
    active sp-time-t
    allocated sp-time-t
    e-end sp-time-t
    ep sp-event-t*
    e sp-event-t
    e-start sp-time-t
    sf-array sp-seq-future-t*
    sf sp-seq-future-t*
    next sp-event-list-t*)
  (set sf-array 0 active 0 allocated 0 current *events)
  (sc-comment "count")
  (while current
    (cond ((<= end current:event.start) break) ((> current:event.end start) (set+ active 1)))
    (set current current:next))
  (status-require (sp-malloc-type active sp-seq-future-t &sf-array))
  (sc-comment "distribute")
  (set current *events)
  (while current
    (set ep &current:event e *ep)
    (cond
      ( (<= e.end start) (set next current:next) (sp-event-list-remove events current)
        (set current next) continue)
      ((<= end e.start) break)
      ( (> e.end start)
        (set
          sf (+ sf-array allocated)
          e-start (if* (> start e.start) (- start e.start) 0)
          e-end (- (sp-min end e.end) e.start)
          sf:start e-start
          sf:end e-end
          sf:out-start (if* (> e.start start) (- e.start start) 0)
          sf:event ep
          sf:status.id status-id-success)
        (status-require (sp-block-new out.channels (- e-end e-start) &sf:out)) (set+ allocated 1)
        (future-new sp-seq-parallel-future-f sf &sf:future)))
    (set current current:next))
  (sc-comment "merge")
  (for-each-index sf-i active
    (set sf (+ sf-array sf-i))
    (touch &sf:future)
    (status-require sf:status)
    (for-each-index ci out.channels
      (for-each-index i sf:out.size
        (set+ (array-get (array-get out.samples ci) (+ sf:out-start i))
          (array-get (array-get sf:out.samples ci) i)))))
  (label exit
    (if sf-array
      (begin
        (for-each-index i allocated
          (sp-block-free (address-of (struct-get (array-get sf-array i) out))))
        (free sf-array)))
    (if status-is-failure (sp-event-list-free events))
    status-return))

(define (sp-group-free a) (void sp-event-t*)
  (set a:free 0)
  (sp-event-list-free (sp-group-event-list a))
  (sp-event-memory-free a))

(define (sp-group-generate start end out a) (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (return (sp-seq start end out (convert-type &a:data sp-event-list-t**))))

(define (sp-group-generate-parallel start end out a)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (return (sp-seq-parallel start end out (convert-type &a:data sp-event-list-t**))))

(define (sp-group-prepare a) (status-t sp-event-t*)
  status-declare
  (sp-seq-events-prepare (convert-type &a:data sp-event-list-t**))
  (set a:generate sp-group-generate a:free sp-group-free)
  status-return)

(define (sp-group-prepare-parallel a) (status-t sp-event-t*)
  status-declare
  (sp-seq-events-prepare (convert-type &a:data sp-event-list-t**))
  (set a:generate sp-group-generate-parallel a:free sp-group-free)
  status-return)

(define (sp-group-add a event) (status-t sp-event-t* sp-event-t)
  status-declare
  (status-require (sp-event-list-add (convert-type &a:data sp-event-list-t**) event))
  (if (< a:end event.end) (set a:end event.end))
  (label exit status-return))

(define (sp-group-append a event) (status-t sp-event-t* sp-event-t)
  (set+ event.start a:end event.end a:end)
  (return (sp-group-add a event)))

(define (sp-channel-config mute delay phs amp amod)
  (sp-channel-config-t sp-bool-t sp-time-t sp-time-t sp-sample-t sp-sample-t*)
  (declare a sp-channel-config-t)
  (struct-set a use #t mute mute delay delay phs phs amp amp amod amod)
  (return a))

(define (sp-channel-config-zero a) (void sp-channel-config-t*)
  (define channel-config sp-channel-config-t (struct-literal 0))
  (for-each-index i sp-channel-limit (set (array-get a i) channel-config)))

(define (sp-wave-event-config-defaults config) (void sp-wave-event-config-t*)
  (struct-pointer-set config
    wvf sp-sine-table
    wvf-size sp-rate
    phs 0
    frq sp-rate
    fmod 0
    amp 1
    amod 0
    channels sp-channels)
  (sp-channel-config-zero config:channel-config))

(define (sp-wave-event-config-new out) (status-t sp-wave-event-config-t**)
  "heap allocates a sp_wave_event_config_t struct and sets some defaults"
  status-declare
  (status-require (sp-malloc-type 1 sp-wave-event-config-t out))
  (sp-wave-event-config-defaults *out)
  (label exit status-return))

(define (sp-map-event-config-new out) (status-t sp-map-event-config-t**)
  status-declare
  (declare result sp-map-event-config-t*)
  (status-require (sp-malloc-type 1 sp-map-event-config-t &result))
  (struct-set *result state 0 isolate 0)
  (set *out result)
  (label exit status-return))

(define (sp-wave-event-free a) (void sp-event-t*) (set a:free 0) (free a:data))

(define (sp-wave-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare i sp-time-t sp sp-wave-event-state-t* s sp-wave-event-state-t)
  (set sp event:data s *sp)
  (for ((set i start) (< i end) (set+ i 1))
    (set+ (array-get out.samples s.channel (- i start))
      (* s.amp (array-get s.amod i) (array-get s.wvf s.phs)) s.phs (sp-array-or-fixed s.fmod s.frq i))
    (if (>= s.phs s.wvf-size) (set s.phs (modulo s.phs s.wvf-size))))
  (set sp:phs s.phs)
  status-return)

(define (sp-wave-event-channel duration config channel out)
  (status-t sp-time-t sp-wave-event-config-t sp-channel-count-t sp-event-t*)
  status-declare
  (declare data sp-wave-event-state-t* channel-config sp-channel-config-t)
  (sp-declare-event event)
  (set data 0 channel-config (array-get config.channel-config channel))
  (status-require (sp-malloc-type 1 sp-wave-event-state-t &data))
  (struct-set *data
    wvf config.wvf
    wvf-size config.wvf-size
    phs (if* channel-config.use channel-config.phs config.phs)
    frq config.frq
    fmod config.fmod
    amp (if* channel-config.use channel-config.amp config.amp)
    amod (if* (and channel-config.use channel-config.amod) channel-config.amod config.amod)
    channel channel)
  (struct-set event
    data data
    start (if* channel-config.use channel-config.delay 0)
    end (if* channel-config.use (+ duration channel-config.delay) duration)
    prepare 0
    generate sp-wave-event-generate
    free sp-wave-event-free)
  (set *out event)
  (label exit (if status-is-failure (if data (free data))) status-return))

(define (sp-wave-event-prepare event) (status-t sp-event-t*)
  "prepare an event playing waveforms from an array.
   config should have been declared with defaults using sp_declare_wave_event_config.
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
  (sp-declare-event channel)
  (define config sp-wave-event-config-t
    (pointer-get (convert-type event:data sp-wave-event-config-t*)))
  (set event:data 0 event:free sp-group-free)
  (for-each-index ci config.channels
    (if (struct-get (array-get config.channel-config ci) mute) continue)
    (status-require (sp-wave-event-channel (- event:end event:start) config ci &channel))
    (status-require (sp-group-add event channel)))
  (status-require (sp-group-prepare event))
  (label exit (if status-is-failure (event:free event)) status-return))

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

(define (sp-noise-event-config-new out) (status-t sp-noise-event-config-t**)
  status-declare
  (declare result sp-noise-event-config-t*)
  (status-require (sp-malloc-type 1 sp-noise-event-config-t &result))
  (struct-set *result
    amp 1
    amod 0
    cutl 0.0
    cuth 0.5
    trnl 0.07
    trnh 0.07
    cutl-mod 0
    cuth-mod 0
    resolution (/ sp-rate 10)
    is-reject 0
    channels sp-channels)
  (sp-channel-config-zero result:channel-config)
  (set *out result)
  (label exit status-return))

(define (sp-noise-event-free a) (void sp-event-t*)
  (set a:free 0)
  (define s sp-noise-event-state-t* a:data)
  (sp-convolution-filter-state-free s:filter-state)
  (free s)
  (sp-event-memory-free a))

(define (sp-noise-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
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
    sp event:data
    s *sp
    duration (- end start)
    block-count (if* (= duration s.resolution) 1 (sp-cheap-floor-positive (/ duration s.resolution)))
    block-rest (modulo duration s.resolution))
  (for ((set block-i 0) (< block-i block-count) (set+ block-i 1))
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
  (set sp:random-state s.random-state sp:filter-state s.filter-state)
  status-return)

(define (sp-noise-event-filter-state cutl cuth trnl trnh is-reject rs out)
  (status-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-bool-t sp-random-state-t* sp-convolution-filter-state-t**)
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

(define (sp-noise-event-channel duration config channel rs state-noise state-temp event)
  (status-t sp-time-t sp-noise-event-config-t sp-channel-count-t sp-random-state-t sp-sample-t* sp-sample-t* sp-event-t*)
  status-declare
  (declare
    state sp-noise-event-state-t*
    channel-config sp-channel-config-t
    filter-state sp-convolution-filter-state-t*)
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
  (struct-set *event
    data state
    start (if* channel-config.use channel-config.delay 0)
    end (if* channel-config.use (+ duration channel-config.delay) duration)
    generate sp-noise-event-generate
    free sp-noise-event-free)
  (label exit
    (if status-is-failure
      (begin
        (if state (free state))
        (if filter-state (sp-convolution-filter-state-free filter-state))))
    status-return))

(define (sp-noise-event-prepare event) (status-t sp-event-t*)
  "an event for noise filtered by a windowed-sinc filter.
   very processing intensive if parameters change with low resolution.
   memory for event.state will be allocated and then owned by the caller.
   all channels use the same initial random-state"
  status-declare
  (declare duration sp-time-t rs sp-random-state-t state-noise sp-sample-t* state-temp sp-sample-t*)
  (sp-declare-event channel)
  (define config sp-noise-event-config-t
    (pointer-get (convert-type event:data sp-noise-event-config-t*)))
  (set
    event:data 0
    event:free sp-group-free
    duration (- event:end event:start)
    config.resolution (if* config.resolution config.resolution 96)
    config.resolution (sp-min config.resolution duration)
    rs (sp-random-state-new (sp-time-random &sp-random-state)))
  (status-require (sp-event-memory-init event 2))
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-noise))
  (sp-event-memory-add event state-noise)
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-temp))
  (sp-event-memory-add event state-temp)
  (for-each-index ci config.channels
    (if (struct-get (array-get config.channel-config ci) mute) continue)
    (status-require (sp-noise-event-channel duration config ci rs state-noise state-temp &channel))
    (status-require (sp-group-add event channel)))
  (status-require (sp-group-prepare event))
  (label exit (if status-is-failure (event:free event)) status-return))

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

(define (sp-cheap-noise-event-config-new out) (status-t sp-cheap-noise-event-config-t**)
  status-declare
  (declare result sp-cheap-noise-event-config-t*)
  (status-require (sp-malloc-type 1 sp-cheap-noise-event-config-t &result))
  (struct-set *result
    amp 1
    amod 0
    cut 0.5
    cut-mod 0
    q-factor 0.01
    passes 1
    type sp-state-variable-filter-lp
    resolution (/ sp-rate 10)
    channels sp-channels)
  (sp-channel-config-zero result:channel-config)
  (set *out result)
  (label exit status-return))

(define (sp-cheap-noise-event-free a) (void sp-event-t*)
  (set a:free 0)
  (define s sp-cheap-noise-event-state-t* a:data)
  (sp-cheap-filter-state-free &s:filter-state)
  (free s)
  (sp-event-memory-free a))

(define (sp-cheap-noise-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
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
    sp event:data
    s *sp
    duration (- end start)
    block-count (if* (= duration s.resolution) 1 (sp-cheap-floor-positive (/ duration s.resolution)))
    block-rest (modulo duration s.resolution))
  (sc-comment "total block count is block-count plus rest-block")
  (for ((set block-i 0) (< block-i block-count) (set+ block-i 1))
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
  (set sp:random-state s.random-state sp:filter-state s.filter-state)
  status-return)

(define (sp-cheap-noise-event-channel duration config channel rs state-noise state-temp out)
  (status-t sp-time-t sp-cheap-noise-event-config-t sp-channel-count-t sp-random-state-t sp-sample-t* sp-sample-t* sp-event-t*)
  status-declare
  (declare data sp-cheap-noise-event-state-t* channel-config sp-channel-config-t)
  (sp-declare-event event)
  (sp-declare-cheap-filter-state filter-state)
  (set data 0 channel-config (array-get config.channel-config channel))
  (status-require (sp-malloc-type 1 sp-cheap-noise-event-state-t &data))
  (status-require (sp-cheap-filter-state-new config.resolution config.passes &filter-state))
  (struct-set *data
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
    data data
    start (if* channel-config.use channel-config.delay 0)
    end (if* channel-config.use (+ duration channel-config.delay) duration)
    prepare 0
    generate sp-cheap-noise-event-generate
    free sp-cheap-noise-event-free)
  (set *out event)
  (label exit
    (if status-is-failure (begin (if data (free data)) (sp-cheap-filter-state-free &filter-state)))
    status-return))

(define (sp-cheap-noise-event-prepare event) (status-t sp-event-t*)
  "an event for noise filtered by a state-variable filter.
   lower processing costs even when parameters change with high resolution.
   multiple passes almost multiply performance costs.
   memory for event.state will be allocated and then owned by the caller"
  status-declare
  (declare rs sp-random-state-t state-noise sp-sample-t* state-temp sp-sample-t*)
  (sp-declare-event channel)
  (define config sp-cheap-noise-event-config-t
    (pointer-get (convert-type event:data sp-cheap-noise-event-config-t*)))
  (set
    event:data 0
    event:free sp-group-free
    config.passes (if* config.passes config.passes 1)
    config.resolution (if* config.resolution config.resolution 96)
    config.resolution (sp-min config.resolution (- event:end event:start))
    rs (sp-random-state-new (sp-time-random &sp-random-state)))
  (status-require (sp-event-memory-init event 2))
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-noise))
  (sp-event-memory-add event state-noise)
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-temp))
  (sp-event-memory-add event state-temp)
  (for-each-index ci config.channels
    (if (struct-get (array-get config.channel-config ci) mute) continue)
    (status-require
      (sp-cheap-noise-event-channel (- event:end event:start) config
        ci rs state-noise state-temp &channel))
    (status-require (sp-group-add event channel)))
  (status-require (sp-group-prepare event))
  (label exit status-return))

(define (sp-event-memory-init a additional-size) (status-t sp-event-t* sp-time-t)
  "ensures that event memory is initialized and can take $additional_size more elements"
  status-declare
  (if a:memory.data
    (if
      (and (> additional-size (array3-unused-size a:memory))
        (sp-memory-resize &a:memory
          (+ (array3-max-size a:memory) (- additional-size (array3-unused-size a:memory)))))
      sp-memory-error)
    (if (sp-memory-new additional-size &a:memory) sp-memory-error
      (if (not a:free) (set a:free sp-event-memory-free))))
  (label exit status-return))

(define (sp-event-memory-add2 a address handler) (void sp-event-t* void* sp-memory-free-t)
  (declare m memreg2-t)
  (struct-set m address address handler handler)
  (sp-memory-add a:memory m))

(define (sp-event-memory-free a) (void sp-event-t*)
  "free all registered memory and unitialize the event-memory register.
   can be used as an event.free function if only memory should be removed before other fields have been set"
  (if (not a:memory.data) return)
  (declare m memreg2-t)
  (for-each-index i (array3-size a:memory) (set m (array3-get a:memory i)) (m.handler m.address))
  (array3-free a:memory)
  (set a:memory.data 0))

(define (sp-map-event-free event) (void sp-event-t*)
  (declare s sp-map-event-state-t*)
  (set event:free 0 s event:data)
  (sp-event-free s:event)
  (free s)
  (sp-event-memory-free event))

(define (sp-map-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (define s sp-map-event-state-t* event:data)
  (status-require (s:event.generate start end out &s:event))
  (status-require (s:map-generate start end out out s:state))
  (label exit status-return))

(define (sp-map-event-isolated-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  "creates temporary output, lets event write to it, and passes the temporary output to a user function"
  status-declare
  (declare temp-out sp-block-t)
  (define s sp-map-event-state-t* event:data)
  (status-require (sp-block-new out.channels out.size &temp-out))
  (status-require (s:event.generate start end temp-out &s:event))
  (status-require (s:map-generate start end temp-out out s:state))
  (label exit (sp-block-free &temp-out) status-return))

(define (sp-map-event-prepare event) (status-t sp-event-t*)
  "the wrapped event will be freed with the map-event.
   use cases: filter chains, post processing.
   config:
     map-generate: map function (start end sp_block_t:in sp_block_t:out void*:state)
     state: custom state value passed to f.
     isolate: use a dedicated output buffer for event
       events can be wrapped in multiple sp_map_event with an isolated sp_map_event on top that
       finally writes to main out to mix with other events"
  status-declare
  (declare state sp-map-event-state-t*)
  (free-on-error-init 1)
  (define config sp-map-event-config-t
    (pointer-get (convert-type event:data sp-map-event-config-t*)))
  (status-require (sp-malloc-type 1 sp-map-event-state-t &state))
  (free-on-error1 state)
  (status-require (config.event.prepare &config.event))
  (set
    state:event config.event
    state:state config.state
    state:map-generate config.map-generate
    event:data state
    event:generate (if* config.isolate sp-map-event-isolated-generate sp-map-event-generate)
    event:free sp-map-event-free)
  (label exit (if status-is-failure free-on-error-free) status-return))

(define (sp-group-add-set group start duration volume config event)
  (status-t sp-event-t* sp-time-t sp-time-t sp-sample-t void* sp-event-t)
  (struct-set event
    start start
    end (+ start (if* (= 0 duration) event.end duration))
    volume volume
    data config)
  (return (sp-group-add group event)))

(define (sp-group-append-set group volume config event)
  (status-t sp-event-t* sp-sample-t void* sp-event-t)
  (struct-set event volume volume data config)
  (return (sp-group-append group event)))