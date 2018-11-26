(sc-comment
  "implementation of a hq windowed sinc filter with a blackman window (common truncated version) for continuous streams.
  variable sample-rate, cutoff radian frequency and transition band width per call.
  sp-windowed-sinc-state-t is used to store the impulse response, the parameters that where used to create it, and
  data that has to be carried over between calls")

(define (sp-window-blackman a width) (sp-float-t sp-float-t sp-sample-count-t)
  (return
    (+
      (- 0.42 (* 0.5 (cos (/ (* 2 M_PI a) (- width 1))))) (* 0.08 (cos (/ (* 4 M_PI a) (- width 1)))))))

(define (sp-windowed-sinc-ir-length transition) (sp-sample-count-t sp-float-t)
  "approximate impulse response length for a transition factor and
  ensure that the length is odd"
  (declare result sp-sample-count-t)
  (set result (ceil (/ 4 transition)))
  (if (not (modulo result 2)) (set result (+ 1 result)))
  (return result))

(define (sp-windowed-sinc-ir cutoff transition result-len result-ir)
  (status-t sp-float-t sp-float-t sp-sample-count-t* sp-sample-t**)
  "create an impulse response kernel for a windowed sinc filter. uses a blackman window (truncated version).
  allocates result-ir, sets result-len.
  cutoff and transition are as a fraction 0..1 of the sampling rate"
  status-declare
  (declare
    center-index sp-float-t
    i sp-sample-count-t
    ir sp-sample-t*
    len sp-sample-count-t
    sum sp-float-t)
  (set
    len (sp-windowed-sinc-ir-length transition)
    center-index (/ (- len 1.0) 2.0))
  (status-require (sph-helper-malloc (* len (sizeof sp-sample-t)) &ir))
  (sc-comment "nan can be set here if the freq and transition values are invalid")
  (for ((set i 0) (< i len) (set i (+ 1 i)))
    (set (array-get ir i) (* (sp-window-blackman i len) (sp-sinc (* 2 cutoff (- i center-index))))))
  (sc-comment "scale")
  (set sum (sp-sample-sum ir len))
  (for ((set i 0) (< i len) (set i (+ 1 i)))
    (set (array-get ir i) (/ (array-get ir i) sum)))
  (set
    *result-ir ir
    *result-len len)
  (label exit
    (return status)))

(define (sp-windowed-sinc-hp-ir cutoff transition result-len result-ir)
  (status-t sp-float-t sp-float-t sp-sample-count-t* sp-sample-t**)
  "a high-pass filter version of the windowed-sinc impulse response with spectral inversion"
  status-declare
  (status-require (sp-windowed-sinc-ir cutoff transition result-len result-ir))
  (sp-spectral-inversion-ir *result-ir *result-len)
  (label exit
    (return status)))

(define (sp-windowed-sinc-state-free state) (void sp-windowed-sinc-state-t*)
  (free state:ir)
  (free state:carryover)
  (free state))

(define (sp-windowed-sinc-state-set sample-rate cutoff transition ir-f result-state)
  (status-t
    sp-sample-count-t sp-float-t sp-float-t sp-windowed-sinc-ir-f-t sp-windowed-sinc-state-t**)
  "create or update a previously created state object. impulse response array properties are calculated
  with ir-f from cutoff and transition.
  eventually frees state.ir"
  status-declare
  (declare
    carryover sp-sample-t*
    ir sp-sample-t*
    ir-len sp-sample-count-t
    state sp-windowed-sinc-state-t*)
  (memreg-init 1)
  (sc-comment
    "create state if not exists. re-use if exists and return early if ir needs not be updated")
  (if *result-state
    (begin
      (set state *result-state)
      (if
        (and
          (= state:sample-rate sample-rate)
          (= state:cutoff cutoff) (= state:transition transition) (= state:ir-f ir-f))
        (return status)
        (if state:ir (free state:ir))))
    (begin
      (status-require (sph-helper-malloc (sizeof sp-windowed-sinc-state-t) &state))
      (memreg-add state)
      (set
        state:carryover-alloc-len 0
        state:carryover-len 0
        state:carryover 0
        state:ir 0)))
  (sc-comment "create new ir")
  (status-require
    (ir-f
      (sp-windowed-sinc-ir-cutoff cutoff sample-rate)
      (sp-windowed-sinc-ir-transition transition sample-rate) &ir-len &ir))
  (sc-comment
    "eventually extend carryover array. the array is never shrunk."
    "carryover-len is at least ir-len - 1." "carryover-alloc-len is the length of the whole array")
  (if state:carryover
    (if (> ir-len state:carryover-alloc-len)
      (begin
        (set carryover state:carryover)
        (status-require (sph-helper-realloc (* (- ir-len 1) (sizeof sp-sample-t)) &carryover))
        (set state:carryover-alloc-len (- ir-len 1)))
      (set carryover state:carryover))
    (begin
      (status-require (sph-helper-calloc (* (- ir-len 1) (sizeof sp-sample-t)) &carryover))
      (set state:carryover-alloc-len (- ir-len 1))))
  (set
    state:carryover carryover
    state:ir ir
    state:ir-len ir-len
    state:sample-rate sample-rate
    state:cutoff cutoff
    state:transition transition
    *result-state state)
  (label exit
    (if status-is-failure memreg-free)
    (return status)))

(define
  (sp-windowed-sinc
    source source-len sample-rate cutoff transition is-high-pass result-state result-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t
    sp-sample-rate-t sp-float-t sp-float-t boolean sp-windowed-sinc-state-t** sp-sample-t*)
  "a windowed sinc filter for segments of continuous streams with
  variable sample-rate, frequency, transition and impulse response type per call.
  * cutoff: radian frequency
  * transition: radian frequency, describes a frequency band.
  * is-high-pass: if true then it will filter low frequencies
  * state: if zero then state will be allocated.
  * result-samples: owned by the caller. length must be at least source-len"
  status-declare
  (declare carryover-len sp-sample-count-t)
  (set carryover-len
    (if* *result-state (- (: *result-state ir-len) 1)
      0))
  ; create/update the impulse response kernel
  (status-require
    (sp-windowed-sinc-state-set
      sample-rate
      cutoff
      transition
      (if* is-high-pass sp-windowed-sinc-hp-ir
        sp-windowed-sinc-ir)
      result-state))
  ; convolve
  (sp-convolve
    source
    source-len
    (: *result-state ir)
    (: *result-state ir-len) carryover-len (: *result-state carryover) result-samples)
  (label exit
    (return status)))