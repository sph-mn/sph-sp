(sc-comment
  "create discrete 2d paths interpolated between given points.
  * maps between one independent discrete value to one dependent continuous value
  * multidimensional interpolation can be archieved with multiple configs and calls. time is the only dependent variable
  * a copy of segments is made internally and only the copy is used
  * uses points as structs because pre-defined size arrays can not be used in structs
  * segments-len must be greater than zero
  * segments must be a valid segment configuration")

(define (sp-path-i-line start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  (declare
    factor sp-path-value-t
    i sp-sample-count-t
    p-end sp-path-point-t
    size sp-sample-count-t)
  (set
    p-end (array-get p-rest 0)
    size (- p-end.x p-start.x))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set
      factor (/ i size)
      (array-get out i) (+ (* p-end.y factor) (* p-start.y (- 1 factor))))))

(define (sp-path-i-move start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*))

(define (sp-path-i-constant start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*))

(define (sp-path-i-bezier start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*))

(define (sp-path-i-catmull-rom start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*))

(define (sp-path-i-path start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*))

(define (sp-path-free a) (void sp-path-t) (free a.segments))

(define (sp-path-new segments-len segments out-path)
  (status-t sp-path-segment-count-t sp-path-segment-t* sp-path-t*)
  status-declare
  (declare
    s sp-path-segment-t
    i sp-path-segment-count-t
    start sp-path-point-t
    path sp-path-t)
  (set
    path.segments 0
    s (array-get segments 0))
  (if
    (or (= sp-path-i-move s.interpolator) (and (= sp-path-i-constant s.interpolator) s.points-len))
    (set start (array-get s.points 0))
    (set
      start.x 0
      start.y 0))
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
    i sp-path-segment-count-t
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
  (void
    sp-path-segment-count-t sp-path-segment-t* sp-sample-count-t sp-sample-count-t sp-path-value-t*)
  "create a path array immediately from segments without creating a path object"
  (declare
    s sp-path-segment-t
    i sp-path-segment-count-t
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