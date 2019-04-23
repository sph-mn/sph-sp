(sc-comment
  "create discrete 2d paths interpolated between given points.
  * maps between one independent discrete value to one dependent continuous value
  * multidimensional interpolation can be archieved with multiple configs and calls. time is the only dependent variable
  * a copy of segments is made internally and only the copy is used
  * uses points as structs because pre-defined size arrays can not be used in structs")

(declare
  sp-path-count-t (type uint16-t)
  sp-path-value-t (type double)
  sp-path-point-t
  (type
    (struct
      (x sp-path-value-t)
      (y sp-path-value-t)))
  sp-path-interpolator-t
  (type
    (function-pointer
      void
      sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*))
  sp-path-segment-t
  (type
    (struct
      (_start sp-path-point-t)
      (points-len sp-path-count-t)
      (points sp-path-point-t*)
      (interpolator sp-path-interpolator-t)
      (options-len sp-path-count-t)
      (options void*)))
  sp-path-t
  (type
    (struct
      (segments-len sp-path-count-t)
      (segments sp-path-segment-t*))))

(define (sp-path-i-line start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  (declare
    i sp-sample-count-t
    p-end sp-path-point-t
    size sp-sample-count-t
    factor sp-path-value-t)
  (set
    p-end (array-get p-rest 0)
    size (- p-end.x p-start.x))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set
      factor (/ i size)
      (array-get out i) (+ (* p-end.y factor) (* p-start.y (- 1 factor))))))

(define (sp-path-free a) (void sp-path-t) (free a.segments))

(define (sp-path-new segments-len segments out-path)
  (status-t sp-path-count-t sp-path-segment-t* sp-path-t*)
  status-declare
  (declare
    s sp-path-segment-t
    i sp-path-count-t
    start sp-path-point-t
    path sp-path-t)
  (set
    path.segments 0
    start.x 0
    start.y 0)
  (status-require (sph-helper-malloc (* segments-len (sizeof sp-path-segment-t)) &path.segments))
  (memcpy path.segments segments (* segments-len (sizeof sp-path-segment-t)))
  (for ((set i 0) (< i segments-len) (set i (+ 1 i)))
    (set
      s (array-get path.segments i)
      s._start start
      start (array-get s.points (- s.points-len 1))))
  (set *out-path path)
  (label exit
    (if (and status-is-failure path.segments) (free path.segments))
    (return status)))

(define (sp-path-get path start end out)
  (void sp-path-t sp-sample-count-t sp-sample-count-t sp-path-value-t*)
  (declare
    i sp-path-count-t
    s sp-path-segment-t
    s-start sp-sample-count-t
    s-end sp-sample-count-t)
  (for ((set i 0) (< i path.segments-len) (set i (+ 1 i)))
    (set
      s (array-get path.segments i)
      s-start s._start.x)
    (if (> s-start start) break)
    (set s-end (struct-get (array-get s.points (- s.points-len 1)) x))
    (s.interpolator start (min s-end end) s._start s.points s.options (+ start out))
    (set s-start s-end))
  (sc-comment "last segment ended before requested end")
  (if (> end s-start) (memset (+ s-start out) 0 (- end s-start))))

(define (sp-path-new-get segments-len segments start end out)
  (void sp-path-count-t sp-path-segment-t* sp-sample-count-t sp-sample-count-t sp-path-value-t*)
  "create a path array immediately from segments without creating a path object"
  (declare
    s sp-path-segment-t
    i sp-path-count-t
    p-start sp-path-point-t
    s-start sp-sample-count-t
    s-end sp-sample-count-t)
  (set
    p-start.x 0
    p-start.y 0)
  (for ((set i 0) (< i segments-len) (set i (+ 1 i)))
    (set
      s (array-get segments i)
      s-start p-start.x)
    (if (> s-start start) break)
    (set s-end (struct-get (array-get s.points (- s.points-len 1)) x))
    (s.interpolator start (min s-end end) s._start s.points s.options (+ start out))
    (set p-start (array-get s.points (- s.points-len 1))))
  (if (> end s-start) (memset (+ s-start out) 0 (- end s-start))))