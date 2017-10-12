(sc-comment
  "implementation of a hq windowed sinc filter with a blackman window (common truncated version) for continuous streams.
  variable sample-rate, cutoff radian frequency and transition band width per call.
  depends on some sp functions defined in main.sc")

(pre-define (sp-windowed-sinc-cutoff freq sample-rate)
  "f32-s integer -> f32-s
  radians-per-second samples-per-second -> cutoff-value"
  (/ (* 2 M_PI freq) sample-rate))

(define (sp-windowed-sinc-ir-length transition) (size-t f32-s)
  "approximate impulse response length for a transition factor and
  ensure that the length is odd"
  (define result b32 (ceil (/ 4 transition))) (if (not (modulo result 2)) (inc result))
  (return result))

(define-type sp-windowed-sinc-state-t
  ; stores impulse response, parameters to create the current impulse response,
  ; and data needed for the next call
  (struct (data sp-sample-t*) (data-len size-t)
    (ir sp-sample-t*) (ir-len size-t) (sample-rate b32) (freq f32-s) (transition f32-s)))

(define (sp-windowed-sinc-ir result result-len sample-rate freq transition)
  (b0 sp-sample-t** size-t* f32-s* b32 f32-s f32-s)
  "create an impulse response kernel for a windowed sinc filter. uses a blackman window (truncated version).
  allocates result, sets result-len"
  (define len size-t (sp-windowed-sinc-ir-length transition)) (set (deref result-len) len)
  (define center-index f32-s (/ (- len 1.0) 2.0))
  (define cutoff f32-s (sp-windowed-sinc-cutoff freq sample-rate))
  (define result-temp sp-sample-t* (malloc (* len (sizeof sp-sample-t))))
  (set result-temp (malloc (* len (sizeof sp-sample-t))))
  (if (not result-temp) (begin (set (deref result) 0) (return))) (define index size-t 0)
  (while (< index len)
    (set (deref result-temp index) (sp-blackman (sinc (* 2 cutoff (- index center-index))) len))
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
  eventually frees state.ir"
  (define state-temp sp-windowed-sinc-state-t*)
  (if (not state)
    (begin (set state-temp (malloc (sizeof sp-windowed-sinc-state-t)))
      (if (not state-temp) (return 1))
      (struct-pointer-set state-temp sample-rate 0 freq 0 transition 0 data 0 data-len 0))
    (set state-temp (deref state)))
  (if
    (and (= (struct-pointer-get state-temp sample-rate) sample-rate)
      (= (struct-pointer-get state-temp freq) freq)
      (= (struct-pointer-get state-temp transition) transition))
    (return 0))
  (if state (free (struct-get state-temp ir))) (define ir-len size-t)
  (define ir sp-sample-t*)
  (sp-windowed-sinc-ir (address-of ir) (address-of ir-len) sample-rate freq transition)
  (if (not ir) (begin (if (not state) (free state-temp)) (return 1)))
  (if (< ir-len (struct-pointer-ref state-temp data-len))
    (struct-pointer-set state-temp data-len ir-len)
    (define data sp-sample-t* (malloc (* ir-len (sizeof sp-sample-t)))))
  (if (not data) (begin (free ir) (if (not state) (free state-temp)) (return 1)))
  (struct-pointer-set state-temp data
    data data-len ir ir ir-len ir-len sample-rate sample-rate freq freq transition transition)
  (set (deref state) state-temp) (return 0))

(define
  (sp-windowed-sinc result source source-len prev prev-len next next-len start end sample-rate freq
    transition
    state)
  (b8 sp-sample-t* sp-sample-t*
    size-t sp-sample-t*
    size-t sp-sample-t* size-t size-t size-t b32 f32-s f32-s sp-windowed-sinc-state-t**)
  "a windowed sinc filter for segments of continuous streams with
  sample-rate, frequency and transition variable per call.
  state can be zero and it will be allocated"
  (if (sp-windowed-sinc-state-create sample-rate freq transition state) (return 1))
  (sp-convolve result source
    source-len (struct-pointer-get (deref state) ir)
    (struct-pointer-get (deref state) ir-len) (struct-pointer-get (deref state) data)
    (struct-pointer-get (deref state) data-len))
  (return 0))

(define
  (scm-sp-windowed-sinc! result source scm-prev scm-next sample-rate freq transition start end)
  (SCM SCM SCM SCM SCM SCM SCM SCM SCM SCM)
  (define source-len b32 (octets->samples (SCM-BYTEVECTOR-LENGTH source)))
  (define prev sp-sample-t* prev-len b32 next sp-sample-t* next-len b32)
  (optional-samples prev prev-len scm-prev) (optional-samples next next-len scm-next)
  (sp-windowed-sinc (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) source-len
    prev prev-len
    next next-len
    (optional-index start 0) (optional-index end (- source-len 1))
    (scm->uint32 sample-rate) (scm->double freq) (scm->double transition))
  (return SCM-UNSPECIFIED))
