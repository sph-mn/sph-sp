(define (sp-seq-parallel-future-entry data) (void* void*)
  (define future sp-seq-future-t* data)
  (set future:status (future:run future))
  (return 0))

(define (sp-seq-parallel-generic start end out events config)
  (status-t sp-time-t sp-time-t void* sp-event-list-t** sp-seq-parallel-generic-config-t*)
  status-declare
  (declare
    current sp-event-list-t*
    next sp-event-list-t*
    ep sp-event-t*
    e sp-event-t
    futures sp-seq-future-t*
    future sp-seq-future-t*
    count sp-size-t
    index sp-size-t
    relative-start sp-time-t
    relative-end sp-time-t
    output-start sp-time-t)
  (set current *events count 0 futures 0)
  (while current
    (if (<= end current:event.start) (set current 0)
      (begin (if (> current:event.end start) (set+ count 1)) (set current current:next))))
  (status-require (sp-calloc-type count sp-seq-future-t &futures))
  (set current *events index 0)
  (while current
    (set ep &current:event e *ep)
    (if (<= e.end start)
      (begin (set next current:next) (sp-event-list-remove events current) (set current next))
      (if (<= end e.start) (set current 0)
        (begin
          (set
            relative-start (if* (> start e.start) (- start e.start) 0)
            relative-end (- (sp-inline-min end e.end) e.start)
            output-start (if* (> e.start start) (- e.start start) 0)
            future (+ futures index)
            future:start relative-start
            future:end relative-end
            future:out-start output-start
            future:event ep
            future:status.id status-id-success
            future:run config:run)
          (if config:make
            (status-require (config:make (- relative-end relative-start) out &future:out))
            (set future:out out))
          (sph-future-new sp-seq-parallel-future-entry future &future:future)
          (set+ index 1)
          (set current current:next)))))
  (sp-for-each-index k index
    (set future (+ futures k))
    (sph-future-touch &future:future)
    (status-require future:status))
  (if config:merge (status-require (config:merge start end out futures index config:context)))
  (label exit
    (if futures
      (begin
        (if config:free
          (sp-for-each-index k index
            (set future (+ futures k))
            (if (and config:make (!= future:out out)) (config:free future:out))))
        (free futures)))
    (if status-is-failure (sp-event-list-free events))
    status-return))

(define (sp-block-future-make count parent-out product-out) (status-t sp-time-t void* void**)
  status-declare
  (declare parent-block sp-block-t* bp sp-block-t*)
  (set parent-block parent-out bp 0)
  (status-require (sp-malloc-type 1 sp-block-t &bp))
  (status-require (sp-block-new parent-block:channel-count count bp))
  (set *product-out bp)
  (label exit (if (and status-is-failure bp) (free bp)) status-return))

(define (sp-block-future-free product) (void void*)
  (if (not product) return)
  (declare bp sp-block-t*)
  (set bp product)
  (sp-block-free bp)
  (free bp))

(define (sp-block-future-run future) (status-t sp-seq-future-t*)
  status-declare
  (declare ep sp-event-t* e sp-event-t bp sp-block-t*)
  (set ep future:event e *ep bp future:out)
  (sp-event-prepare-optional-srq e)
  (set *ep e)
  (status-require (e.generate future:start future:end (convert-type bp void*) ep))
  (label exit status-return))

(define (sp-block-merge-all start end parent-out futures count context)
  (status-t sp-time-t sp-time-t void* sp-seq-future-t* sp-size-t void*)
  status-declare
  (declare out-block sp-block-t*)
  (set out-block parent-out)
  (sp-for-each-index k count
    (declare f sp-seq-future-t* b sp-block-t*)
    (set f (+ futures k) b f:out)
    (sp-for-each-index ci out-block:channel-count
      (sp-for-each-index i b:size
        (set (array-get (array-get out-block:samples ci) (+ f:out-start i))
          (+ (array-get (array-get out-block:samples ci) (+ f:out-start i))
            (array-get (array-get b:samples ci) i))))))
  status-return)

(define (sp-seq-parallel-block start end out events)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  status-declare
  (declare c sp-seq-parallel-generic-config-t)
  (set
    c.make sp-block-future-make
    c.free sp-block-future-free
    c.merge sp-block-merge-all
    c.run sp-block-future-run
    c.context 0)
  (status-require (sp-seq-parallel-generic start end &out events &c))
  (label exit status-return))

(define (sp-group-generate-parallel-block start end out group)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  (return (sp-seq-parallel-block start end out (convert-type &group:config sp-event-list-t**))))
