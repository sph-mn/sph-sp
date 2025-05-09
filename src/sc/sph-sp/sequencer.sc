(sc-comment
  "event.free functions must set event.free to null.
   event.prepare and event.generate must be user replaceable.
   event.prepare is to be set to null by callers of event.prepare")

(pre-define sp-event-memory-growth-factor 2 sp-event-memory-initial-size 4)

(define (sp-event-list-display-element a) (void sp-event-list-t*)
  (printf "%lu %lu %lu event %lu %lu\n" a:previous a a:next a:event.start a:event.end))

(define (sp-event-list-display a) (void sp-event-list-t*)
  (while a (sp-event-list-display-element a) (set a a:next)))

(define (sp-event-list-reverse a) (void sp-event-list-t**)
  (declare current sp-event-list-t* next sp-event-list-t*)
  (set next *a)
  (while next (set current next next next:next current:next current:previous current:previous next))
  (set *a current))

(define (sp-event-list-find-duplicate a b) (void sp-event-list-t* sp-event-list-t*)
  (define i sp-time-t 0 count sp-time-t 0)
  (while a
    (if (= a b)
      (if (= 1 count) (begin (printf "duplicate list entry %p at index %lu\n" a i) (exit 1))
        (set+ count 1)))
    (set+ i 1)
    (set a a:next)))

(define (sp-event-list-validate a) (void sp-event-list-t*)
  (define i sp-time-t 0 b sp-event-list-t* a c sp-event-list-t* 0)
  (while b
    (if (not (= c b:previous))
      (begin (printf "link to previous is invalid at index %lu, element %p\n" i b) (exit 1)))
    (if (and (= b:next b:previous) (not (= 0 b:next)))
      (begin (printf "circular list entry at index %lu, element %p\n" i b) (exit 1)))
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
  (set *events 0))

(define (sp-event-memory-add-with-handler a address handler)
  (status-t sp-event-t* void* sp-memory-free-t)
  "event memory addition with automatic array expansion"
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
  (sp-event-memory-array-add a:memory m)
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
  "depends on the event memory array to already have enough free space.
   no needed return status check, no free space checks or resizing"
  (declare m memreg2-t)
  (struct-set m address address handler handler)
  (sp-event-memory-array-add a:memory m))

(define (sp-event-memory-free a) (void sp-event-t*)
  "free all registered memory and unitialize the event-memory register"
  (if (not a:memory.data) return)
  (declare m memreg2-t)
  (sp-for-each-index i (array3-size a:memory) (set m (array3-get a:memory i)) (m.handler m.address))
  (array3-free a:memory)
  (set a:memory.data 0))

(define (sp-event-block-generate resolution generate start end out event)
  (status-t sp-time-t sp-event-block-generate-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  "calls generate for sub-blocks of at most size resolution"
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

(define (sp-event-schedule event onset duration config)
  (sp-event-t sp-event-t sp-time-t sp-time-t void*)
  "assumes that event start is set to the beginning, for example 0. copies event"
  (set+ event.start onset event.end (+ onset duration))
  (if config (set event.config config))
  (return event))

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
  (sp-event-prepare-optional-srq *ep)
  (status-require (ep:generate a:start a:end a:out ep))
  (label exit (set a:status status) (return 0)))

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

(define (sp-group-free group) (void sp-event-t*)
  (set group:free 0)
  (sp-event-list-free (sp-group-event-list group))
  (sp-event-memory-free group))

(define (sp-group-generate start end out group)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (return (sp-seq start end out (convert-type &group:config sp-event-list-t**))))

(define (sp-group-generate-parallel start end out group)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (return (sp-seq-parallel start end out (convert-type &group:config sp-event-list-t**))))

(define (sp-group-prepare group) (status-t sp-event-t*)
  status-declare
  (if group:config (sp-seq-events-prepare (convert-type &group:config sp-event-list-t**)))
  (set group:free sp-group-free)
  status-return)

(define (sp-group-add group event) (status-t sp-event-t* sp-event-t)
  "events with end zero will be prepared immediately for cases when event.prepare sets end"
  status-declare
  (if (not event.end) (sp-event-prepare-optional-srq event))
  (status-require (sp-event-list-add (convert-type &group:config sp-event-list-t**) event))
  (if (< group:end event.end) (set group:end event.end))
  (label exit status-return))

(define (sp-group-append group event) (status-t sp-event-t* sp-event-t)
  status-declare
  (if (not event.end) (sp-event-prepare-optional-srq event))
  (set+ event.start group:end event.end group:end)
  (set group:end event.end)
  (status-require (sp-event-list-add (convert-type &group:config sp-event-list-t**) event))
  (label exit status-return))

(define (sp-map-event-config-new-n count out) (status-t sp-time-t sp-map-event-config-t**)
  (return (sp-calloc-type count sp-map-event-config-t out)))

(define (sp-map-event-prepare event) (status-t sp-event-t*)
  "the wrapped event will be freed with the map-event.
   use cases: filter chains, post processing.
   config:
     map-generate: map function (start end sp_block_t:in sp_block_t:out void*:state)
     config: custom state value passed to f.
     isolate: use a dedicated output buffer for event
       events can be wrapped in multiple sp_map_event with an isolated sp_map_event on top that
       finally writes to main out to mix with other events"
  status-declare
  (declare c sp-map-event-config-t*)
  (set c event:config)
  (sp-event-prepare-optional-srq c:event)
  (set event:free sp-map-event-free)
  (label exit status-return))

(define (sp-map-event-free event) (void sp-event-t*)
  (declare c sp-map-event-config-t*)
  (set event:free 0 c event:config)
  (sp-event-free c:event)
  (sp-event-memory-free event))

(define (sp-map-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare c sp-map-event-config-t*)
  (set c event:config)
  (status-require (c:event.generate start end out &c:event))
  (status-require (c:map-generate start end out out c:config))
  (label exit status-return))

(define (sp-map-event-isolated-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  "creates temporary output, lets event write to it, and passes the temporary output to a user function"
  status-declare
  (sp-declare-block temp-out)
  (declare c sp-map-event-config-t*)
  (set c event:config)
  (status-require (sp-block-new out.channel-count out.size &temp-out))
  (status-require (c:event.generate start end temp-out &c:event))
  (status-require (c:map-generate start end temp-out out c:config))
  (label exit (sp-block-free &temp-out) status-return))

(define (sp-wave-event-config-defaults) sp-wave-event-config-t
  (define out sp-wave-event-config-t (struct-literal 0))
  (struct-set out
    channel-config:use 1
    channel-count sp-channel-count-limit
    wvf sp-sine-table
    wvf-size sp-rate)
  (sp-for-each-index i sp-channel-count-limit
    (struct-set (array-get out.channel-config i) amp 1 channel i))
  (return out))

(define (sp-wave-event-config-new-n count out) (status-t sp-time-t sp-wave-event-config-t**)
  "allocate sp_wave_event_config_t and set to defaults"
  status-declare
  (define defaults sp-wave-event-config-t (sp-wave-event-config-defaults))
  (status-require (sp-malloc-type count sp-wave-event-config-t out))
  (sp-for-each-index i count (set (array-get *out i) defaults))
  (label exit status-return))

(define (sp-wave-event-prepare event) (status-t sp-event-t*)
  "prepare an event playing waveforms from an array.
   event end will be longer if channel config delay is used.
   expects sp_wave_event_config_t at event.config.
   config:
   * frq (frequency): fixed base frequency. added to fmod if fmod is set
   * fmod (frequency): array with hertz values that will add to frq. note that it currently does not support negative frequencies
   * wvf (waveform): array with waveform samples
   * wvf-size: count of waveform samples
   * phs (phase): initial phase offset
   * pmod (phase): phase offset per sample
   * amp (amplitude): multiplied with amod
   * amod (amplitude): array with sample values"
  status-declare
  (declare c sp-wave-event-config-t* cc sp-wave-event-channel-config-t* ci sp-channel-count-t)
  (set c event:config)
  (for ((define i sp-channel-count-t 1) (< i c:channel-count) (set+ i 1))
    (set cc (+ c:channel-config i))
    (if cc:use
      (begin
        (sp-inline-default cc:amod c:channel-config:amod)
        (sp-inline-default cc:pmod c:channel-config:pmod)
        (sp-inline-default cc:fmod c:channel-config:fmod)
        (sp-inline-default cc:frq c:channel-config:frq))
      (set ci cc:channel *cc *c:channel-config cc:channel ci)))
  status-return)

(define (sp-wave-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare
    c sp-wave-event-config-t*
    cc sp-wave-event-channel-config-t
    ccp sp-wave-event-channel-config-t*
    phsn sp-time-t)
  (set c event:config)
  (for ((define cci sp-channel-count-t 0) (< cci c:channel-count) (set+ cci 1))
    (set ccp (+ c:channel-config cci) cc *ccp)
    (for ((define i sp-time-t start) (< i end) (set+ i 1))
      (if cc.pmod
        (begin
          (set phsn (+ cc.phs (array-get cc.pmod i)))
          (if (>= phsn c:wvf-size) (set phsn (modulo phsn c:wvf-size))))
        (set phsn cc.phs))
      (set+ (array-get out.samples cc.channel (- i start))
        (* cc.amp (array-get cc.amod i) (array-get c:wvf phsn)) cc.phs
        (if* cc.fmod (+ cc.frq (array-get cc.fmod i)) cc.frq))
      (if (>= cc.phs c:wvf-size) (set cc.phs (modulo cc.phs c:wvf-size))))
    (set ccp:phs cc.phs))
  status-return)

(define (sp-noise-event-config-defaults) sp-noise-event-config-t
  (define out sp-noise-event-config-t (struct-literal 0))
  (struct-set out
    channel-config:use 1
    channel-count sp-channel-count-limit
    resolution sp-default-resolution)
  (sp-for-each-index i sp-channel-count-limit
    (struct-set (array-get out.channel-config i)
      amp 1
      channel i
      wdt sp-max-frq
      trnl (* sp-rate 0.08)
      trnh (* sp-rate 0.08)))
  (return out))

(define (sp-noise-event-config-new-n count out) (status-t sp-time-t sp-noise-event-config-t**)
  status-declare
  (define defaults sp-noise-event-config-t (sp-noise-event-config-defaults))
  (status-require (sp-malloc-type count sp-noise-event-config-t out))
  (sp-for-each-index i count (set (array-get *out i) defaults))
  (label exit status-return))

(define (sp-noise-event-filter-state c cc ir-length)
  (status-t sp-noise-event-config-t* sp-noise-event-channel-config-t* sp-time-t)
  "the initial filter output shows a small delay of circa 40 samples for transition 0.07. the size seems to be related to ir-len.
   the filter state is initialized with one unused call to skip the delay."
  (declare frqn sp-frq-t)
  (sp-samples-random-primitive &c:random-state ir-length *c:temp)
  (set frqn (sp-optional-array-get cc:fmod cc:frq 0))
  (return
    (sp-windowed-sinc-bp-br *c:temp ir-length
      (sp-hz->factor frqn) (sp-hz->factor (+ frqn (sp-optional-array-get cc:wmod cc:wdt 0)))
      (sp-hz->factor cc:trnl) (sp-hz->factor cc:trnh) c:is-reject
      &cc:filter-state (array-get c:temp 1))))

(define (sp-noise-event-free event) (void sp-event-t*)
  (declare c sp-noise-event-config-t* cc sp-noise-event-channel-config-t*)
  (set event:free 0 c event:config)
  (sp-for-each-index i c:channel-count
    (set cc (+ c:channel-config i))
    (if cc:filter-state (sp-convolution-filter-state-free cc:filter-state)))
  (free (array-get c:temp 0))
  (free (array-get c:temp 1))
  (free (array-get c:temp 2))
  (sp-event-memory-free event))

(define (sp-noise-event-prepare event) (status-t sp-event-t*)
  "an event for noise filtered by a windowed-sinc filter.
   very processing intensive if parameters change with low resolution.
   event:config.resolution is how often the filter parameters should be updated.
   filter parameters are updated at least for each generate call"
  status-declare
  (declare
    cc sp-noise-event-channel-config-t*
    ci sp-channel-count-t
    c sp-noise-event-config-t*
    duration sp-time-t
    filter-mod sp-bool-t
    filter-states sp-bool-t
    ir-lengths (array sp-time-t sp-channel-count-limit 0)
    temp-length sp-time-t)
  (error-memory-init (+ 3 sp-channel-count-limit))
  (set
    c event:config
    c:random-state (sp-random-state-new (sp-time-random-primitive &sp-random-state))
    cc c:channel-config
    duration (- event:end event:start)
    filter-mod (or cc:fmod cc:wmod)
    filter-states 0
    temp-length (sp-windowed-sinc-lp-hp-ir-length (sp-hz->factor (sp-inline-min cc:trnl cc:trnh)))
    (array-get ir-lengths 0) temp-length)
  (sc-comment "the first channel is always required")
  (if (not c:channel-config:amod) (sp-status-set-goto sp-s-id-invalid-argument))
  (sc-comment "set channel defaults and collect size information for temporary buffers")
  (for ((define cci sp-channel-count-t 1) (< cci c:channel-count) (set+ cci 1))
    (set cc (+ c:channel-config cci))
    (if cc:use
      (begin
        (if (not cc:amod) (set cc:amod c:channel-config:amod))
        (if (or cc:frq cc:wdt cc:fmod cc:wmod cc:trnl cc:trnh)
          (begin
            (set filter-states 1)
            (if (or cc:fmod cc:wmod) (set filter-mod 1))
            (if (not cc:fmod) (set cc:fmod c:channel-config:fmod))
            (if (not cc:trnh) (set cc:trnh c:channel-config:trnh))
            (if (not cc:trnl) (set cc:trnl c:channel-config:trnl))
            (if (not cc:wdt) (set cc:wdt c:channel-config:wdt))
            (if (not cc:wmod) (set cc:wmod c:channel-config:wmod))
            (set (array-get ir-lengths cci)
              (sp-windowed-sinc-lp-hp-ir-length (sp-hz->factor (sp-inline-min cc:trnl cc:trnh))))
            (if (< temp-length (array-get ir-lengths cci))
              (set temp-length (array-get ir-lengths cci))))))
      (set ci cc:channel *cc *c:channel-config cc:channel ci)))
  (sc-comment "no updates necessary if parameters do not change")
  (if (not filter-mod) (set c:resolution duration))
  (sc-comment "this limits the largest block that can be safely requested")
  (set
    temp-length (sp-inline-max temp-length c:resolution)
    temp-length (sp-inline-min temp-length (* sp-render-block-seconds sp-rate)))
  (sc-comment
    "three buffers: one for the white noise, one for the first channel filter result that may be shared with other channels,
     and one for other channel filter results")
  (status-require (sp-malloc-type temp-length sp-sample-t c:temp))
  (error-memory-add (array-get c:temp 0))
  (status-require (sp-malloc-type temp-length sp-sample-t (+ 1 c:temp)))
  (error-memory-add (array-get c:temp 1))
  (sc-comment "allocate and initialize the filter-states using the buffers allocated above")
  (set cc c:channel-config)
  (status-require (sp-noise-event-filter-state c cc (array-get ir-lengths 0)))
  (error-memory-add2 cc:filter-state sp-convolution-filter-state-free)
  (if filter-states
    (begin
      (status-require (sp-malloc-type temp-length sp-sample-t (+ 2 c:temp)))
      (error-memory-add (array-get c:temp 2))
      (for ((define cci sp-channel-count-t 1) (< cci c:channel-count) (set+ cci 1))
        (set cc (+ c:channel-config cci))
        (if (array-get ir-lengths cci)
          (begin
            (status-require (sp-noise-event-filter-state c cc (array-get ir-lengths cci)))
            (error-memory-add2 cc:filter-state sp-convolution-filter-state-free))))))
  (label exit (if status-is-failure error-memory-free) status-return))

(define (sp-noise-event-generate-block duration block-i event-i out event)
  (status-t sp-time-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare
    c sp-noise-event-config-t*
    cc sp-noise-event-channel-config-t
    ccp sp-noise-event-channel-config-t*
    frqn sp-frq-t
    outn sp-sample-t
    temp sp-sample-t*)
  (set c event:config)
  (sp-samples-random-primitive &c:random-state duration *c:temp)
  (for ((define cci sp-channel-count-t 0) (< cci c:channel-count) (set+ cci 1))
    (set ccp (+ c:channel-config cci) cc *ccp)
    (if (and cc.use cc.filter-state)
      (begin
        (set
          temp (array-get c:temp (+ 1 (< 0 cci)))
          frqn (sp-optional-array-get cc.fmod cc.frq event-i))
        (status-require
          (sp-windowed-sinc-bp-br *c:temp duration
            (sp-hz->factor frqn)
            (sp-hz->factor (+ frqn (sp-optional-array-get cc.wmod cc.wdt event-i)))
            (sp-hz->factor cc.trnl) (sp-hz->factor cc.trnh) c:is-reject &ccp:filter-state temp)))
      (set temp (array-get c:temp 1)))
    (sp-for-each-index i duration
      (set outn (* cc.amp (array-get cc.amod (+ event-i i)) (array-get temp i)))
      (set+ (array-get out.samples cc.channel (+ block-i i)) (sp-inline-limit outn -1 1))))
  (label exit status-return))

(define (sp-noise-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (return
    (sp-event-block-generate
      (struct-pointer-get (convert-type event:config sp-noise-event-config-t*) resolution)
      sp-noise-event-generate-block start end out event)))