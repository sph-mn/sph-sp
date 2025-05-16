(pre-define
  sp-seq-events-prepare sp-event-list-reverse
  sp-default-resolution (if* (< sp-rate 10000) sp-rate (/ sp-rate 1000))
  (sp-event-reset x) (set x sp-null-event)
  (sp-declare-event id) (begin (define id sp-event-t (struct-literal 0)) (set id.memory.data 0))
  (sp-declare-event-list id) (define id sp-event-list-t* 0)
  (sp-event-duration a) (- a.end a.start)
  (sp-group-event-list event) (convert-type (address-of (: event config)) sp-event-list-t**)
  (sp-event-free a) (if a.free (a.free &a))
  (sp-event-prepare-srq a) (begin (status-require (a.prepare &a)) (set a.prepare 0))
  (sp-event-prepare-optional-srq a) (if a.prepare (sp-event-prepare-srq a))
  sp-event-memory-array-add array3-add
  (sp-event-memory-add event address) (sp-event-memory-add-with-handler event address free)
  (sp-event-memory-fixed-add event address)
  (sp-event-memory-fixed-add-with-handler event address free)
  (sp-wave-event-config-new out) (sp-wave-event-config-new-n 1 out)
  (sp-map-event-config-new out) (sp-map-event-config-new-n 1 out)
  (sp-noise-event-config-new out) (sp-noise-event-config-new-n 1 out)
  (sp-define-event name _prepare duration)
  (begin
    "use case: event variables defined at the top-level"
    (define
      name sp-event-t
      (struct-literal (prepare _prepare) (start 0)
        (end duration) (config 0) (memory (struct-literal 0)))))
  (sp-wave-event event-pointer _config)
  (struct-pointer-set event-pointer
    prepare sp-wave-event-prepare
    generate sp-wave-event-generate
    config _config)
  (sp-noise-event event-pointer _config)
  (struct-pointer-set event-pointer
    prepare sp-noise-event-prepare
    generate sp-noise-event-generate
    config _config)
  (sp-map-event event-pointer _config)
  (struct-pointer-set event-pointer
    prepare sp-map-event-prepare
    generate (if* _config:isolate sp-map-event-isolated-generate sp-map-event-generate)
    config _config)
  (sp-group-event event-pointer)
  (struct-pointer-set event-pointer prepare sp-group-prepare generate sp-group-generate config 0)
  (sp-group-parallel-event event-pointer)
  (struct-pointer-set event-pointer
    prepare sp-group-prepare
    generate sp-group-generate-parallel
    config 0)
  (sp-event-path-samples-srq event-pointer out ...)
  (begin
    (status-require (sp-path-samples out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-samples3-srq event-pointer out ...)
  (begin
    (status-require (sp-path-samples3 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-samples4-srq event-pointer out ...)
  (begin
    (status-require (sp-path-samples4 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-samples5-srq event-pointer out ...)
  (begin
    (status-require (sp-path-samples5 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-samples-curve3-srq event-pointer out ...)
  (begin
    (status-require (sp-path-samples-curve3 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-samples-curve4-srq event-pointer out ...)
  (begin
    (status-require (sp-path-samples-curve4 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-samples-curve5-srq event-pointer out ...)
  (begin
    (status-require (sp-path-samples-curve5 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-times-srq event-pointer out ...)
  (begin
    (status-require (sp-path-times out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-times3-srq event-pointer out ...)
  (begin
    (status-require (sp-path-times3 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-times4-srq event-pointer out ...)
  (begin
    (status-require (sp-path-times4 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-times5-srq event-pointer out ...)
  (begin
    (status-require (sp-path-times5 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-times-curve3-srq event-pointer out ...)
  (begin
    (status-require (sp-path-times-curve3 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-times-curve4-srq event-pointer out ...)
  (begin
    (status-require (sp-path-times-curve4 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-path-times-curve5-srq event-pointer out ...)
  (begin
    (status-require (sp-path-times-curve5 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-zero-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-zero out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-zero3-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-zero3 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-zero4-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-zero4 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-zero5-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-zero5 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-zero-curve3-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-zero-curve3 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-zero-curve4-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-zero-curve4 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-zero-curve5-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-zero-curve5 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-scale-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-scale out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-scale3-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-scale3 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-scale4-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-scale4 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-scale5-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-scale5 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-scale-curve3-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-scale-curve3 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-scale-curve4-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-scale-curve4 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-envelope-scale-curve5-srq event-pointer out ...)
  (begin
    (status-require (sp-envelope-scale-curve5 out __VA_ARGS__))
    (status-require (sp-event-memory-add event-pointer *out)))
  (sp-event-alloc-srq event-pointer allocator size pointer-address)
  (begin
    (status-require (allocator size pointer-address))
    (status-require (sp-event-memory-add event-pointer (pointer-get pointer-address))))
  (sp-event-malloc-srq event-pointer size pointer-address)
  (begin
    (status-require (sph-helper-malloc size pointer-address))
    (status-require (sp-event-memory-add event-pointer (pointer-get pointer-address))))
  (sp-event-malloc-type-n-srq event-pointer count type pointer-address)
  (sp-event-malloc-srq event-pointer (* count (sizeof type)) pointer-address)
  (sp-event-malloc-type-srq event-pointer type pointer-address)
  (sp-event-malloc-type-n-srq event-pointer 1 type pointer-address)
  (sp-event-samples-srq event-pointer size pointer-address)
  (sp-event-alloc-srq event-pointer sp-samples-new size pointer-address)
  (sp-event-times-srq event-pointer size pointer-address)
  (sp-event-alloc-srq event-pointer sp-times-new size pointer-address)
  (sp-event-units-srq event-pointer size pointer-address)
  (sp-event-alloc-srq event-pointer sp-units-new size pointer-address)
  (sp-event-config-get a type) (pointer-get (convert-type a.config type*)))

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
      (config void*)
      (memory sp-memory-t)))
  sp-event-generate-t (type (function-pointer status-t sp-time-t sp-time-t sp-block-t sp-event-t*))
  sp-event-prepare-t (type (function-pointer status-t sp-event-t*))
  sp-map-generate-t
  (type (function-pointer status-t sp-time-t sp-time-t sp-block-t sp-block-t void*))
  sp-event-list-t
  (type
    (struct
      sp-event-list-struct
      (previous (struct sp-event-list-struct*))
      (next (struct sp-event-list-struct*))
      (event sp-event-t)))
  sp-event-block-generate-t
  (type (function-pointer status-t sp-time-t sp-time-t sp-time-t sp-block-t sp-event-t*))
  sp-map-event-config-t
  (type
    (struct (config void*) (event sp-event-t) (map-generate sp-map-generate-t) (isolate sp-bool-t)))
  sp-wave-event-channel-config-t
  (type
    (struct
      (amod sp-sample-t*)
      (amp sp-sample-t)
      (channel sp-channel-count-t)
      (fmod sp-time-t*)
      (frq sp-frq-t)
      (phs sp-time-t)
      (pmod sp-time-t*)
      (use sp-bool-t)))
  sp-wave-event-config-t
  (type
    (struct
      (wvf sp-sample-t*)
      (wvf-size sp-time-t)
      (channel-count sp-channel-count-t)
      (channel-config (array sp-wave-event-channel-config-t sp-channel-count-limit))))
  sp-noise-event-channel-config-t
  (type
    (struct
      (amp sp-sample-t)
      (amod sp-sample-t*)
      (channel sp-channel-count-t)
      (filter-state sp-convolution-filter-state-t*)
      (frq sp-frq-t)
      (fmod sp-time-t*)
      (wdt sp-frq-t)
      (wmod sp-time-t*)
      (trnl sp-frq-t)
      (trnh sp-frq-t)
      (use sp-bool-t)))
  sp-noise-event-config-t
  (type
    (struct
      (is-reject sp-bool-t)
      (random-state sp-random-state-t)
      (resolution sp-time-t)
      (temp (array sp-sample-t* 3))
      (channel-count sp-channel-count-t)
      (channel-config (array sp-noise-event-channel-config-t sp-channel-count-limit)))))

(define sp-null-event sp-event-t (struct-literal 0))

(declare
  (sp-event-list-add a event) (status-t sp-event-list-t** sp-event-t)
  (sp-event-list-display a) (void sp-event-list-t*)
  (sp-event-list-free events) (void sp-event-list-t**)
  (sp-event-list-remove a element) (void sp-event-list-t** sp-event-list-t*)
  (sp-event-list-reverse a) (void sp-event-list-t**)
  (sp-event-list-validate a) (void sp-event-list-t*)
  (sp-event-memory-add-with-handler event address handler)
  (status-t sp-event-t* void* sp-memory-free-t)
  (sp-event-memory-ensure a additional-size) (status-t sp-event-t* sp-time-t)
  (sp-event-memory-fixed-add-with-handler event address handler)
  (void sp-event-t* void* sp-memory-free-t)
  (sp-event-memory-free event) (void sp-event-t*)
  (sp-group-add a event) (status-t sp-event-t* sp-event-t)
  (sp-group-append a event) (status-t sp-event-t* sp-event-t)
  (sp-group-event-free a) (void sp-event-t*)
  (sp-group-event-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-event-parallel-f start end out event) (void sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-free group) (void sp-event-t*)
  (sp-group-generate-parallel start end out a) (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-generate start end out a) (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-group-prepare event) (status-t sp-event-t*)
  (sp-group-prepare-parallel a) (status-t sp-event-t*)
  (sp-map-event-config-new-n count out) (status-t sp-time-t sp-map-event-config-t**)
  (sp-map-event-free event) (void sp-event-t*)
  (sp-map-event-generate start end out event) (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-map-event-isolated-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-map-event-prepare event) (status-t sp-event-t*)
  (sp-noise-event-config-defaults) sp-noise-event-config-t
  (sp-noise-event-config-new-n count out) (status-t sp-time-t sp-noise-event-config-t**)
  (sp-noise-event-free event) (void sp-event-t*)
  (sp-noise-event-generate start end out event) (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-noise-event-prepare event) (status-t sp-event-t*)
  (sp-seq-parallel start end out events) (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  (sp-seq start end out events) (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  (sp-wave-event-config-defaults) sp-wave-event-config-t
  (sp-wave-event-config-new-n count out) (status-t sp-time-t sp-wave-event-config-t**)
  (sp-wave-event-generate start end out event) (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (sp-wave-event-prepare event) (status-t sp-event-t*)
  (sp-event-schedule event onset duration config) (sp-event-t sp-event-t sp-time-t sp-time-t void*))