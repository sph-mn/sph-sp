(sc-comment
  "event.free functions must set event.free to null.
   event.prepare and event.generate must be user replaceable.
   event.prepare is set to null by callers of event.prepare")

(define (sp-event-list-display-element a) (void sp-event-list-t*)
  (printf "%lu %lu %lu event %lu %lu\n" a:previous a a:next a:event.start a:event.end))

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
      (if (= 1 count) (begin (printf "duplicate list entry %lu at index %lu\n" a i) (exit 1))
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
            (sc-comment "middle")
            (set
              new:previous current:previous
              new:next current
              current:previous:next new
              current:previous new)
            (goto exit))))
      (sc-comment "last")
      (set new:next 0 new:previous current current:next new)))
  (label exit status-return))

(define (sp-event-list-free events) (void sp-event-list-t**)
  "free all list elements and the associated events. update the list head so it becomes the empty list.
   needed if sp_seq will not further process and free currently incomplete events"
  (declare temp sp-event-list-t* current sp-event-list-t*)
  (set current *events)
  (while current (sp-event-free current:event) (set temp current current current:next) (free temp))
  (set events 0))

(pre-define sp-event-memory-growth-factor 2 sp-event-memory-initial-size 4)

(define (sp-event-memory-add-with-handler a address handler)
  (status-t sp-event-t* void* sp-memory-free-t)
  "event memory addition with automatic resizing"
  status-declare
  (if a:memory.data
    (if
      (and (not (array3-unused-size a:memory))
        (sp-memory-resize &a:memory (* sp-event-memory-growth-factor (array3-max-size a:memory))))
      sp-memory-error)
    (if (sp-memory-new sp-event-memory-initial-size &a:memory) sp-memory-error
      (if (not a:free) (set a:free sp-event-memory-free))))
  (declare m memreg2-t)
  (struct-set m address address handler handler)
  (sp-memory-add a:memory m)
  (label exit status-return))

