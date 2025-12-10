(sc-comment
  "event.free functions must set event.free to null.
   event.prepare and event.generate must be user replaceable.
   event.prepare is to be set to null by callers of event.prepare")

(define (sp-event-list-display-element list) (void sp-event-list-t*)
  (printf "%p <- %p -> %p event %zu %zu\n" (convert-type list:previous void*)
    (convert-type list void*) (convert-type list:next void*) (convert-type list:value.start size-t)
    (convert-type list:value.end size-t)))

(define (sp-event-list-remove head-pointer list) (void sp-event-list-t** sp-event-list-t*)
  (if (not list) return)
  (sp-event-list-unlink head-pointer list)
  (if list:value.free (list:value.free &list:value))
  (free list))

(define (sp-event-list-add head-pointer event) (status_t sp-event-list-t** sp-event-t)
  status-declare
  (declare current sp-event-list-t* new sp-event-list-t*)
  (status-require (sp-malloc-type 1 sp-event-list-t &new))
  (set new:value event current *head-pointer)
  (if current
    (if (<= current:value.start event.start)
      (set new:previous 0 new:next current current:previous new *head-pointer new)
      (begin
        (while (and current:next (> current:next:value.start event.start))
          (set current current:next))
        (set new:next current:next new:previous current)
        (if current:next (set current:next:previous new))
        (set current:next new)))
    (set new:previous 0 new:next 0 *head-pointer new))
  (label exit status-return))

(define (sp-event-list-free head-pointer) (void sp-event-list-t**)
  (declare current sp-event-list-t* next sp-event-list-t*)
  (set current *head-pointer)
  (while current
    (if current:value.free (current:value.free &current:value))
    (set next current:next)
    (free current)
    (set current next))
  (set *head-pointer 0))

(define (sp-event-block-generate resolution generate start end out event)
  (status-t sp-time-t sp-event-block-generate-t sp-time-t sp-time-t void* sp-event-t*)
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

(define (sp-seq start end out events) (status-t sp-time-t sp-time-t void* sp-event-list-t**)
  "event arrays must have been prepared/sorted once with sp_seq_event_prepare for seq to work correctly.
   like for paths, start is inclusive, end is exclusive, so that 0..100 and 100..200 attach seamless.
   events can have three function pointers: prepare, generate and free.
   generate receives event relative start/end times, and output blocks that start at the current block.
   prepare will only be called once and the function pointer will be set to zero afterwards.
   if prepare fails, sp_seq returns immediately.
   past events including the event list elements are freed when processing the following block.
   on error, all events and the event list are freed"
  status-declare
  (declare
    e sp-event-t
    ep sp-event-t*
    next sp-event-list-t*
    current sp-event-list-t*
    offset sp-time-t
    relative-start sp-time-t
    relative-end sp-time-t
    shifted sp-block-t)
  (set current *events)
  (while current
    (set ep &current:value e *ep)
    (cond
      ( (<= e.end start) (set next current:next) (sp-event-list-remove events current)
        (set current next) continue)
      ((<= end e.start) break)
      ( (> e.end start) (sp-event-prepare-optional-srq e)
        (set
          *ep e
          offset (if* (> e.start start) (- e.start start) 0)
          relative-start (+ offset (- start e.start))
          relative-end (if* (< (- end e.start) (- e.end e.start)) (- end e.start) (- e.end e.start))
          shifted (sp-block-with-offset (pointer-get (convert-type out sp-block-t*)) offset))
        (status-require (e.generate relative-start relative-end &shifted ep))))
    (set current current:next))
  (label exit (if status-is-failure (sp-event-list-free events)) status-return))

(define (sp-group-free group) (void sp-event-t*)
  (set group:free 0)
  (sp-event-list-free (sp-group-event-list group))
  (sp-event-memory-free group))

(define (sp-group-generate start end out group) (status-t sp-time-t sp-time-t void* sp-event-t*)
  (return (sp-seq start end out (convert-type &group:config sp-event-list-t**))))

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
  (status-t sp-time-t sp-time-t void* sp-event-t*)
  status-declare
  (define c sp-map-event-config-t* event:config)
  (status-require (c:event.generate start end out &c:event))
  (status-require (c:map-generate start end out out c:config))
  (label exit status-return))

(define (sp-map-event-isolated-generate start end out event)
  (status-t sp-time-t sp-time-t void* sp-event-t*)
  "creates temporary output, lets event write to it, and passes the temporary output to a user function"
  status-declare
  (sp-declare-block temp-out)
  (declare c sp-map-event-config-t*)
  (set c event:config)
  (status-require
    (sp-block-new (struct-pointer-get (convert-type out sp-block-t*) channel-count)
      (struct-pointer-get (convert-type out sp-block-t*) size) &temp-out))
  (status-require (c:event.generate start end &temp-out &c:event))
  (status-require (c:map-generate start end &temp-out out c:config))
  (label exit (sp-block-free &temp-out) status-return))

(define (sp-resonator-event-config-defaults) sp-resonator-event-config-t
  (declare
    out sp-resonator-event-config-t
    channel-index sp-size-t
    channel-config sp-resonator-event-channel-config-t*)
  (set
    out.random-state (sp-random-state-new (sp-time-random-primitive &sp-random-state))
    out.resolution sp-default-resolution
    out.noise-in 0
    out.noise-out 0
    out.bandwidth-threshold 0.0
    out.channel-count sp-channel-count-limit
    channel-index 0)
  (while (< channel-index sp-channel-count-limit)
    (set
      channel-config (+ out.channel-config channel-index)
      channel-config:amod 0
      channel-config:amp 1.0
      channel-config:channel channel-index
      channel-config:filter-state 0
      channel-config:frq 0.0
      channel-config:fmod 0
      channel-config:wdt 0.0
      channel-config:wmod 0
      channel-config:phs 0
      channel-config:pmod 0
      channel-config:wvf sp-sine-table
      channel-config:wvf-size sp-rate
      channel-config:use (= channel-index 0)
      channel-index (+ channel-index 1)))
  (return out))

(define (sp-resonator-event-config-new-n count out)
  (status-t sp-time-t sp-resonator-event-config-t**)
  status-declare
  (declare defaults-value sp-resonator-event-config-t index sp-time-t)
  (set defaults-value (sp-resonator-event-config-defaults))
  (status-require (sp-malloc-type count sp-resonator-event-config-t out))
  (set index 0)
  (while (< index count) (set (array-get *out index) defaults-value index (+ index 1)))
  (label exit status-return))

(define (sp-resonator-event-free event) (void sp-event-t*)
  (declare
    config sp-resonator-event-config-t*
    channel-config sp-resonator-event-channel-config-t*
    channel-index sp-channel-count-t)
  (set config event:config)
  (if (not config) (return))
  (set channel-index 0)
  (while (< channel-index config:channel-count)
    (set channel-config (+ config:channel-config channel-index))
    (if channel-config:filter-state
      (begin
        (sp-convolution-filter-state-free channel-config:filter-state)
        (set channel-config:filter-state 0)))
    (set channel-index (+ channel-index 1)))
  (if config:noise-in (begin (free config:noise-in) (set config:noise-in 0)))
  (if config:noise-out (begin (free config:noise-out) (set config:noise-out 0))))

(define (sp-resonator-event-prepare event) (status-t sp-event-t*)
  status-declare
  (declare
    config sp-resonator-event-config-t*
    base-channel sp-resonator-event-channel-config-t*
    channel-config sp-resonator-event-channel-config-t*
    channel-index sp-channel-count-t
    duration sp-time-t
    temp-length sp-time-t
    keep-channel sp-channel-count-t)
  (set config event:config base-channel config:channel-config)
  (if (or (not base-channel:amod) (not base-channel:wvf) (<= base-channel:wvf-size 0))
    (sp-status-set-goto sp-s-id-invalid-argument))
  (set duration (- event:end event:start))
  (if (<= config:resolution 0) (set config:resolution duration))
  (set temp-length config:resolution)
  (if (> temp-length (convert-type (* sp-render-block-seconds sp-rate) sp-time-t))
    (set temp-length (convert-type (* sp-render-block-seconds sp-rate) sp-time-t)))
  (if (<= temp-length 0) (set temp-length 1))
  (set config:random-state (sp-random-state-new (sp-time-random-primitive &sp-random-state)))
  (status-require (sp-malloc-type temp-length sp-sample-t &config:noise-in))
  (status-require (sp-malloc-type temp-length sp-sample-t &config:noise-out))
  (set channel-index 0)
  (while (< channel-index config:channel-count)
    (set channel-config (+ config:channel-config channel-index))
    (if (not channel-config:use)
      (set
        keep-channel channel-config:channel
        *channel-config *base-channel
        channel-config:channel keep-channel)
      (begin
        (if (not channel-config:amod) (set channel-config:amod base-channel:amod))
        (if (not channel-config:wvf)
          (set channel-config:wvf base-channel:wvf channel-config:wvf-size base-channel:wvf-size))
        (if (<= channel-config:wvf-size 0)
          (set channel-config:wvf base-channel:wvf channel-config:wvf-size base-channel:wvf-size))
        (if (not channel-config:fmod) (set channel-config:fmod base-channel:fmod))
        (if (not channel-config:wmod) (set channel-config:wmod base-channel:wmod))
        (if (not channel-config:pmod) (set channel-config:pmod base-channel:pmod))))
    (set channel-config:filter-state 0 channel-index (+ channel-index 1)))
  (label exit status-return))

(define (sp-resonator-event-generate-block duration block-i event-i out event)
  (status-t sp-time-t sp-time-t sp-time-t void* sp-event-t*)
  status-declare
  (declare
    config sp-resonator-event-config-t*
    channel-config sp-resonator-event-channel-config-t*
    channel-value sp-resonator-event-channel-config-t
    channel-index sp-channel-count-t
    noise-in sp-sample-t*
    noise-out sp-sample-t*
    frq-value sp-frq-t
    bandwidth-value sp-frq-t
    cutoff-low-factor sp-sample-t
    cutoff-high-factor sp-sample-t
    transition-factor sp-sample-t
    arguments-buffer (array uint8-t ((* 3 (sizeof sp-sample-t))))
    arguments-length uint8-t
    sample-index sp-time-t
    phase-value sp-time-t
    amplitude-value sp-sample-t
    sine-value sp-sample-t
    noise-value sp-sample-t
    out-value sp-sample-t)
  (set config event:config noise-in config:noise-in noise-out config:noise-out)
  (sp-samples-random-primitive &config:random-state duration noise-in)
  (set channel-index 0)
  (while (< channel-index config:channel-count)
    (set channel-config (+ config:channel-config channel-index) channel-value *channel-config)
    (if (not channel-value.use) (begin (set+ channel-index 1) continue))
    (set
      frq-value (sp-optional-array-get channel-value.fmod channel-value.frq event-i)
      bandwidth-value (sp-optional-array-get channel-value.wmod channel-value.wdt event-i))
    (if (< bandwidth-value config:bandwidth-threshold)
      (begin
        (set sample-index 0)
        (while (< sample-index duration)
          (set phase-value channel-value.phs)
          (if channel-value.pmod
            (begin
              (set+ phase-value (array-get channel-value.pmod (+ event-i sample-index)))
              (if (>= phase-value channel-value.wvf-size)
                (set- phase-value
                  (* channel-value.wvf-size (floor (/ phase-value channel-value.wvf-size)))))))
          (set
            amplitude-value
            (* channel-value.amp (array-get channel-value.amod (+ event-i sample-index)))
            sine-value (array-get channel-value.wvf (convert-type phase-value sp-time-t))
            out-value (* amplitude-value sine-value))
          (set+
            (array-get (struct-pointer-get (convert-type out sp-block-t*) samples)
              channel-value.channel (+ block-i sample-index))
            (sp-inline-limit out-value -1 1))
          (set channel-value.phs (+ channel-value.phs frq-value))
          (if (>= channel-value.phs channel-value.wvf-size)
            (set- channel-value.phs
              (* channel-value.wvf-size (floor (/ channel-value.phs channel-value.wvf-size)))))
          (set+ sample-index 1))
        (set channel-config:phs channel-value.phs))
      (begin
        (set
          cutoff-low-factor (sp-hz-to-factor (- frq-value (* 0.5 bandwidth-value)))
          cutoff-high-factor (sp-hz-to-factor (+ frq-value (* 0.5 bandwidth-value))))
        (if (< cutoff-low-factor 0.0) (set cutoff-low-factor 0.0))
        (if (> cutoff-high-factor 0.5) (set cutoff-high-factor 0.5))
        (set
          transition-factor (* 0.5 (sp-hz-to-factor bandwidth-value))
          arguments-length (* 3 (sizeof sp-sample-t))
          (pointer-get (convert-type arguments-buffer sp-sample-t*)) cutoff-low-factor
          (pointer-get (+ (convert-type arguments-buffer sp-sample-t*) 1)) cutoff-high-factor
          (pointer-get (+ (convert-type arguments-buffer sp-sample-t*) 2)) transition-factor)
        (status-require
          (sp-convolution-filter noise-in duration
            sp-resonator-ir-f arguments-buffer arguments-length
            &channel-config:filter-state noise-out))
        (set sample-index 0)
        (while (< sample-index duration)
          (set
            amplitude-value
            (* channel-value.amp (array-get channel-value.amod (+ event-i sample-index)))
            noise-value (array-get noise-out sample-index)
            out-value (* amplitude-value noise-value))
          (set+
            (array-get (struct-pointer-get (convert-type out sp-block-t*) samples)
              channel-value.channel (+ block-i sample-index))
            (sp-inline-limit out-value -1 1))
          (set+ sample-index 1))))
    (set+ channel-index 1))
  (label exit status-return))

(define (sp-resonator-event-generate start end out event)
  (status-t sp-time-t sp-time-t void* sp-event-t*)
  (return
    (sp-event-block-generate
      (struct-pointer-get (convert-type event:config sp-resonator-event-config-t*) resolution)
      sp-resonator-event-generate-block start end out event)))
