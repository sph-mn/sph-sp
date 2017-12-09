(library (sph sp generate)
  (export
    sp-clip
    sp-fold-integers
    sp-generate
    sp-noise
    sp-path
    sp-path-new
    sp-path-new-p
    sp-segment
    sp-sine)
  (import
    (guile)
    (sph)
    (sph list)
    (sph math)
    (sph number)
    (sph sp)
    (sph uniform-vector)
    (sph vector)
    (srfi srfi-4))

  (define default-random-state (random-state-from-platform))

  (define (sp-sine time freq)
    "returns a sample value for a sine value after \"time\" seconds.
     freq: radians per second
     time: seconds since start with phase offset 0
     phase: repeats each 2pi"
    ; wavelength = pi radians
    (sin (* time freq)))

  (define*
    (sp-noise sample-count #:optional (random random:uniform) (random-state default-random-state))
    "integer [{random-state -> real} random-state] -> f64vector
     creates a vector of random sample values, which corresponds to random frequencies.
     default is a uniform distribution that does not repeat at every run of the program.
     guile includes several random number generators, for example: random:normal, random:uniform (the default), random:exp.
     if the state is the same, the number series will be the same"
    (f64vector-create sample-count (l (index) (random random-state))))

  (define (sp-fold-integers start end f . states)
    (let loop ((index start) (states states))
      (if (< index end) (loop (+ 1 index) (apply f index states)) states)))

  (define (sp-segment size f . states)
    "integer procedure -> (vector . states)
     create a new segment with values set by calling f for each sample"
    (let (result (make-f64vector size))
      (let loop ((index 0) (states states))
        (if (< index size)
          (let (states (apply f index states)) (f64vector-set! result index (first states))
            (loop (+ 1 index) (tail states)))
          (pair result states)))))

  (define (sp-clip a) "eventually adjust value to not exceed -1 or 1"
    (if (< 0 a) (min 1.0 a) (max -1.0 a)))

  (define (sp-generate sample-rate start duration segment-f sample-f . states)
    "integer number number procedure procedure any ... -> (any ...):states
     segment-f :: env time segment custom ... -> states
     sample-f :: env time custom ... -> sample-value"
    (let* ((sample-duration (/ 1 sample-rate)) (env (vector sample-rate sample-duration)))
      (apply sp-fold-integers start
        (+ start duration)
        (l (time . states)
          ; generate one segment per second
          (apply segment-f env
            time
            (apply sp-segment sample-rate
              (l (index . states) (apply sample-f env (+ time (* index sample-duration)) states))
              states)))
        states)))

  (define sp-path-new-p
    (let
      ( (line-new
          (l (result start sample-duration a)
            "add line interpolating procedures to result with params \"a\""
            (let*
              ( (points (pair start (map-slice 2 vector a)))
                (segments
                  (map-segments 2
                    (l (p1 p2)
                      (let*
                        ((start (vector-first p1)) (end (vector-first p2)) (duration (- end start)))
                        (vector start end
                          (l (time)
                            (vector-second (linear-interpolation (/ (- time start) duration) p1 p2))))))
                    points)))
              (list (append result segments) (last points)))))
        (bezier-new
          (l (result start sample-duration a)
            (let*
              ( (points (pair start (map-slice 2 vector a)))
                (segment
                  (let*
                    ( (p1 (first points)) (p2 (last points)) (start (vector-first p1))
                      (end (vector-first p2)) (duration (- end start)))
                    (vector start end
                      (l (time)
                        (vector-second (apply bezier-curve (/ (- time start) duration) points)))))))
              (list (append result (list segment)) (last points)))))
        (arc-new
          (l (result start sample-duration a)
            "the arc ends at point (x, y)
            the ellipse has the two radii (rx, ry)
            the x-axis of the ellipse is rotated by x-axis-rotation"
            (apply
              (l* (x y radius-x #:optional (radius-y radius-x) (rotation 0) large-arc sweep)
                (let*
                  ( (p1 start) (p2 (vector x y)) (start (vector-first start))
                    (end x) (duration (- end start)))
                  (list
                    (append result
                      (list
                        (vector start end
                          (l (time)
                            (vector-second
                              (first
                                (elliptical-arc (/ (- time start) duration) p1
                                  p2 radius-x radius-y rotation large-arc sweep)))))))
                    (vector x y))))
              a))))
      (l (sample-rate segments)
        "number ((symbol:type any:parameter ...) ...) -> path-state
        draw a path of multiple segments between points interpolated by functions selectable for each segment.
        returns a state object that is to be passed to sp-path to get a point on the path.
        there are no required segment types but at least one must be given.
        if no start is given then the path starts at 0. start can also be used as a path segment to create gaps.
        similar to the path element of svg vector graphics.
        for \"arc\" see how arcs are created with a path with svg graphics
        segment types:
          (start time value)
          (bezier time value time/value ...)
          (line time value time/value ...)
          (arc x y radius-x [radius-y rotation large-arc sweep])
        usage
          (sp-path-new 16000 (start 0.2 0) (line 0.5 0.25) (line 0.8 0.4) (line 1.2 0.01))"
        (let*
          ( (sample-duration (/ 1 sample-rate))
            (segments
              (first
                (fold-multiple
                  (l (a result start)
                    (case (first a) ((line) (line-new result start sample-duration (tail a)))
                      ((bezier) (bezier-new result start sample-duration (tail a)))
                      ((arc) (arc-new result start sample-duration (tail a)))
                      ((start) (list result (apply vector (tail a))))))
                  segments null #(0 0)))))
          (let ((next (first segments)) (index-i 0) (index (apply vector segments)))
            (list next index-i index))))))

  (define-syntax-rule (sp-path-new sample-rate (symbol param ...) ...)
    (sp-path-new-p sample-rate (list (list (quote symbol) param ...) ...)))

  (define (sp-path-advance path-state)
    "path-state -> path-state
     stop interpolating the current segment"
    (apply
      (l (next index-i index)
        (let (index-i (+ 1 index-i))
          (if (< index-i (vector-length index)) (list (vector-ref index index-i) index-i index)
            (list #f #f index))))
      path-state))

  (define sp-path-segment-start (vector-accessor 0))
  (define sp-path-segment-end (vector-accessor 1))
  (define sp-path-segment-f (vector-accessor 2))

  (define* (sp-path time path #:optional (c pair))
    "number path-state [procedure -> result] -> (data . path-state)/any
     get value at time for a path created by sp-path-new.
     returns zero for gaps or after the end of the path"
    (let (a (first path))
      (if (and a (>= time (sp-path-segment-start a)))
        (if (< time (sp-path-segment-end a)) (pair ((sp-path-segment-f a) time) path)
          (sp-path time (sp-path-advance path) c))
        (pair 0 path)))))
