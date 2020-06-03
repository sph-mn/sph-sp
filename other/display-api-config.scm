; config to be used by exe/list-bindings.
; for generated bindings

(add
  ; additional expressions to include. for example for preprocessor generated identifiers
  (declare
    sp-samples-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-times-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-path-segments-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-events-t (type (struct (data void*) (size size-t) (used size-t) (current size-t)))))
