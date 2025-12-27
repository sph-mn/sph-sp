(pre-define sp-path-curve-strength 12.0)

(declare
  sp-path-t (type (struct (t sp-time-t*) (values (array sp-sample-t* 2)) (point-count sp-time-t)))
  (sp-path-get path start end out cursor)
  (void sp-path-t* sp-time-t sp-time-t sp-sample-t* sp-time-t*))