(define (sp-event-memory-ensure a additional-size) (status-t sp-event-t* sp-time-t)
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

(define (sp-event-memory-fixed-add-with-handler a address handler)
  (void sp-event-t* void* sp-memory-free-t)
  "event memory addition without needed return status type and without free space checks"
  (declare m memreg2-t)
  (struct-set m address address handler handler)
  (sp-memory-add a:memory m))

(define (sp-event-memory-free a) (void sp-event-t*)
  "free all registered memory and unitialize the event-memory register.
   can be used as an event.free function if only memory should be removed before other fields have been set"
  (if (not a:memory.data) return)
  (declare m memreg2-t)
  (sp-for-each-index i (array3-size a:memory) (set m (array3-get a:memory i)) (m.handler m.address))
  (array3-free a:memory)
  (set a:memory.data 0))

(define (sp-event-block-generate resolution generate start end out event)
  (status-t sp-time-t sp-event-block-generate-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare
    block-count sp-time-t
    block-initial sp-time-t
    block-rest sp-time-t
    duration sp-time-t
    i sp-time-t)
  (set duration (- end start))
  (if (< duration resolution) (set resolution duration))
  (set
    block-count (sp-cheap-floor-positive (/ duration resolution))
    block-initial (* block-count resolution)
    block-rest (modulo duration resolution))
  (for ((set i 0) (< i block-initial) (set+ i resolution))
    (status-require (generate resolution i (+ i start) out event)))
  (if block-rest (status-require (generate block-rest i (+ i start) out event)))
  (label exit status-return))

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
  (declare e sp-event-t ep sp-event-t* next sp-event-list-t* current sp-event-list-t*)
  (set current *events)
  (while current
    (set ep &current:event e *ep)
    (cond
      ( (<= e.end start) (set next current:next) (sp-event-list-remove events current)
        (set current next) continue)
      ((<= end e.start) break)
      ( (> e.end start) (sp-event-prepare-optional-srq e) (set *ep e)
        (status-require
          (e.generate (if* (> start e.start) (- start e.start) 0)
            (- (sp-inline-min end e.end) e.start)
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
      (future sph-future-t))))

(define (sp-seq-parallel-future-f data) (void* void*)
  status-declare
  (declare a sp-seq-future-t* ep sp-event-t*)
  (set a data ep a:event)
  (sp-event-pointer-prepare-optional-srq ep)
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
          e-end (- (sp-inline-min end e.end) e.start)
          sf:start e-start
          sf:end e-end
          sf:out-start (if* (> e.start start) (- e.start start) 0)
          sf:event ep
          sf:status.id status-id-success)
        (status-require (sp-block-new out.channel-count (- e-end e-start) &sf:out))
        (set+ allocated 1) (sph-future-new sp-seq-parallel-future-f sf &sf:future)))
    (set current current:next))
  (sc-comment "merge")
  (sp-for-each-index sf-i active
    (set sf (+ sf-array sf-i))
    (sph-future-touch &sf:future)
    (status-require sf:status)
    (sp-for-each-index ci out.channel-count
      (sp-for-each-index i sf:out.size
        (set+ (array-get (array-get out.samples ci) (+ sf:out-start i))
          (array-get (array-get sf:out.samples ci) i)))))
  (label exit
    (if sf-array
      (begin
        (sp-for-each-index i allocated
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
  (if a:data (sp-seq-events-prepare (convert-type &a:data sp-event-list-t**)))
  (set a:free sp-group-free)
  status-return)

(define (sp-group-prepare-parallel a) (status-t sp-event-t*)
  status-declare
  (sp-seq-events-prepare (convert-type &a:data sp-event-list-t**))
  (set a:free sp-group-free)
  status-return)

(define (sp-group-add a event) (status-t sp-event-t* sp-event-t)
  status-declare
  (if (not event.end) (sp-event-prepare-optional-srq event))
  (status-require (sp-event-list-add (convert-type &a:data sp-event-list-t**) event))
  (if (< a:end event.end) (set a:end event.end))
  (label exit status-return))

(define (sp-group-add-set group start duration event)
  (status-t sp-event-t* sp-time-t sp-time-t sp-event-t)
  "set event start and duration and add event to group"
  (struct-set event start start end (+ start (if* (= 0 duration) event.end duration)))
  (return (sp-group-add group event)))

(define (sp-group-append a event) (status-t sp-event-t* sp-event-t)
  status-declare
  (if (not event.end) (sp-event-prepare-optional-srq event))
  (set+ event.start a:end event.end a:end)
  (status-require (sp-group-add a event))
  (label exit status-return))

(define (sp-channel-config mute delay phs amp amod)
  (sp-channel-config-t sp-bool-t sp-time-t sp-time-t sp-sample-t sp-sample-t*)
  (declare a sp-channel-config-t)
  (struct-set a use #t mute mute delay delay phs phs amp amp amod amod)
  (return a))

(define (sp-channel-config-reset a) (void sp-channel-config-t*)
  (define channel-config sp-channel-config-t (struct-literal 0))
  (sp-for-each-index i sp-channel-limit
    (set (array-get a i) channel-config (struct-get (array-get a i) amp) 1)))

(define (sp-wave-event-config) sp-wave-event-config-t
  (declare result sp-wave-event-config-t)
  (struct-set result
    wvf sp-sine-table
    wvf-size sp-rate
    phs 0
    frq sp-rate
    fmod 0
    amp 1
    amod 0
    channel-count sp-channel-count)
  (sp-channel-config-reset result.channel-config)
  (return result))

(define (sp-wave-event-config-new out) (status-t sp-wave-event-config-t**)
  "heap allocates a sp_wave_event_config_t struct and set defaults"
  status-declare
  (status-require (sp-malloc-type 1 sp-wave-event-config-t out))
  (set **out (sp-wave-event-config))
  (label exit status-return))

(define (sp-wave-event-channel-free a) (void sp-event-t*)
  (set a:free 0)
  (free a:config)
  (free a:data))

(define (sp-wave-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare i sp-time-t sp sp-wave-event-state-t* s sp-wave-event-state-t)
  (set sp event:data s *sp)
  (for ((set i start) (< i end) (set+ i 1))
    (set+ (array-get out.samples s.channel (- i start))
      (* s.amp (array-get s.amod i) (array-get s.wvf s.phs)) s.phs
      (if* s.fmod (+ s.frq (array-get s.fmod i)) s.frq))
    (if (>= s.phs s.wvf-size) (set s.phs (modulo s.phs s.wvf-size))))
  (set sp:phs s.phs)
  status-return)

(define (sp-wave-event-channel duration config channel generate out)
  (status-t sp-time-t sp-wave-event-config-t sp-channel-count-t sp-event-generate-t sp-event-t*)
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
    amp (if* channel-config.use (* config.amp channel-config.amp) config.amp)
    amod (if* (and channel-config.use channel-config.amod) channel-config.amod config.amod)
    channel channel)
  (struct-set event
    data data
    start (if* channel-config.use channel-config.delay 0)
    end (if* channel-config.use (+ duration channel-config.delay) duration)
    prepare 0
    generate generate
    free sp-wave-event-channel-free)
  (set *out event)
  (label exit (if status-is-failure (free data)) status-return))

(define (sp-wave-event-prepare event) (status-t sp-event-t*)
  "prepare an event playing waveforms from an array.
   event end will be longer if channel config delay is used.
   expects sp_wave_event_config_t at event.config.
   config:
   * frq (frequency): fixed base frequency
   * fmod (frequency): array with hertz values that will add to frq
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
  (declare config sp-wave-event-config-t)
  (set config (pointer-get (convert-type event:config sp-wave-event-config-t*)))
  (sp-for-each-index ci config.channel-count
    (if (struct-get (array-get config.channel-config ci) mute) continue)
    (status-require
      (sp-wave-event-channel (- event:end event:start) config ci event:generate &channel))
    (status-require (sp-group-add event channel)))
  (sp-group-event event)
  (sp-event-pointer-prepare-srq event)
  (label exit (if status-is-failure (sp-event-pointer-free event)) status-return))

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
    resolution (/ sp-rate 1000)
    is-reject 0
    channel-count sp-channel-count)
  (sp-channel-config-reset result:channel-config)
  (set *out result)
  (label exit status-return))

(define (sp-noise-event-free a) (void sp-event-t*)
  (declare s sp-noise-event-state-t*)
  (set a:free 0 s a:data)
  (sp-convolution-filter-state-free s:filter-state)
  (free s)
  (sp-event-memory-free a))

(define (sp-noise-event-generate-block duration block-i event-i out event)
  (status-t sp-time-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare s sp-noise-event-state-t ampn sp-sample-t)
  (set s (pointer-get (convert-type event:data sp-noise-event-state-t*)))
  (sp-samples-random-primitive &s.random-state duration s.noise)
  (status-require
    (sp-windowed-sinc-bp-br s.noise duration
      (sp-optional-array-get s.cutl-mod s.cutl event-i)
      (sp-optional-array-get s.cuth-mod s.cuth event-i) s.trnl s.trnh
      s.is-reject &s.filter-state s.temp))
  (sp-for-each-index i duration
    (set ampn (* s.amp (array-get s.amod (+ event-i i)) (array-get s.temp i)))
    (set+ (array-get out.samples s.channel (+ block-i i)) (sp-inline-limit ampn -1 1)))
  (set (pointer-get (convert-type event:data sp-noise-event-state-t*)) s)
  (label exit status-return))

(define (sp-noise-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (return
    (sp-event-block-generate
      (struct-pointer-get (convert-type event:data sp-noise-event-state-t*) resolution)
      sp-noise-event-generate-block start end out event)))

(define (sp-noise-event-filter-state cutl cuth trnl trnh is-reject rs out)
  (status-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-bool-t sp-random-state-t* sp-convolution-filter-state-t**)
  "the result shows a small delay, for example, circa 40 samples for transition 0.07. the size seems to be related to ir-len.
   the filter state is initialised with one unused call to skip the delay."
  status-declare
  (declare ir-len sp-time-t temp sp-sample-t* noise sp-sample-t*)
  (memreg-init 2)
  (set ir-len (sp-windowed-sinc-lp-hp-ir-length (sp-inline-min trnl trnh)))
  (status-require (sp-malloc-type ir-len sp-sample-t &noise))
  (memreg-add noise)
  (status-require (sp-malloc-type ir-len sp-sample-t &temp))
  (memreg-add temp)
  (sp-samples-random-primitive rs ir-len noise)
  (status-require (sp-windowed-sinc-bp-br noise ir-len cutl cuth trnl trnh is-reject out temp))
  (label exit memreg-free status-return))

(define (sp-noise-event-channel duration config channel rs state-noise state-temp generate event)
  (status-t sp-time-t sp-noise-event-config-t sp-channel-count-t sp-random-state-t sp-sample-t* sp-sample-t* sp-event-generate-t sp-event-t*)
  status-declare
  (declare
    state sp-noise-event-state-t*
    channel-config sp-channel-config-t
    filter-state sp-convolution-filter-state-t*)
  (set state 0 filter-state 0 channel-config (array-get config.channel-config channel))
  (status-require
    (sp-noise-event-filter-state (sp-optional-array-get config.cutl-mod config.cutl 0)
      (sp-optional-array-get config.cuth-mod config.cuth 0) config.trnl config.trnh
      config.is-reject &rs &filter-state))
  (status-require (sp-malloc-type 1 sp-noise-event-state-t &state))
  (struct-pointer-set state
    amp (if* channel-config.use (* config.amp channel-config.amp) config.amp)
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
    generate generate
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
   all channel-count use the same initial random-state"
  status-declare
  (declare
    duration sp-time-t
    rs sp-random-state-t
    state-noise sp-sample-t*
    state-temp sp-sample-t*
    config sp-noise-event-config-t)
  (sp-declare-event channel)
  (set
    config (pointer-get (convert-type event:config sp-noise-event-config-t*))
    duration (- event:end event:start)
    rs (sp-random-state-new (sp-time-random-primitive &sp-random-state)))
  (if (and (or config.cutl-mod config.cuth-mod) (not (= duration config.resolution)))
    (set
      config.resolution (if* config.resolution config.resolution 96)
      config.resolution (sp-inline-min config.resolution duration))
    (set config.resolution duration))
  (status-require (sp-event-memory-ensure event 2))
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-noise))
  (sp-event-memory-fixed-add event state-noise)
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-temp))
  (sp-event-memory-fixed-add event state-temp)
  (sp-for-each-index ci config.channel-count
    (if (struct-get (array-get config.channel-config ci) mute) continue)
    (status-require
      (sp-noise-event-channel duration config ci rs state-noise state-temp event:generate &channel))
    (status-require (sp-group-add event channel)))
  (sp-group-event event)
  (sp-event-pointer-prepare-srq event)
  (label exit (if status-is-failure (sp-event-pointer-free event)) status-return))

