(sc-comment "depends on spline-path-h.c")

(define (spline-path-i-move start end p-start p-rest options out)
  (void spline-path-time-t spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 1"
  (memset out 0 (* (sizeof spline-path-value-t) (- end start))))

(define (spline-path-i-constant start end p-start p-rest options out)
  (void spline-path-time-t spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 0"
  (declare i spline-path-time-t)
  (for ((set i start) (< i end) (set i (+ 1 i))) (set (array-get out (- i start)) p-start.y)))

(define (spline-path-i-line start end p-start p-rest options out)
  (void spline-path-time-t spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 1"
  (declare
    i spline-path-time-t
    p-end spline-path-point-t
    s-size spline-path-value-t
    t spline-path-value-t)
  (set p-end (array-get p-rest 0) s-size (- p-end.x p-start.x))
  (for ((set i start) (< i end) (set i (+ 1 i)))
    (set
      t (/ (- i p-start.x) s-size)
      (array-get out (- i start)) (+ (* p-end.y t) (* p-start.y (- 1 t))))))

(define (spline-path-i-bezier start end p-start p-rest options out)
  (void spline-path-time-t spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 3"
  (declare
    i spline-path-time-t
    mt spline-path-value-t
    p-end spline-path-point-t
    s-size spline-path-value-t
    t spline-path-value-t)
  (set p-end (array-get p-rest 2) s-size (- p-end.x p-start.x))
  (for ((set i start) (< i end) (set i (+ 1 i)))
    (set
      t (/ (- i p-start.x) s-size)
      mt (- 1 t)
      (array-get out (- i start))
      (+ (* p-start.y mt mt mt) (* (struct-get (array-get p-rest 0) y) 3 mt mt t)
        (* (struct-get (array-get p-rest 1) y) 3 mt t t) (* p-end.y t t t)))))

(define (spline-path-get path start end out)
  (void spline-path-t spline-path-time-t spline-path-time-t spline-path-value-t*)
  "get values on path between start (inclusive) and end (exclusive).
   since x values are integers, a path from (0 0) to (10 20) for example will have reached 20 only at the 11th point.
   out memory is managed by the caller. the size required for out is end minus start"
  (sc-comment "find all segments that overlap with requested range")
  (declare
    i spline-path-segment-count-t
    s spline-path-segment-t
    s-start spline-path-time-t
    s-end spline-path-time-t
    out-start spline-path-time-t)
  (for ((set i 0) (< i path.segments-count) (set i (+ 1 i)))
    (set
      s (array-get path.segments i)
      s-start s._start.x
      s-end (struct-get (array-get s.points (- s._points-count 1)) x))
    (if (> s-start end) break)
    (if (< s-end start) continue)
    (set
      out-start (if* (> s-start start) (- s-start start) 0)
      s-start (if* (> s-start start) s-start start)
      s-end (if* (< s-end end) s-end end))
    (s.interpolator s-start s-end s._start s.points s.options (+ out-start out)))
  (sc-comment
    "outside points zero. set the last segment point which would be set by a following segment."
    "can only be true for the last segment")
  (if (> end s-end)
    (begin
      (set
        (array-get out s-end) (struct-get (array-get s.points (- s._points-count 1)) y)
        s-end (+ 1 s-end))
      (if (> end s-end) (memset (+ s-end out) 0 (* (- end s-end) (sizeof spline-path-value-t)))))))

(define (spline-path-i-path start end p-start p-rest options out)
  (void spline-path-time-t spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 0. options is one spline-path-t"
  (spline-path-get (pointer-get (convert-type options spline-path-t*)) (- start p-start.x)
    (- end p-start.x) out))

(define (spline-path-start path) (spline-path-point-t spline-path-t)
  (declare p spline-path-point-t s spline-path-segment-t)
  (set s (array-get path.segments 0))
  (if (= spline-path-i-move s.interpolator) (set p (array-get s.points 0)) (set p.x 0 p.y 0))
  (return p))

(define (spline-path-end path) (spline-path-point-t spline-path-t)
  (declare s spline-path-segment-t)
  (set s (array-get path.segments (- path.segments-count 1)))
  (if (= spline-path-i-constant s.interpolator)
    (set s (array-get path.segments (- path.segments-count 2))))
  (return (array-get s.points (- s._points-count 1))))

(define (spline-path-new segments-count segments out-path)
  (uint8-t spline-path-segment-count-t spline-path-segment-t* spline-path-t*)
  "segments are copied into out-path"
  (declare
    i spline-path-segment-count-t
    path spline-path-t
    s spline-path-segment-t
    start spline-path-point-t)
  (set start.x 0 start.y 0)
  (set path.segments (malloc (* segments-count (sizeof spline-path-segment-t))))
  (if (not path.segments) (return 1))
  (memcpy path.segments segments (* segments-count (sizeof spline-path-segment-t)))
  (for ((set i 0) (< i segments-count) (set i (+ 1 i)))
    (set
      (struct-get (array-get path.segments i) _start) start
      s (array-get path.segments i)
      s._points-count (if* (= spline-path-i-bezier s.interpolator) 3 1))
    (case = s.interpolator
      ( (spline-path-i-path)
        (set
          start (spline-path-end (pointer-get (convert-type s.options spline-path-t*)))
          *s.points start))
      ((spline-path-i-constant) (set *s.points start s.points:x spline-path-time-max))
      (else (set start (array-get s.points (- s._points-count 1)))))
    (set (array-get path.segments i) s))
  (set path.segments-count segments-count *out-path path)
  (return 0))

(define (spline-path-free a) (void spline-path-t) (free a.segments))

(define (spline-path-new-get segments-count segments start end out)
  (uint8-t spline-path-segment-count-t spline-path-segment-t* spline-path-time-t spline-path-time-t spline-path-value-t*)
  "create a path array immediately from segments without creating a path object"
  (declare path spline-path-t)
  (if (spline-path-new segments-count segments &path) (return 1))
  (spline-path-get path start end out)
  (spline-path-free path)
  (return 0))

(define (spline-path-move x y) (spline-path-segment-t spline-path-time-t spline-path-value-t)
  "returns a move segment for the specified point"
  (declare s spline-path-segment-t)
  (set s.interpolator spline-path-i-move s.points:x x s.points:y y)
  (return s))

(define (spline-path-line x y) (spline-path-segment-t spline-path-time-t spline-path-value-t)
  (declare s spline-path-segment-t)
  (set s.interpolator spline-path-i-line s.points:x x s.points:y y)
  (return s))

(define (spline-path-bezier x1 y1 x2 y2 x3 y3)
  (spline-path-segment-t spline-path-time-t spline-path-value-t spline-path-time-t spline-path-value-t spline-path-time-t spline-path-value-t)
  (declare s spline-path-segment-t)
  (set
    s.interpolator spline-path-i-bezier
    s.points:x x1
    s.points:y y1
    (: (+ 1 s.points) x) x2
    (: (+ 1 s.points) y) y2
    (: (+ 2 s.points) x) x3
    (: (+ 2 s.points) y) y3)
  (return s))

(define (spline-path-constant) (spline-path-segment-t)
  (declare s spline-path-segment-t)
  (set s.interpolator spline-path-i-constant)
  (return s))

(define (spline-path-path path) (spline-path-segment-t spline-path-t*)
  "return a segment that is another spline-path. length is the full length of the path.
   the path does not necessarily connect and is drawn as it would be on its own starting from the preceding segment"
  (declare s spline-path-segment-t)
  (set s.interpolator spline-path-i-path s.options path)
  (return s))