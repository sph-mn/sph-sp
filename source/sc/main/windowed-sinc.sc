(sc-comment
  "implementation of a hq windowed sinc filter with a blackman window (common truncated version) for continuous streams.
  variable sample-rate, cutoff radian frequency and transition band width per call.
  depends on some sp functions defined in main.sc and sph-sp.sc.
  sp-windowed-sinc-state-t is used to store impulse response, parameters to create the current impulse response,
  and data needed for the next call")

(define (sp-windowed-sinc-ir-length transition) (size-t sp-float-t)
  "approximate impulse response length for a transition factor and
  ensure that the length is odd"
  (declare result uint32-t)
  (set result (ceil (/ 4 transition)))
  (if (not (modulo result 2)) (inc result))
  (return result))

(define (sp-windowed-sinc-ir result result-len sample-rate freq transition)
  (void sp-sample-t** size-t* uint32-t sp-float-t sp-float-t)
  "create an impulse response kernel for a windowed sinc filter. uses a blackman window (truncated version).
  allocates result, sets result-len.
  failed if result is null"
  (declare
    len size-t
    center-index sp-float-t
    cutoff sp-float-t
    result-temp sp-sample-t*
    index size-t
    result-sum sp-float-t)
  (set
    len (sp-windowed-sinc-ir-length transition)
    *result-len len
    center-index (/ (- len 1.0) 2.0)
    cutoff (sp-windowed-sinc-cutoff freq sample-rate)
    result-temp (malloc (* len (sizeof sp-sample-t))))
  (if (not result-temp)
    (begin
      (set *result 0)
      (return)))
  (set index 0)
  (while (< index len)
    (set (array-get result-temp index)
      (sp-window-blackman (sp-sinc (* 2 cutoff (- index center-index))) len))
    (inc index))
  (set result-sum (sp-sample-sum result-temp len))
  (while len
    (set
      len (- len 1)
      (array-get result-temp index) (/ (array-get result-temp index) result-sum)))
  (set *result result-temp))

(define (sp-windowed-sinc-state-destroy state) (void sp-windowed-sinc-state-t*)
  (free state:ir)
  (free state:data)
  (free state))

(define (sp-windowed-sinc-state-create sample-rate freq transition state)
  (uint8-t uint32-t sp-float-t sp-float-t sp-windowed-sinc-state-t**)
  "create or update a state object. impulse response array properties are calculated
  from sample-rate, freq and transition.
  eventually frees state.ir.
  ir-len-prev data elements have to be copied to the next result"
  ; create state if not exists
  (declare
    state-temp sp-windowed-sinc-state-t*
    ir-len size-t
    ir sp-sample-t*
    data sp-sample-t*)
  (if (not *state)
    (begin
      (set state-temp (malloc (sizeof sp-windowed-sinc-state-t)))
      (if (not state-temp) (return 1))
      (set
        state-temp:sample-rate 0
        state-temp:freq 0
        state-temp:transition 0
        state-temp:data 0
        state-temp:data-len 0
        state-temp:ir-len-prev 0))
    (set state-temp *state))
  ; re-use ir, nothing changed
  (if
    (and
      (= state-temp:sample-rate sample-rate)
      (= state-temp:freq freq) (= state-temp:transition transition))
    (return 0))
  ; replace ir
  (if state
    (begin
      (free state-temp:ir)))
  (sp-windowed-sinc-ir &ir &ir-len sample-rate freq transition)
  (if (not ir)
    (begin
      (if (not *state) (free state-temp))
      (return 1)))
  (set state-temp:ir-len-prev
    (if* state state-temp:ir-len
      ir-len))
  ; set bigger data buffer if needed
  (if (> ir-len state-temp:data-len)
    (begin
      (set data (calloc ir-len (sizeof sp-sample-t)))
      (if (not data)
        (begin
          (free ir)
          (if (not *state) (free state-temp))
          (return 1)))
      (if state-temp:data
        (begin
          (memcpy data state-temp:data (* state-temp:ir-len-prev (sizeof sp-sample-t)))
          (free state-temp:data)))
      (set
        state-temp:data data
        state-temp:data-len ir-len)))
  (set
    state-temp:ir ir
    state-temp:ir-len ir-len
    state-temp:sample-rate sample-rate
    state-temp:freq freq
    statet-temp:transition transition)
  (set *state state-temp)
  (return 0))

(define (sp-windowed-sinc result source source-len sample-rate freq transition state)
  (int sp-sample-t* sp-sample-t* size-t uint32-t sp-float-t sp-float-t sp-windowed-sinc-state-t**)
  "a windowed sinc filter for segments of continuous streams with
  sample-rate, frequency and transition variable per call.
  state can be zero and it will be allocated"
  (if (sp-windowed-sinc-state-create sample-rate freq transition state) (return 1))
  (sp-convolve
    result source source-len (: *state ir) (: *state ir-len) (: *state data) (: *state ir-len-prev))
  (return 0))