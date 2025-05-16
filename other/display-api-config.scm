; configuration used by exe/display-api.

(add
  ; additional expressions to include
  (declare sp-samples-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-times-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-path-segments-t (type (struct (data void*) (size size-t) (used size-t)))
    sp-events-t (type (struct (data void*) (size size-t) (used size-t) (current size-t)))))

(remove "__.*" "arrays-template-.*")
