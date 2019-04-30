(sc-comment
  "* spline-path creates discrete 2d paths interpolated between some given points
  * maps from one independent discrete value to one dependent continuous value
  * only the dependent value is returned
  * kept minimal (only 2d, only selected interpolators, limited segment count) to be extremely fast
  * multidimensional interpolation can be archieved with multiple configs and calls
  * a copy of segments is made internally and only the copy is used
  * uses points as structs because pre-defined size arrays can not be used in structs
  * segments-len must be greater than zero
  * segments must be a valid spline-path segment configuration
  * interpolators are called with path-relative start/end inside segment and with out positioned at the segment output
  * all segment types have a fixed number of points. line: 1, bezier: 3, move: 1, constant: 0, path: 0
  * negative x values not supported
  * internally all segments start at (0 0) and no gaps are between segments
  * assumes that bit 0 is spline-path-value-t zero
  * segments draw to the endpoint inclusive, start point exclusive")

(pre-include "inttypes.h" "strings.h" "stdlib.h")

(pre-define-if-not-defined
  spline-path-time-t uint32-t
  spline-path-value-t double
  spline-path-point-count-t uint8-t
  spline-path-segment-count-t uint16-t
  spline-path-time-max UINT32_MAX)

(pre-define
  spline-path-point-limit 3
  (spline-path-interpolator-points-len a)
  (if* (= spline-path-i-bezier a) 3
    1))

(declare
  spline-path-point-t
  (type
    (struct
      (x spline-path-value-t)
      (y spline-path-value-t)))
  spline-path-interpolator-t
  (type
    (function-pointer
      void
      spline-path-time-t
      spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*))
  spline-path-segment-t
  (type
    (struct
      (_start spline-path-point-t)
      (_points-len spline-path-point-count-t)
      (points (array spline-path-point-t spline-path-point-limit))
      (interpolator spline-path-interpolator-t)
      (options void*)))
  spline-path-t
  (type
    (struct
      (segments-len spline-path-segment-count-t)
      (segments spline-path-segment-t*)))
  (spline-path-i-line start end p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  (spline-path-free a) (void spline-path-t)
  (spline-path-new segments-len segments out-path)
  (uint8-t spline-path-segment-count-t spline-path-segment-t* spline-path-t*)
  (spline-path-get path start end out)
  (void spline-path-t spline-path-time-t spline-path-time-t spline-path-value-t*)
  (spline-path-new-get segments-len segments start end out)
  (uint8-t
    spline-path-segment-count-t
    spline-path-segment-t* spline-path-time-t spline-path-time-t spline-path-value-t*)
  (spline-path-i-move start end p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  (spline-path-i-constant start end p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  (spline-path-i-bezier start end p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  (spline-path-i-path start end p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  (spline-path-start path) (spline-path-point-t spline-path-t)
  (spline-path-end path) (spline-path-point-t spline-path-t))

(define (spline-path-i-move start size p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 1"
  (memset out 0 (* (sizeof spline-path-value-t) size)))

(define (spline-path-i-constant start size p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 0"
  (declare i spline-path-time-t)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set (array-get out i) p-start.y)))

(define (spline-path-i-line start size p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 1"
  (declare
    i spline-path-time-t
    p-end spline-path-point-t
    s-size spline-path-value-t
    s-relative-start spline-path-time-t
    t spline-path-value-t)
  (set
    p-end (array-get p-rest 0)
    s-size (- p-end.x p-start.x)
    s-relative-start (- start p-start.x))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set
      t (/ (+ s-relative-start i) s-size)
      (array-get out i) (+ (* p-end.y t) (* p-start.y (- 1 t))))))

(define (spline-path-i-bezier start size p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 3"
  (declare
    i spline-path-time-t
    mt spline-path-value-t
    p-end spline-path-point-t
    s-size spline-path-value-t
    s-relative-start spline-path-time-t
    t spline-path-value-t)
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

(define (spline-path-i-path start size p-start p-rest options out)
  (void
    spline-path-time-t
    spline-path-time-t spline-path-point-t spline-path-point-t* void* spline-path-value-t*)
  "p-rest length 0. options is one spline-path-t"
  (spline-path-get
    (pointer-get (convert-type options spline-path-t*))
    (- start p-start.x) (+ (- start p-start.x) size) out))

(define (spline-path-start path) (spline-path-point-t spline-path-t)
  (declare
    p spline-path-point-t
    s spline-path-segment-t)
  (set s (array-get path.segments 0))
  (if (= spline-path-i-move s.interpolator) (set p (array-get s.points 0))
    (set
      p.x 0
      p.y 0))
  (return p))

(define (spline-path-end path) (spline-path-point-t spline-path-t)
  (declare s spline-path-segment-t)
  (set s (array-get path.segments (- path.segments-len 1)))
  (if (= spline-path-i-constant s.interpolator)
    (set s (array-get path.segments (- path.segments-len 2))))
  (return (array-get s.points (- s._points-len 1))))

(define (spline-path-new segments-len segments out-path)
  (uint8-t spline-path-segment-count-t spline-path-segment-t* spline-path-t*)
  (declare
    i spline-path-segment-count-t
    path spline-path-t
    s spline-path-segment-t
    start spline-path-point-t)
  (set
    start.x 0
    start.y 0)
  (set path.segments (malloc (* segments-len (sizeof spline-path-segment-t))))
  (if (not path.segments) (return 1))
  (memcpy path.segments segments (* segments-len (sizeof spline-path-segment-t)))
  (for ((set i 0) (< i segments-len) (set i (+ 1 i)))
    (set
      (struct-get (array-get path.segments i) _start) start
      s (array-get path.segments i)
      s._points-len (spline-path-interpolator-points-len s.interpolator))
    (case = s.interpolator
      ( (spline-path-i-path)
        (set
          start (spline-path-end (pointer-get (convert-type s.options spline-path-t*)))
          *s.points start))
      ( (spline-path-i-constant)
        (set
          *s.points start
          s.points:x spline-path-time-max))
      (else (set start (array-get s.points (- s._points-len 1)))))
    (set (array-get path.segments i) s))
  (set
    path.segments-len segments-len
    *out-path path)
  (return 0))

(define (spline-path-get path start end out)
  (void spline-path-t spline-path-time-t spline-path-time-t spline-path-value-t*)
  "get values on path between start (inclusive) and end (exclusive)"
  (sc-comment "write segment start first, then the segment such that it ends before the next start")
  (declare
    i spline-path-segment-count-t
    s spline-path-segment-t
    s-start spline-path-time-t
    s-end spline-path-time-t
    t spline-path-segment-count-t)
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
    (s.interpolator
      t
      (-
        (if* (> end s-end) s-end
          end)
        t)
      s._start s.points s.options (+ (- t start) out))
    (set t s-end))
  (if (<= t end)
    (begin
      (s.interpolator t 1 s._start s.points s.options (+ (- t start) out))
      (set t (+ 1 t))
      (if (<= t end) (memset (+ (- t start) out) 0 (* (sizeof spline-path-value-t) (- end t)))))))

(define (spline-path-free a) (void spline-path-t) (free a.segments))

(define (spline-path-new-get segments-len segments start end out)
  (uint8-t
    spline-path-segment-count-t
    spline-path-segment-t* spline-path-time-t spline-path-time-t spline-path-value-t*)
  "create a path array immediately from segments without creating a path object"
  (declare path spline-path-t)
  (if (spline-path-new segments-len segments &path) (return 1))
  (spline-path-get path start end out)
  (spline-path-free path)
  (return 0))