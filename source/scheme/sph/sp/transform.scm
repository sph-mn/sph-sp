(library (sph sp transform)
  (export)
  (import
    (sph))

  (define* (sp-moving-average source prev next distance #:optional start end)
    "f32vector false/f32vector false/f32vector integer [integer/false integer/false] -> f32vector"
    (f32vector-copy-empty* source
      (l (a) (sp-moving-average! a source prev next distance start end)))))