(declare sp-cheap-noise-event-state-t
  (type
    (struct
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (cut sp-sample-t)
      (cut-mod sp-sample-t*)
      (q-factor sp-sample-t)
      (q-factor-mod sp-sample-t*)
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
    q-factor-mod 0
    passes 1
    type sp-state-variable-filter-lp
    resolution (/ sp-rate 10)
    channel-count sp-channel-count)
  (sp-channel-config-reset result:channel-config)
  (set *out result)
  (label exit status-return))

(define (sp-cheap-noise-event-free a) (void sp-event-t*)
  (declare s sp-cheap-noise-event-state-t*)
  (set a:free 0 s a:data)
  (sp-cheap-filter-state-free &s:filter-state)
  (free s)
  (sp-event-memory-free a))

(define (sp-cheap-noise-event-generate-block duration block-i event-i out event)
  (status-t sp-time-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare s sp-cheap-noise-event-state-t ampn sp-sample-t)
  (set s (pointer-get (convert-type event:data sp-cheap-noise-event-state-t*)))
  (sp-samples-random-primitive &s.random-state duration s.noise)
  (sp-cheap-filter s.type s.noise
    duration (sp-optional-array-get s.cut-mod s.cut event-i) s.passes
    (sp-optional-array-get s.q-factor-mod s.q-factor event-i) &s.filter-state s.temp)
  (sp-for-each-index i duration
    (set ampn (* s.amp (array-get s.amod (+ event-i i)) (array-get s.temp i)))
    (set+ (array-get out.samples s.channel (+ block-i i)) (sp-inline-limit ampn -1 1)))
  (set (pointer-get (convert-type event:data sp-cheap-noise-event-state-t*)) s)
  status-return)

(define (sp-cheap-noise-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (return
    (sp-event-block-generate
      (struct-pointer-get (convert-type event:data sp-cheap-noise-event-state-t*) resolution)
      sp-cheap-noise-event-generate-block start end out event)))

(define
  (sp-cheap-noise-event-channel duration config channel rs state-noise state-temp generate out)
  (status-t sp-time-t sp-cheap-noise-event-config-t sp-channel-count-t sp-random-state-t sp-sample-t* sp-sample-t* sp-event-generate-t sp-event-t*)
  status-declare
  (declare data sp-cheap-noise-event-state-t* channel-config sp-channel-config-t)
  (sp-declare-event event)
  (sp-declare-cheap-filter-state filter-state)
  (set data 0 channel-config (array-get config.channel-config channel))
  (status-require (sp-malloc-type 1 sp-cheap-noise-event-state-t &data))
  (status-require (sp-cheap-filter-state-new config.resolution config.passes &filter-state))
  (struct-set *data
    amp (if* channel-config.use (* config.amp channel-config.amp) config.amp)
    amod (if* (and channel-config.use channel-config.amod) channel-config.amod config.amod)
    cut config.cut
    cut-mod config.cut-mod
    type config.type
    passes config.passes
    q-factor config.q-factor
    q-factor-mod config.q-factor-mod
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
    generate generate
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
  (declare
    rs sp-random-state-t
    state-noise sp-sample-t*
    state-temp sp-sample-t*
    duration sp-time-t
    config sp-cheap-noise-event-config-t)
  (sp-declare-event channel)
  (set
    config (pointer-get (convert-type event:config sp-cheap-noise-event-config-t*))
    duration (- event:end event:start)
    config.passes (if* config.passes config.passes 1)
    rs (sp-random-state-new (sp-time-random-primitive &sp-random-state)))
  (if (and config.cut-mod (not (= duration config.resolution)))
    (set
      config.resolution (if* config.resolution config.resolution 96)
      config.resolution (sp-inline-min config.resolution duration))
    (set config.resolution duration))
  (status-require (sp-event-memory-ensure event 2))
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-noise))
  (sp-event-memory-fixed-add event state-noise)
  (status-require (sp-malloc-type config.resolution sp-sample-t &state-temp))
  (sp-event-memory-fixed-add event state-temp)
  (sp-for-each-index ci config.channel-count
    (if (struct-get (array-get config.channel-config ci) mute) continue)
    (status-require
      (sp-cheap-noise-event-channel (- event:end event:start) config
        ci rs state-noise state-temp event:generate &channel))
    (status-require (sp-group-add event channel)))
  (sp-group-event event)
  (sp-event-pointer-prepare-srq event)
  (label exit status-return))

(define (sp-map-event-config-new out) (status-t sp-map-event-config-t**)
  status-declare
  (declare result sp-map-event-config-t*)
  (status-require (sp-malloc-type 1 sp-map-event-config-t &result))
  (struct-set *result state 0 isolate 0)
  (set *out result)
  (label exit status-return))

(define (sp-map-event-free event) (void sp-event-t*)
  (declare s sp-map-event-state-t*)
  (set event:free 0 s event:data)
  (sp-event-free s:event)
  (free s)
  (sp-event-memory-free event))

(define (sp-map-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare s sp-map-event-state-t*)
  (set s event:data)
  (status-require (s:event.generate start end out &s:event))
  (status-require (s:map-generate start end out out s:state))
  (label exit status-return))

(define (sp-map-event-isolated-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  "creates temporary output, lets event write to it, and passes the temporary output to a user function"
  status-declare
  (declare temp-out sp-block-t s sp-map-event-state-t*)
  (set s event:data)
  (status-require (sp-block-new out.channel-count out.size &temp-out))
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
  (declare state sp-map-event-state-t* config sp-map-event-config-t)
  (error-memory-init 1)
  (set config (pointer-get (convert-type event:config sp-map-event-config-t*)))
  (status-require (sp-malloc-type 1 sp-map-event-state-t &state))
  (error-memory-add state)
  (sp-event-prepare-optional-srq config.event)
  (set
    state:event config.event
    state:state config.state
    state:map-generate config.map-generate
    event:data state
    event:free sp-map-event-free)
  (label exit (if status-is-failure error-memory-free) status-return))

(define (sp-sound-event-config) sp-sound-event-config-t
  "return a sound event configuration struct with defaults set"
  (declare result sp-sound-event-config-t)
  (struct-set result
    noise 0
    amp 1
    amod 0
    frq 0
    fmod 0
    wdt (/ sp-rate 2)
    wmod 0
    phs 0
    channel-count sp-channel-count)
  (sp-channel-config-reset result.channel-config)
  (return result))

(define (sp-sound-event-config-new out) (status-t sp-sound-event-config-t**)
  "heap allocates a sp_wave_event_config_t struct and set defaults"
  status-declare
  (status-require (sp-malloc-type 1 sp-sound-event-config-t out))
  (set **out (sp-sound-event-config))
  (label exit status-return))

(define (sp-sound-event-config-new-n count out) (status-t sp-size-t sp-sound-event-config-t**)
  status-declare
  (declare default-value sp-sound-event-config-t)
  (status-require (sp-malloc-type count sp-sound-event-config-t out))
  (set default-value (sp-sound-event-config))
  (sp-for-each-index i count (set (array-get *out i) default-value))
  (label exit status-return))

(define (sp-sound-event-prepare-cheap-noise event) (status-t sp-event-t*)
  status-declare
  (declare
    cut sp-sample-t
    q-factor sp-sample-t
    cut-mod sp-sample-t*
    q-factor-mod sp-sample-t*
    frq sp-time-t
    wdt sp-time-t
    duration sp-time-t
    event-config sp-cheap-noise-event-config-t*
    config sp-sound-event-config-t)
  (set
    config (pointer-get (convert-type event:config sp-sound-event-config-t*))
    duration (- event:end event:start)
    cut (sp-hz->factor config.frq)
    q-factor (sp-hz->factor config.wdt)
    cut-mod 0
    q-factor-mod 0)
  (if (or config.fmod config.wmod)
    (begin
      (srq (sp-event-memory-ensure event 3))
      (srq (sp-samples-new duration &cut-mod))
      (sp-event-memory-fixed-add event cut-mod)
      (srq (sp-samples-new duration &q-factor-mod))
      (sp-event-memory-fixed-add event q-factor-mod)
      (sp-for-each-index i duration
        (set
          frq (if* config.fmod (+ config.frq (array-get config.fmod i)) config.frq)
          wdt (if* config.wmod (+ config.wdt (array-get config.wmod i)) config.wdt)
          (array-get cut-mod i) (sp-hz->factor (- frq (/ wdt 2)))
          (array-get q-factor-mod i) (sp-hz->factor wdt))))
    (srq (sp-event-memory-ensure event 1)))
  (srq (sp-cheap-noise-event-config-new &event-config))
  (sp-event-memory-fixed-add event event-config)
  (struct-pointer-set event-config
    amp config.amp
    amod config.amod
    cut cut
    cut-mod cut-mod
    q-factor q-factor
    q-factor-mod q-factor-mod
    type sp-state-variable-filter-bp)
  (sp-for-each-index i sp-channel-limit
    (set (array-get event-config:channel-config i) (array-get config.channel-config i)))
  (sp-cheap-noise-event event event-config)
  (sp-event-pointer-prepare-srq event)
  (label exit status-return))

(define (sp-sound-event-prepare-noise event) (status-t sp-event-t*)
  "each sound event copies fmod and wmod if used"
  status-declare
  (declare
    duration sp-time-t
    cutl-mod sp-sample-t*
    cuth-mod sp-sample-t*
    cutl sp-sample-t
    cuth sp-sample-t
    frq sp-time-t
    wdt sp-time-t
    event-config sp-noise-event-config-t*
    config sp-sound-event-config-t)
  (set
    config (pointer-get (convert-type event:config sp-sound-event-config-t*))
    duration (- event:end event:start)
    cutl (sp-hz->factor config.frq)
    cuth (sp-hz->factor (+ config.wdt config.frq))
    cutl-mod 0
    cuth-mod 0)
  (if (or config.fmod config.wmod)
    (begin
      (sp-event-memory-ensure event 3)
      (srq (sp-samples-new duration &cutl-mod))
      (sp-event-memory-fixed-add event cutl-mod)
      (srq (sp-samples-new duration &cuth-mod))
      (sp-event-memory-fixed-add event cuth-mod)
      (sp-for-each-index i duration
        (set
          frq (if* config.fmod (+ config.frq (array-get config.fmod i)) config.frq)
          wdt (if* config.wmod (+ config.wdt (array-get config.wmod i)) config.wdt)
          (array-get cutl-mod i) (sp-hz->factor frq)
          (array-get cuth-mod i) (sp-hz->factor (+ frq wdt)))))
    (srq (sp-event-memory-ensure event 1)))
  (srq (sp-noise-event-config-new &event-config))
  (sp-event-memory-fixed-add event event-config)
  (struct-pointer-set event-config
    amp config.amp
    amod config.amod
    cutl-mod cutl-mod
    cuth-mod cuth-mod
    cutl cutl
    cuth cuth)
  (sp-channel-config-copy config.channel-config event-config:channel-config)
  (sp-noise-event event event-config)
  (sp-event-pointer-prepare-srq event)
  (label exit status-return))

(define (sp-sound-event-prepare-wave-event event) (status-t sp-event-t*)
  status-declare
  (declare event-config sp-wave-event-config-t* config sp-sound-event-config-t)
  (set config (pointer-get (convert-type event:config sp-sound-event-config-t*)))
  (srq (sp-wave-event-config-new &event-config))
  (srq (sp-event-memory-ensure event 1))
  (sp-event-memory-fixed-add event event-config)
  (struct-pointer-set event-config
    amp config.amp
    amod config.amod
    frq config.frq
    fmod config.fmod
    phs config.phs)
  (sp-channel-config-copy config.channel-config event-config:channel-config)
  (sp-wave-event event event-config)
  (sp-event-pointer-prepare-srq event)
  (label exit status-return))

(define (sp-sound-event-prepare event) (status-t sp-event-t*)
  "generic event for waves and filtered noise. all frequency values (frq, fmod, wdt, wmod) are in hertz, even for filtered noise"
  status-declare
  (declare config sp-sound-event-config-t)
  (set config (pointer-get (convert-type event:config sp-sound-event-config-t*)))
  (cond ((= 1 config.noise) (srq (sp-sound-event-prepare-noise event)))
    ((= 2 config.noise) (srq (sp-sound-event-prepare-cheap-noise event)))
    (else (srq (sp-sound-event-prepare-wave-event event))))
  (label exit (if status-is-failure (sp-event-pointer-free event)) status-return))