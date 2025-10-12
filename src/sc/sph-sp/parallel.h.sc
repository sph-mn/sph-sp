(pre-define (sp-group-parallel-block-event event-pointer)
  (struct-pointer-set event-pointer
    prepare sp-group-prepare
    generate sp-group-generate-parallel-block
    config 0))

(declare
  sp-seq-future-t struct
  sp-seq-future-t (type (sc-insert "struct sp_seq_future_t"))
  sp-seq-future-make (type (function-pointer status-t sp-time-t void* void**))
  sp-seq-future-free (type (function-pointer void void*))
  sp-seq-future-merge
  (type (function-pointer status-t sp-time-t sp-time-t void* sp-seq-future-t* sp-size-t void*))
  sp-seq-future-run (type (function-pointer status-t sp-seq-future-t*))
  sp-seq-future-t
  (struct
    (start sp-time-t)
    (end sp-time-t)
    (out-start sp-time-t)
    (out void*)
    (event sp-event-t*)
    (status status-t)
    (future sph-future-t)
    (run sp-seq-future-run))
  sp-seq-parallel-generic-config-t
  (type
    (struct
      (make sp-seq-future-make)
      (free sp-seq-future-free)
      (merge sp-seq-future-merge)
      (run sp-seq-future-run)
      (context void*)))
  (sp-seq-parallel-generic start end out events config)
  (status-t sp-time-t sp-time-t void* sp-event-list-t** sp-seq-parallel-generic-config-t*)
  (sp-seq-parallel-block start end out events)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-list-t**)
  (sp-group-generate-parallel-block start end out a)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*))
