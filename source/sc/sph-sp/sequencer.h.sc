(pre-define
  (sp-event-reset x) (set x sp-event-null)
  (sp-declare-event id) (begin (define id sp-event-t (struct-literal 0)) (set id.memory.data 0))
  (sp-declare-event-2 id1 id2) (begin (sp-declare-event id1) (sp-declare-event id2))
  (sp-declare-event-3 id1 id2 id3)
  (begin (sp-declare-event id1) (sp-declare-event id2) (sp-declare-event id3))
  (sp-declare-event-4 id1 id2 id3 id4)
  (begin (sp-declare-event-2 id1 id2) (sp-declare-event-2 id3 id4))
  (sp-declare-group id) (begin (sp-declare-event id) (set id.prepare sp-group-prepare))
  (sp-declare-group-parallel id)
  (begin (sp-declare-event id) (set id.prepare sp-group-prepare-parallel))
  (sp-declare-event-list id) (define id sp-event-list-t* 0)
  (sp-event-duration a) (- a.end a.start)
  (sp-event-duration-set a duration) (set a.end (+ a.start duration))
  (sp-event-move a start) (set a.end (+ start (- a.end a.start)) a.start start)
  sp-group-size-t uint16-t
  (sp-event-memory-add event address) (sp-event-memory-add2 event address free)
  (sp-event-memory-add-2 a data1 data2)
  (begin (sp-event-memory-add a data1) (sp-event-memory-add a data2))
  (sp-event-memory-add-3 a data1 data2 data3)
  (begin (sp-event-memory-add-2 a data1 data2) (sp-event-memory-add a data3))
  sp-sine-config-t sp-wave-event-config-t
  sp-memory-add array3-add
  sp-seq-events-prepare sp-event-list-reverse
  (free-event-on-error event-address) (free-on-error (: event-address free) event-address)
  (free-event-on-exit event-address) (free-on-exit (: event-address free) event-address)
  (sp-group-event-list event) (convert-type (address-of (: event data)) sp-event-list-t**)
  (sp-event-free a) (if a.free (a.free &a))
  (sp-event-pointer-free a) (if a:free (a:free a))
  (sp-define-event name _prepare duration)
  (begin
    "use case: event variables defined at the top-level"
    (define name sp-event-t
      (struct-literal (prepare _prepare) (start 0)
        (end duration) (data 0) (memory (struct-literal 0)))))
  (sp-event-memory-malloc event count type pointer-address)
  (begin
    "allocated memory with malloc, save address in pointer at pointer-address,
     and also immediately add the memory to event memory to be freed with event.free"
    (sp-malloc-type count type pointer-address)
    (sp-event-memory-add _event *pointer-address))
  (sp-event-config-load variable-name type event)
  (define variable-name type (pointer-get (convert-type event:config type*)))
  (sp-sound-event event-pointer _config)
  (struct-pointer-set event-pointer prepare sp-sound-event-prepare config _config)
  (sp-wave-event event-pointer _config)
  (struct-pointer-set event-pointer prepare sp-wave-event-prepare config _config)
  (sp-noise-event event-pointer _config)
  (struct-pointer-set event-pointer prepare sp-noise-event-prepare config _config)
  (sp-cheap-noise-event event-pointer _config)
  (struct-pointer-set event-pointer prepare sp-cheap-noise-event-prepare config _config)
  (sp-group-event event-pointer) (struct-pointer-set event-pointer prepare sp-group-prepare))

(array3-declare-type sp-memory memreg2-t)

