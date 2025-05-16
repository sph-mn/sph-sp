(define (sp-path-samples out length point-count x y c)
  (status-t sp-sample-t** sp-time-t sp-path-point-count-t sp-sample-t* sp-sample-t* sp-sample-t*)
  "x length is point_count minus two.
   x contains only intermediate point x values x(0 + 1) to x(n - 1)
   the path will start at x 0 and end at x length.
   point_count must be greater one.
   y length is point_count.
   c length is point_count minus one.
   c is for curvature and values are between -1.0 and 1.0.
   c can be 0 in which case it is ignored"
  status-declare
  (declare path spline-path-t segments (array spline-path-segment-t sp-path-point-count-limit))
  (define
    last-point-index sp-path-point-count-t (- point-count 1)
    xn sp-sample-t length
    yn sp-sample-t (array-get y last-point-index)
    cn sp-sample-t (if* c (array-get c (- last-point-index 1)) 0))
  (set
    (array-get segments 0) (spline-path-move 0 (array-get y 0))
    (array-get segments last-point-index)
    (if* (< cn 1.0e-5) (spline-path-line xn yn) (spline-path-bezier-arc xn yn cn)))
  (for ((define i sp-path-point-count-t 1) (< i last-point-index) (set+ i 1))
    (set
      xn (array-get x (- i 1))
      yn (array-get y i)
      cn (if* c (array-get c (- i 1)) 0)
      (array-get segments i)
      (if* (< cn 1.0e-5) (spline-path-line xn yn) (spline-path-bezier-arc xn yn cn))))
  (spline-path-set &path segments point-count)
  (status-require (sp-samples-new length out))
  (spline-path-get &path 0 length *out)
  (label exit status-return))

(define (sp-path-times out length point-count x y c)
  (status-t sp-time-t** sp-time-t sp-path-point-count-t sp-sample-t* sp-sample-t* sp-sample-t*)
  "c is for curvature and values are between -1.0 and 1.0"
  status-declare
  (declare temp sp-sample-t*)
  (status-require (sp-path-samples &temp length point-count x y c))
  (status-require (sp-samples->times-replace temp length out))
  (label exit status-return))

(pre-define (sp-define-path-n type-name type)
  (begin
    (define ((pre-concat sp-path- type-name 2) out length y1 y2)
      (status-t type** sp-time-t sp-sample-t sp-sample-t)
      (declare y (array sp-sample-t 2))
      (array-set* y y1 y2)
      ((pre-concat sp-path- type-name) out length 2 0 y 0))
    (define ((pre-concat sp-path- type-name 3) out length x1 y1 y2 y3)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (declare y (array sp-sample-t 3))
      (array-set* y y1 y2 y3)
      ((pre-concat sp-path- type-name) out length 3 &x1 y 0))
    (define ((pre-concat sp-path- type-name 4) out length x1 x2 y1 y2 y3 y4)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (declare x (array sp-sample-t 2) y (array sp-sample-t 4))
      (array-set* x x1 x2)
      (array-set* y y1 y2 y3 y4)
      ((pre-concat sp-path- type-name) out length 4 x y 0))
    (define ((pre-concat sp-path- type-name 5) out length x1 x2 x3 y1 y2 y3 y4 y5)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (declare x (array sp-sample-t 3) y (array sp-sample-t 5))
      (array-set* x x1 x2 x3)
      (array-set* y y1 y2 y3 y4 y5)
      ((pre-concat sp-path- type-name) out length 5 x y 0))
    (define ((pre-concat sp-path- type-name _curve 2) out length y1 y2 c1)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t)
      (declare y (array sp-sample-t 2))
      (array-set* y y1 y2)
      ((pre-concat sp-path- type-name) out length 2 0 y &c1))
    (define ((pre-concat sp-path- type-name _curve 3) out length x1 y1 y2 y3 c1 c2)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (declare y (array sp-sample-t 3) c (array sp-sample-t 2))
      (array-set* y y1 y2 y3)
      (array-set* c c1 c2)
      ((pre-concat sp-path- type-name) out length 3 &x1 y c))
    (define ((pre-concat sp-path- type-name _curve 4) out length x1 x2 y1 y2 y3 y4 c1 c2 c3)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (declare x (array sp-sample-t 2) y (array sp-sample-t 4) c (array sp-sample-t 3))
      (array-set* x x1 x2)
      (array-set* y y1 y2 y3 y4)
      (array-set* c c1 c2 c3)
      ((pre-concat sp-path- type-name) out length 4 x y c))
    (define ((pre-concat sp-path- type-name _curve 5) out length x1 x2 x3 y1 y2 y3 y4 y5 c1 c2 c3 c4)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (declare x (array sp-sample-t 3) y (array sp-sample-t 5) c (array sp-sample-t 4))
      (array-set* x x1 x2 x3)
      (array-set* y y1 y2 y3 y4 y5)
      (array-set* c c1 c2 c3 c4)
      ((pre-concat sp-path- type-name) out length 5 x y c))))

(sp-define-path-n samples sp-sample-t)
(sp-define-path-n times sp-time-t)

