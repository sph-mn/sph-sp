(sc-comment
  "implementation of a hq windowed sinc filter with a blackman window (common truncated version) for continuous streams.
  variable sample-rate, cutoff radian frequency and transition band width per call.
  depends on some sp functions defined in main.sc and sph-sp.sc.
  sp-windowed-sinc-state-t is used to store impulse response, parameters to create the current impulse response,
  and data needed for the next call")

(define (sp-window-blackman a width) (sp-float-t sp-float-t size-t)
  (return
    (+
      (- 0.42 (* 0.5 (cos (/ (* 2 M_PI a) (- width 1))))) (* 0.8 (cos (/ (* 4 M_PI a) (- width 1)))))))

(define (sp-windowed-sinc-ir-length transition) (size-t sp-float-t)
  "approximate impulse response length for a transition factor and
  ensure that the length is odd"
  (declare result size-t)
  (set result (ceil (/ 4 transition)))
  (if (not (modulo result 2)) (set result (+ 1 result)))
  (return result))

(define (sp-windowed-sinc-ir sample-rate freq transition result-len result-ir)
  (status-t sp-sample-rate-t sp-float-t sp-float-t size-t* sp-sample-t**)
  "create an impulse response kernel for a windowed sinc filter. uses a blackman window (truncated version).
  allocates result, sets result-len"
  status-declare
  (declare
    center-index sp-float-t
    cutoff sp-float-t
    i size-t
    len size-t
    sum sp-float-t
    ir sp-sample-t*)
  (set
    len (sp-windowed-sinc-ir-length transition)
    center-index (/ (- len 1.0) 2.0)
    cutoff (sp-windowed-sinc-cutoff freq sample-rate))
  (status-require (sph-helper-malloc (* len (sizeof sp-sample-t)) &ir))
  (for ((set i 0) (< i len) (set i (+ 1 i)))
    (set (array-get ir i) (sp-window-blackman (sp-sinc (* 2 cutoff (- i center-index))) len)))
  (set sum (sp-sample-sum ir len))
  (while len
    (set
      len (- len 1)
      (array-get ir i) (/ (array-get ir i) sum)))
  (set
    *result-ir ir
    *result-len len)
  (label exit
    (return status)))

(define (sp-windowed-sinc-state-free state) (void sp-windowed-sinc-state-t*)
  (free state:ir)
  (free state:carryover)
  (free state))

(define (sp-windowed-sinc-state-create sample-rate freq transition result-state)
  (status-t sp-sample-rate-t sp-float-t sp-float-t sp-windowed-sinc-state-t**)
  "create or update a previously created state object. impulse response array properties are calculated
  from sample-rate, freq and transition.
  eventually frees state.ir.
  ir-len-prev carryover elements have to be copied to the next result"
  status-declare
  (declare
    carryover sp-sample-t*
    ir sp-sample-t*
    ir-len size-t
    state sp-windowed-sinc-state-t*)
  (memreg-init 2)
  (sc-comment "create state if not exists")
  (if *result-state (set state *result-state)
    (begin
      (status-require (sph-helper-malloc (sizeof sp-windowed-sinc-state-t) &state))
      (memreg-add state)
      (set
        state:sample-rate 0
        state:freq 0
        state:ir 0
        state:transition 0
        state:carryover 0
        state:carryover-len 0
        state:ir-len-prev 0)))
  (sc-comment "re-use ir if nothing changed")
  (if (and (= state:sample-rate sample-rate) (= state:freq freq) (= state:transition transition))
    (goto exit))
  (sc-comment "replace ir")
  (if result-state (free state:ir))
  (status-require (sp-windowed-sinc-ir sample-rate freq transition &ir-len &ir))
  (set state:ir-len-prev
    (if* result-state state:ir-len
      ir-len))
  (sc-comment "set bigger carryover buffer if needed")
  (if (> ir-len state:carryover-len)
    (begin
      (status-require (sph-helper-calloc (* ir-len (sizeof sp-sample-t)) &carryover))
      (memreg-add carryover)
      (if state:carryover
        (begin
          (memcpy carryover state:carryover (* state:ir-len-prev (sizeof sp-sample-t)))
          (free state:carryover)))
      (set
        state:carryover carryover
        state:carryover-len ir-len)))
  (set
    state:ir ir
    state:ir-len ir-len
    state:sample-rate sample-rate
    state:freq freq
    state:transition transition
    *result-state state)
  (label exit
    (if status-is-failure memreg-free)
    (return status)))

(define
  (sp-windowed-sinc source source-len sample-rate freq transition result-state result-samples)
  (status-t
    sp-sample-t* size-t uint32-t sp-float-t sp-float-t sp-windowed-sinc-state-t** sp-sample-t*)
  "a windowed sinc filter for segments of continuous streams with
  possibly variable sample-rate, frequency and transition per call.
  state can be zero and it will be allocated.
  result-samples length is source-len"
  status-declare
  (status-require (sp-windowed-sinc-state-create sample-rate freq transition result-state))
  (sp-convolve
    source
    source-len
    (: *result-state ir)
    (: *result-state ir-len) (: *result-state ir-len-prev) (: *result-state carryover) result-samples)
  (label exit
    (return status)))