(declare
  sp-memory-free-t (type (function-pointer void void*))
  sp-event-t struct
  sp-event-t
  (type
    (struct
      sp-event-t
      (start sp-time-t)
      (end sp-time-t)
      (generate (function-pointer status-t sp-time-t sp-time-t sp-block-t (struct sp-event-t*)))
      (prepare (function-pointer status-t (struct sp-event-t*)))
      (free (function-pointer void (struct sp-event-t*)))
      (data void*)
      (config void*)
      (memory sp-memory-t)))
  sp-event-generate-t (type (function-pointer status-t sp-time-t sp-time-t sp-block-t sp-event-t*))
  sp-event-list-t
  (type
    (struct
      sp-event-list-struct
      (previous (struct sp-event-list-struct*))
      (next (struct sp-event-list-struct*))
      (event sp-event-t)))
  sp-channel-config-t
  (type
    (struct
      (use sp-bool-t)
      (mute sp-bool-t)
      (delay sp-time-t)
      (phs sp-time-t)
      (amp sp-sample-t)
      (amod sp-sample-t*)))
  sp-wave-event-config-t
  (type
    (struct
      (wvf sp-sample-t*)
      (wvf-size sp-time-t)
      (phs sp-time-t)
      (frq sp-time-t)
      (fmod sp-time-t*)
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (channel-count sp-channel-count-t)
      (channel-config (array sp-channel-config-t sp-channel-limit))))
  sp-wave-event-state-t
  (type
    (struct
      (wvf sp-sample-t*)
      (wvf-size sp-time-t)
      (phs sp-time-t)
      (frq sp-time-t)
      (fmod sp-time-t*)
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (channel sp-channel-count-t)))
  sp-noise-event-config-t
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
      (channel-count sp-channel-count-t)
      (channel-config (array sp-channel-config-t sp-channel-limit))))
  sp-cheap-noise-event-config-t
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
      (random-state sp-random-state-t*)
      (resolution sp-time-t)
      (channel-count sp-channel-count-t)
      (channel-config (array sp-channel-config-t sp-channel-limit))))
  sp-sound-event-config-t
  (type
    (struct
      (noise sp-bool-t)
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (frq sp-time-t)
      (fmod sp-time-t*)
      (phs sp-time-t)
      (wdt sp-time-t)
      (wmod sp-time-t*)
      (channel-count sp-channel-count-t)
      (channel-config (array sp-channel-config-t sp-channel-limit))))
  sp-event-prepare-t (function-pointer status-t sp-event-t*)
  sp-map-generate-t
  (type (function-pointer status-t sp-time-t sp-time-t sp-block-t sp-block-t void*))
  sp-map-event-state-t
  (type (struct (event sp-event-t) (map-generate sp-map-generate-t) (state void*)))
  sp-map-event-config-t
  (type
    (struct (event sp-event-t) (map-generate sp-map-generate-t) (state void*) (isolate sp-bool-t)))
  (sp-channel-config-zero a) (void sp-channel-config-t*))

(define sp-event-null sp-event-t (struct-literal 0))

(declare
  (sp-event-list-display a) (void sp-event-list-t*)
  (sp-event-list-reverse a) (void sp-event-list-t**)
  (sp-event-list-validate a) (void sp-event-list-t*)
  (sp-event-list-remove-element a element) (void sp-event-list-t** sp-event-list-t*)
  (sp-event-list-add a event) (status-t sp-event-list-t** sp-event-t)
  (sp-event-list-free events) (void sp-event-list-t**)
  (sp-event-memory-ensure a additional-size) (status-t sp-event-t* sp-time-t)
  (sp-event-memory-add2 event address handler) (void sp-event-t* void* sp-memory-free-t)
  (sp-event-memory-free event) (void sp-event-t*)
  (sp-seq start end out events) (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  (sp-seq-parallel start end out events) (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  (sp-wave-event-prepare event) (status-t sp-event-t*)
  (sp-noise-event-prepare event) (status-t sp-event-t*)
  (sp-cheap-noise-event-prepare event) (status-t sp-event-t*)
  (sp-group-prepare event) (status-t sp-event-t*)
  (sp-group-prepare-parallel a) (status-t sp-event-t*)
  (sp-group-add a event) (status-t sp-event-t* sp-event-t)
  (sp-group-append a event) (status-t sp-event-t* sp-event-t)
  (sp-group-add-set group start duration event) (status-t sp-event-t* sp-time-t sp-time-t sp-event-t)
  (sp-group-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-event-parallel-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-event-free a) (void sp-event-t*)
  (sp-map-event-prepare event) (status-t sp-event-t*)
  (sp-channel-config mute delay phs amp amod)
  (sp-channel-config-t sp-bool-t sp-time-t sp-time-t sp-sample-t sp-sample-t*)
  (sp-group-free) (void sp-event-t*)
  (sp-wave-event-free) (void sp-event-t*)
  (sp-noise-event-free) (void sp-event-t*)
  (sp-cheap-noise-event-free) (void sp-event-t*)
  (sp-map-event-free) (void sp-event-t*)
  (sp-noise-event-config-new out) (status-t sp-noise-event-config-t**)
  (sp-cheap-noise-event-config-new out) (status-t sp-cheap-noise-event-config-t**)
  (sp-wave-event-config-new out) (status-t sp-wave-event-config-t**)
  (sp-map-event-config-new out) (status-t sp-map-event-config-t**)
  (sp-wave-event-config-defaults config) (void sp-wave-event-config-t*)
  (sp-sound-event-prepare event) (status-t sp-event-t*)
  (sp-sound-event-config-new out) (status-t sp-sound-event-config-t**))