(define (sp-envelope-zero out length point-count x y c)
  (status-t sp-sample-t** sp-time-t sp-path-point-count-t sp-sample-t* sp-sample-t* sp-sample-t*)
  "from a first implicit y value 0 to a final implicit y value 0.
   only the intermediate points are provided.
   x, y and c length is point_count minus two.
   x values will be cumulative (0.1, 0.2) -> 0.3.
   x values will be multiplied by length."
  (declare path-y (array sp-sample-t sp-path-point-count-limit))
  (set* (array-get x 1) length)
  (set (array-get path-y 0) 0 (array-get path-y (- point-count 1)) 0)
  (for ((define i sp-path-point-count-t 2) (< i (- point-count 1)) (set+ i 1))
    (set
      (array-get x i) (+ (array-get x (- i 1)) (* (array-get x i) length))
      (array-get path-y i) (array-get y i)))
  (return (sp-path-samples out length point-count x path-y c)))

(pre-define (sp-define-envelope-zero-n prefix type-name type)
  (begin
    (define ((pre-concat prefix 3) out length x1 y1)
      (status-t type** sp-time-t sp-sample-t sp-sample-t)
      (return ((pre-concat sp-path- type-name 3) out length (* x1 length) 0 y1 0)))
    (define ((pre-concat prefix 4) out length x1 x2 y1 y2)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (return
        ( (pre-concat sp-path- type-name 4) out length
          (* x1 length) (+ (* x1 length) (* x2 length)) 0 y1 y2 0)))
    (define ((pre-concat prefix 5) out length x1 x2 x3 y1 y2 y3)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (set* x1 length)
      (set x2 (+ x1 (* x2 length)) x3 (+ x2 (* x3 length)))
      (return ((pre-concat sp-path- type-name 5) out length x1 x2 x3 0 y1 y2 y3 0)))
    (define ((pre-concat prefix _curve 3) out length x1 y1 c1 c2)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (return ((pre-concat sp-path- type-name _curve 3) out length (* x1 length) 0 y1 0 c1 c2)))
    (define ((pre-concat prefix _curve 4) out length x1 x2 y1 y2 c1 c2 c3)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (return
        ( (pre-concat sp-path- type-name _curve 4) out length
          (* x1 length) (+ (* x1 length) (* x2 length)) 0 y1 y2 0 c1 c2 c3)))
    (define ((pre-concat prefix _curve 5) out length x1 x2 x3 y1 y2 y3 c1 c2 c3 c4)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (set* x1 length)
      (set x2 (+ x1 (* x2 length)) x3 (+ x2 (* x3 length)))
      (return ((pre-concat sp-path- type-name _curve 5) out length x1 x2 x3 0 y1 y2 y3 0 c1 c2 c3 c4)))))

(sp-define-envelope-zero-n sp-envelope-zero samples sp-sample-t)

(define (sp-envelope-scale out length point-count y-scalar x y c)
  (status-t sp-time-t** sp-time-t sp-path-point-count-t sp-sample-t sp-sample-t* sp-sample-t* sp-sample-t*)
  "y and c length is point_count.
   x length is point_count minus two.
   x values will be cumulative (0.1, 0.2) -> 0.3.
   x values will be multiplied by length."
  (declare path-y (array sp-sample-t sp-path-point-count-limit))
  (set* (array-get x 1) length)
  (set
    (array-get path-y 0) (* y-scalar (array-get y 0))
    (array-get path-y (- point-count 1)) (* y-scalar (array-get y (- point-count 1))))
  (for ((define i sp-path-point-count-t 2) (< i (- point-count 1)) (set+ i 1))
    (set
      (array-get x i) (+ (array-get x (- i 1)) (* (array-get x i) length))
      (array-get path-y i) (* y-scalar (array-get y i))))
  (return (sp-path-times out length point-count x path-y c)))

(pre-define (sp-define-envelope-scale-n prefix type-name type)
  (begin
    (define ((pre-concat prefix 3) out length y-scalar x1 y1 y2 y3)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (return
        ( (pre-concat sp-path- type-name 3) out length
          (* x1 length) (* y1 y-scalar) (* y2 y-scalar) (* y3 y-scalar))))
    (define ((pre-concat prefix 4) out length y-scalar x1 x2 y1 y2 y3 y4)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (return
        ( (pre-concat sp-path- type-name 4) out length
          (* x1 length) (+ (* x1 length) (* x2 length)) (* y-scalar y1)
          (* y-scalar y2) (* y-scalar y3) (* y-scalar y4))))
    (define ((pre-concat prefix 5) out length y-scalar x1 x2 x3 y1 y2 y3 y4 y5)
      (status-t type** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
      (set* x1 length)
      (set x2 (+ x1 (* x2 length)) x3 (+ x2 (* x3 length)))
      (return
        ( (pre-concat sp-path- type-name 5) out length
          x1 x2 x3 (* y-scalar y1) (* y-scalar y2) (* y-scalar y3) (* y-scalar y4) (* y-scalar y5))))))

(sp-define-envelope-scale-n sp-envelope-scale times sp-time-t)