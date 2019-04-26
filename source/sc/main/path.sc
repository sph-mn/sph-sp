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
  * all segment types have a fixed number of points. line: 1, bezier: 3, move: 1, constant: 0, path: 0
  * negative x values not supported
  * internally all segments start at (0 0) and no gaps are between segments
  * assumes that bit 0 is sp-path-value-t zero
  * segments draw to the endpoint inclusive, start point exclusive")

(define (sp-path-i-move start size p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 1"
  (memset out 0 (* (sizeof sp-path-value-t) size)))

(define (sp-path-i-constant start size p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 0"
  (declare i sp-sample-count-t)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set (array-get out i) p-start.y)))

(define (sp-path-i-line start size p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 1"
  (declare
    i sp-sample-count-t
    p-end sp-path-point-t
    s-size sp-path-value-t
    s-relative-start sp-sample-count-t
    t sp-path-value-t)
  (set
    p-end (array-get p-rest 0)
    s-size (- p-end.x p-start.x)
    s-relative-start (- start p-start.x))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set
      t (/ (+ s-relative-start i) s-size)
      (array-get out i) (+ (* p-end.y t) (* p-start.y (- 1 t))))))

(define (sp-path-i-bezier start size p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 3"
  (declare
    i sp-sample-count-t
    mt sp-path-value-t
    p-end sp-path-point-t
    s-size sp-path-value-t
    s-relative-start sp-sample-count-t
    t sp-path-value-t)
  (set
    p-end (array-get p-rest 2)
    s-size (- p-end.x p-start.x)
    s-relative-start (- start p-start.x))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set
      t (/ (+ s-relative-start i) s-size)
      mt (- 1 t)
      (array-get out i)
      (+
        (* p-start.y mt mt mt)
        (* (struct-get (array-get p-rest 0) y) 3 mt mt t)
        (* (struct-get (array-get p-rest 1) y) 3 mt t t) (* p-end.y t t t)))))

(define (sp-path-i-path start size p-start p-rest options out)
  (void sp-sample-count-t sp-sample-count-t sp-path-point-t sp-path-point-t* void* sp-path-value-t*)
  "p-rest length 0. options is one sp-path-t"
  (sp-path-get
    (pointer-get (convert-type options sp-path-t*))
    (- start p-start.x) (+ (- start p-start.x) size) out))

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
  (set
    start.x 0
    start.y 0)
  (status-require (sph-helper-malloc (* segments-len (sizeof sp-path-segment-t)) &path.segments))
  (memcpy path.segments segments (* segments-len (sizeof sp-path-segment-t)))
  (for ((set i 0) (< i segments-len) (set i (+ 1 i)))
    (set
      (struct-get (array-get path.segments i) _start) start
      s (array-get path.segments i)
      s._points-len (sp-path-interpolator-points-len s.interpolator))
    (case = s.interpolator
      ( (sp-path-i-path)
        (set
          start (sp-path-end (pointer-get (convert-type s.options sp-path-t*)))
          *s.points start))
      ( (sp-path-i-constant)
        (set
          *s.points start
          s.points:x sp-sample-count-max))
      (else (set start (array-get s.points (- s._points-len 1)))))
    (set (array-get path.segments i) s))
  (set
    path.segments-len segments-len
    *out-path path)
  (label exit
    (return status)))

(define (sp-path-get path start end out)
  (void sp-path-t sp-sample-count-t sp-sample-count-t sp-path-value-t*)
  "get values on path between start (inclusive) and end (exclusive)"
  (sc-comment "write segment start first, then the segment such that it ends before the next start")
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
    (if (= t s-start)
      (set
        (array-get out (- t start)) s._start.y
        t (+ 1 t)))
    (s.interpolator t (- (min s-end end) t) s._start s.points s.options (+ (- t start) out))
    (set t s-end))
  (if (<= t end)
    (begin
      (s.interpolator t 1 s._start s.points s.options (+ (- t start) out))
      (set t (+ 1 t))
      (if (<= t end) (memset (+ (- t start) out) 0 (* (sizeof sp-path-value-t) (- end t)))))))

(define (sp-path-free a) (void sp-path-t) (free a.segments))

(define (sp-path-new-get segments-len segments start end out)
  (status-t
    sp-path-segment-count-t sp-path-segment-t* sp-sample-count-t sp-sample-count-t sp-path-value-t*)
  "create a path array immediately from segments without creating a path object"
  status-declare
  (declare path sp-path-t)
  (status-require (sp-path-new segments-len segments &path))
  (sp-path-get path start end out)
  (sp-path-free path)
  (label exit
    (return status)))