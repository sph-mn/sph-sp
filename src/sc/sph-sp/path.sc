(define (sp-path-get path start end out cursor)
  (void sp-path-t* sp-time-t sp-time-t sp-sample-t* sp-time-t*)
  "g(t,c) = (exp(k*c*t) - 1) / (exp(k*c) - 1)"
  (declare
    point-index sp-time-t
    out-index sp-time-t
    t sp-time-t*
    y sp-sample-t*
    c sp-sample-t*
    segment-start-time sp-time-t
    segment-end-time sp-time-t
    segment-length sp-time-t
    segment-to-time sp-time-t
    step-index sp-time-t
    start-value sp-sample-t
    end-value sp-sample-t
    step-value sp-sample-t
    curve-value sp-sample-t
    a-value sp-sample-t
    denom-value sp-sample-t
    ratio-value sp-sample-t
    exp-value sp-sample-t)
  (set
    t path:t
    y (array-get path:values 0)
    c (array-get path:values 1)
    point-index *cursor
    out-index 0)
  (if (= c 0)
    (begin
      (while (< start end)
        (set
          segment-start-time (array-get t point-index)
          segment-end-time (array-get t (+ point-index 1))
          segment-length (- segment-end-time segment-start-time)
          segment-to-time (if* (< end segment-end-time) end segment-end-time)
          start-value (array-get y point-index)
          end-value (array-get y (+ point-index 1))
          step-value (/ (- end-value start-value) segment-length)
          step-index (- start segment-start-time))
        (while (< start segment-to-time)
          (set (array-get out out-index) (+ start-value (* step-value step-index)))
          (set+ out-index 1 start 1 step-index 1))
        (if (= segment-to-time segment-end-time) (set+ point-index 1)))
      (set *cursor point-index)
      return))
  (while (< start end)
    (set
      segment-start-time (array-get t point-index)
      segment-end-time (array-get t (+ point-index 1))
      segment-length (- segment-end-time segment-start-time)
      segment-to-time (if* (< end segment-end-time) end segment-end-time)
      start-value (array-get y point-index)
      end-value (array-get y (+ point-index 1))
      curve-value (array-get c point-index))
    (if (= curve-value 0.0)
      (begin
        (set
          step-value (/ (- end-value start-value) segment-length)
          step-index (- start segment-start-time))
        (while (< start segment-to-time)
          (set (array-get out out-index) (+ start-value (* step-value step-index)))
          (set+ out-index 1 start 1 step-index 1)))
      (begin
        (set
          a-value (* sp-path-curve-strength curve-value)
          denom-value (expm1 a-value)
          ratio-value (exp (/ a-value segment-length))
          step-index (- start segment-start-time)
          exp-value (exp (* a-value (/ step-index segment-length))))
        (while (< start segment-to-time)
          (set (array-get out out-index)
            (+ start-value (* (- end-value start-value) (/ (- exp-value 1.0) denom-value))))
          (set+ out-index 1 start 1)
          (set* exp-value ratio-value))))
    (if (= segment-to-time segment-end-time) (set+ point-index 1)))
  (set *cursor point-index))
