; configuration used by exe/display-api.

(add
  ; additional expressions to include. for example for preprocessor generated identifiers
  (declare sp-samples-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-times-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-path-segments-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-events-t (type (struct (data void*) (size size-t) (used size-t) (current size-t)))
    (sp-event-memory-add event address) (status-t sp-event-t* void*)
    (sp-event-memory-fixed-add event address) (status-t sp-event-t* void*)))

(remove "__*")
