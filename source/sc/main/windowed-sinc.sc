(sc-comment
  "implementation of a hq windowed sinc filter with a blackman window (common truncated version) for continuous streams.
  variable sample-rate, cutoff radian frequency and transition band width per call.
  depends on some sp functions defined in main.sc and sph-sp.sc")

(define (sp-windowed-sinc-ir-length transition) (size-t f32-s)
  "approximate impulse response length for a transition factor and
  ensure that the length is odd"
  (define result b32 (ceil (/ 4 transition))) (if (not (modulo result 2)) (inc result))
  (return result))

(define (sp-windowed-sinc-ir result result-len sample-rate freq transition)
  (b0 sp-sample-t** size-t* b32 f32-s f32-s)
  "create an impulse response kernel for a windowed sinc filter. uses a blackman window (truncated version).
  allocates result, sets result-len.
  failed if result is null"
  (define len size-t (sp-windowed-sinc-ir-length transition)) (set (deref result-len) len)
  (define center-index f32-s (/ (- len 1.0) 2.0))
  (define cutoff f32-s (sp-windowed-sinc-cutoff freq sample-rate))
  (define result-temp sp-sample-t* (malloc (* len (sizeof sp-sample-t))))
  (set result-temp (malloc (* len (sizeof sp-sample-t))))
  (if (not result-temp) (begin (set (deref result) 0) (return))) (define index size-t 0)
  (while (< index len)
    (set (deref result-temp index) (sp-window-blackman (sp-sinc (* 2 cutoff (- index center-index))) len))
    (inc index))
  (define result-sum f32-s (float-sum result-temp len))
  (while len (dec len) (set (deref result-temp index) (/ (deref result-temp index) result-sum)))
  (set (deref result) result-temp))

(define (sp-windowed-sinc-state-destroy state) (b0 sp-windowed-sinc-state-t*)
  (free (struct-pointer-get state ir)) (free (struct-pointer-get state data)) (free state))

(define (sp-windowed-sinc-state-create sample-rate freq transition state)
  (b8 b32 f32-s f32-s sp-windowed-sinc-state-t**)
  "create or update a state object. impulse response array properties are calculated
  from sample-rate, freq and transition.
  eventually frees state.ir.
  ir-len-prev data elements have to be copied to the next result"
  ; create state if not exists
  (define state-temp sp-windowed-sinc-state-t*)
  (if (not (deref state))
    (begin (set state-temp (malloc (sizeof sp-windowed-sinc-state-t)))
      (if (not state-temp) (return 1))
      (struct-pointer-set state-temp sample-rate
        0 freq 0 transition 0 data 0 data-len 0 ir-len-prev 0))
    (set state-temp (deref state)))
  ; re-use ir, nothing changed
  (if
    (and (= (struct-pointer-get state-temp sample-rate) sample-rate)
      (= (struct-pointer-get state-temp freq) freq)
      (= (struct-pointer-get state-temp transition) transition))
    (return 0))
  ; replace ir
  (if state (begin (free (struct-pointer-get state-temp ir)))) (define ir-len size-t)
  (define ir sp-sample-t*)
  (sp-windowed-sinc-ir (address-of ir) (address-of ir-len) sample-rate freq transition)
  (if (not ir) (begin (if (not (deref state)) (free state-temp)) (return 1)))
  (struct-pointer-set state-temp ir-len-prev
    (if* state (struct-pointer-get state-temp ir-len) ir-len))
  ; set bigger data buffer if needed
  (if (> ir-len (struct-pointer-get state-temp data-len))
    (begin (define data sp-sample-t* (calloc ir-len (sizeof sp-sample-t)))
      (if (not data) (begin (free ir) (if (not (deref state)) (free state-temp)) (return 1)))
      (if (struct-pointer-get state-temp data)
        (begin
          (memcpy data (struct-pointer-get state-temp data)
            (* (struct-pointer-get state-temp ir-len-prev) (sizeof sp-sample-t)))
          (free (struct-pointer-get state-temp data))))
      (struct-pointer-set state-temp data data data-len ir-len)))
  (struct-pointer-set state-temp ir
    ir ir-len ir-len sample-rate sample-rate freq freq transition transition)
  (set (deref state) state-temp) (return 0))

(define (sp-windowed-sinc result source source-len sample-rate freq transition state)
 (status-i-t sp-sample-t* sp-sample-t* size-t b32 f32-s f32-s sp-windowed-sinc-state-t**)
  "a windowed sinc filter for segments of continuous streams with
  sample-rate, frequency and transition variable per call.
  state can be zero and it will be allocated"
  (if (sp-windowed-sinc-state-create sample-rate freq transition state) (return 1))
  (sp-convolve result source
    source-len (struct-pointer-get (deref state) ir)
    (struct-pointer-get (deref state) ir-len) (struct-pointer-get (deref state) data)
    (struct-pointer-get (deref state) ir-len-prev))
  (return 0))
