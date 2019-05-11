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

; todo: cheap filter only as bandpass, passes support

(define*
  (sp-cheap-filter
    type
    out
    in
    cutoff
    passes
    state
    #:key
    (q-factor 0)
    (in-start 0) (in-count (- (sp-samples-length in) in-start)) (out-start 0) (unity-gain #t))
  "symbol samples samples real:0..0.5 integer samples [keys ...] -> unspecified
     a less processing intensive filter based on sp-state-variable-filter.
     type can be low, high, band, notch, peak or all.
     the basic filter algorithm is cheaper than the sp-filter windowed sinc, especially with frequently changing parameter values,
     but multiple passes and the processing for unity-gain relativise that quite a bit"
  (let*
    ( (f
        (case type ((low) sp-state-variable-filter-lp)
          ((high) sp-state-variable-filter-hp)
          ((band) sp-state-variable-filter-bp)
          ((reject) sp-state-variable-filter-br)
          ((peak) sp-state-variable-filter-peak)
          ((all) sp-state-variable-filter-all)))
      (state
        (sp-multipass
          (l
            (out in in-start in-count out-start state)
            (f out out-start in in-start in-count cutoff q-factor (or state (sp-samples-new 2))))
          out in passes state in-start in-count out-start)))
    (if unity-gain (sp-set-unity-gain out in in-start in-count out-start)) state))