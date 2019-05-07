(define (sp-multipass f out in passes state in-start in-count out-start)
  (define (first-or-false a)
    (if (null? a) #f
      (first a)))
  (define (tail-or-null a)
    (if (null? a) a
      (tail a)))
  (if (= 1 passes) (list (f out in in-start in-count out-start (first-or-false state)))
    (let*
      ( (out-temp (sp-samples-new in-count))
        (result-state (list (f out-temp in in-start in-count 0 (first-or-false state))))
        (state (tail-or-null state)))
      (let
        loop
        ( (passes (- passes 1))
          (in-temp out-temp)
          (out-temp (sp-samples-new in-count)) (state state) (result-state result-state))
        (if (< 1 passes)
          (let
            (result-state
              (pair (f out-temp in-temp 0 in-count 0 (first-or-false state)) result-state))
            (loop
              (- passes 1) out-temp (sp-samples-zero! in-temp) (tail-or-null state) result-state))
          (reverse (pair (f out in-temp 0 in-count out-start (first-or-false state)) result-state)))))))