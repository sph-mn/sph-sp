(sc-comment
  "* sp-path creates discrete 2d paths interpolated between given points
  * maps fro one independent discrete value to one dependent continuous value
  * only the dependent value is returned
  * multidimensional interpolation can be archieved with multiple configs and calls
  * a copy of segments is made internally and only the copy is used
  * uses points as structs because pre-defined size arrays can not be used in structs
  * segments-len must be greater than zero
  * segments must be a valid sp-path segment configuration
  * interpolators are called with path-relative start/end inside segment and with out positioned at the segment output
  * all segment types have a fixed number of points. line: 1, bezier: 3, move: 1, constant: 0, path: 0")

(define (sp-path-i-move start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 1"
  (memset out 0 (- end start))
  (set (array-get out (- end start 1)) (struct-get (array-get p-rest 0) y)))

(define (sp-path-i-constant start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 0"
  (declare i sp-sample-count-t)
  (for ((set i 0) (< i (- end start)) (set i (+ 1 i)))
    (set (array-get out i) p-start.y)))

(define (sp-path-i-line start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 1"
  (declare
    i sp-sample-count-t
    p-end sp-path-point-t
    s-size sp-sample-count-t
    s-start sp-sample-count-t
    t sp-path-value-t)
  (set
    p-end (array-get p-rest 0)
    s-size (- p-end.x p-start.x)
    s-start (- start p-start.x))
  (for ((set i 0) (< i (- end start)) (set i (+ 1 i)))
    (set
      t (/ (+ s-start i) s-size)
      (array-get out i) (+ (* p-end.y t) (* p-start.y (- 1 t))))))

(define (sp-path-i-bezier start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 3"
  (declare
    i sp-sample-count-t
    mt sp-path-value-t
    p-end sp-path-point-t
    s-size sp-sample-count-t
    s-start sp-sample-count-t
    t sp-path-value-t)
  (set
    p-end (array-get p-rest 2)
    s-size (- p-end.x p-start.x)
    s-start (- start p-start.x))
  (for ((set i 0) (< i (- end start)) (set i (+ 1 i)))
    (set
      t (/ (+ s-start i) s-size)
      mt (- 1 t)
      (array-get out i)
      (+
        (* p-start.y mt mt mt)
        (* (struct-get (array-get p-rest 0) y) 3 mt mt t)
        (* (struct-get (array-get p-rest 1) y) 3 mt t t) (* p-end.x t t t)))))

(define (sp-path-i-path start end p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 0. options is one sp-path-t"
  (sp-path-get
    (pointer-get (convert-type options sp-path-t*)) (- start p-start.x) (- end p-start.x) out))

(define (sp-path-start path) (sp-path-point-t sp-path-t)
  (declare
    p sp-path-point-t
    s sp-path-segment-t)
  (set s (array-get path.segments 0))
  (if (= sp-path-i-move s.interpolator) (set p (array-get s.points 0))
    (set
      p.x 0
      p.y 0))
  (return p))

(define (sp-path-end path) (sp-path-point-t sp-path-t)
  (declare s sp-path-segment-t)
  (set s (array-get path.segments (- path.segments-len 1)))
  (if (= sp-path-i-constant s.interpolator)
    (set s (array-get path.segments (- path.segments-len 2))))
  (return (array-get s.points (- s._points-len 1))))

(define (sp-path-new segments-len segments out-path)
  (status-t sp-path-segment-count-t sp-path-segment-t* sp-path-t*)
  status-declare
  (declare
    i sp-path-segment-count-t
    path sp-path-t
    s sp-path-segment-t
    start sp-path-point-t)
  (set s (array-get segments 0))
  (if (= sp-path-i-move s.interpolator) (set start (array-get s.points 0))
    (set
      start.x 0
      start.y 0))
  (status-require (sph-helper-malloc (* segments-len (sizeof sp-path-segment-t)) &path.segments))
  (memcpy path.segments segments (* segments-len (sizeof sp-path-segment-t)))
  (for ((set i 0) (< i segments-len) (set i (+ 1 i)))
    (set
      (struct-get (array-get path.segments i) _start) start
      s (array-get path.segments i)
      s._points-len (sp-path-interpolator-points-len s.interpolator))
    (set start
      (if* (= sp-path-i-path s.interpolator)
        (sp-path-end (pointer-get (convert-type s.options sp-path-t*)))
        (array-get s.points (- s._points-len 1)))))
  (set
    path.segments-len segments-len
    *out-path path)
  (label exit
    (return status)))

(define (sp-path-get path start end out)
  (void sp-path-t sp-sample-count-t sp-sample-count-t sp-path-value-t*)
  (declare
    i sp-path-segment-count-t
    s sp-path-segment-t
    s-start sp-sample-count-t
    s-end sp-sample-count-t
    t sp-path-segment-count-t)
  (set t start)
  (for ((set i 0) (< i path.segments-len) (set i (+ 1 i)))
    (set
      s (array-get path.segments i)
      s-start s._start.x)
    (if (< end s-start) break)
    (set s-end (struct-get (array-get s.points (- s._points-len 1)) x))
    (if (> t s-end) continue)
    (s.interpolator t (min s-end end) s._start s.points s.options (+ (- t start) out))
    (set t s-end))
  (if (> end t) (memset (+ (- t start) out) 0 (- end t))))

(define (sp-path-free a) (void sp-path-t) (free a.segments))

#;(define (sp-path-new-get segments-len segments start end out)
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
    (set s-end (struct-get (array-get s.points (- s._points-len 1)) x))
    (s.interpolator start (min s-end end) s._start s.points s.options (+ start out))
    (set p-start (array-get s.points (- s._points-len 1))))
  (if (> end s-start) (memset (+ s-start out) 0 (- end s-start))))