(define (sp-convolve a a-len b b-len carryover-len carryover out)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-time-t sp-sample-t* sp-sample-t*)
  "discrete linear convolution for sample arrays, possibly of a continuous stream. maps segments (a, a_len) to out
   using (b, b_len) as the impulse response. b_len must be greater than zero.
   all heap memory is owned and allocated by the caller.
   out length is a_len.
   carryover is previous carryover or an empty array.
   carryover length must at least b_len - 1.
   carryover_len should be zero for the first call or its content should be zeros.
   carryover_len for subsequent calls should be b_len - 1.
   if b_len changed it should be b_len - 1 from the previous call for the first call with the changed b_len.
   if b_len is one then there is no carryover.
   if a_len is smaller than b_len then, with the current implementation, additional performance costs ensue from shifting the carryover array each call.
   carryover is the extension of out for generated values that dont fit into out,
   as a and b are always fully convolved"
  (declare size sp-time-t a-index sp-time-t b-index sp-time-t c-index sp-time-t)
  (sc-comment "prepare out and carryover")
  (if carryover-len
    (if (<= carryover-len a-len)
      (begin
        (sc-comment "copy all entries to out and reset")
        (memcpy out carryover (* carryover-len (sizeof sp-sample-t)))
        (sp-samples-zero carryover carryover-len)
        (sp-samples-zero (+ carryover-len out) (- a-len carryover-len)))
      (begin
        (sc-comment "carryover is larger. move all carryover entries that fit into out")
        (memcpy out carryover (* a-len (sizeof sp-sample-t)))
        (memmove carryover (+ a-len carryover) (* (- carryover-len a-len) (sizeof sp-sample-t)))
        (sp-samples-zero (+ (- carryover-len a-len) carryover) a-len)))
    (sp-samples-zero out a-len))
  (sc-comment "process values that dont lead to carryover")
  (set size (if* (< a-len b-len) 0 (- a-len (- b-len 1))))
  (if size (sp-convolve-one a size b b-len out))
  (sc-comment "process values with carryover")
  (for ((set a-index size) (< a-index a-len) (set+ a-index 1))
    (for ((set b-index 0) (< b-index b-len) (set+ b-index 1))
      (set c-index (+ a-index b-index))
      (if (< c-index a-len)
        (set+ (array-get out c-index) (* (array-get a a-index) (array-get b b-index)))
        (begin
          (set c-index (- c-index a-len))
          (set+ (array-get carryover c-index) (* (array-get a a-index) (array-get b b-index))))))))

(define (sp-convolution-filter-state-free state) (void sp-convolution-filter-state-t*)
  (if (not state) return)
  (free state:ir)
  (free state:carryover)
  (free state:ir-f-arguments)
  (free state))

(define (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  "create or update a previously created state object.
   impulse response array properties are calculated with ir_f using ir_f_arguments.
   eventually frees state.ir.
   the state object is used to store the impulse response, the parameters that where used to create it and
   overlapping data that has to be carried over between calls.
   ir_f_arguments can be stack allocated and will be copied to state on change"
  status-declare
  (declare
    carryover sp-sample-t*
    carryover-alloc-len sp-time-t
    ir sp-sample-t*
    ir-len sp-time-t
    state sp-convolution-filter-state-t*)
  (memreg-init 2)
  (sc-comment
    "create state if not exists. re-use if exists and return early if ir needs not be updated")
  (if *out-state
    (begin
      (sc-comment "existing")
      (set state *out-state)
      (if
        (and (= state:ir-f ir-f) (= ir-f-arguments-len state:ir-f-arguments-len)
          (= 0 (memcmp state:ir-f-arguments ir-f-arguments ir-f-arguments-len)))
        (begin (sc-comment "unchanged") status-return)
        (begin
          (sc-comment "changed")
          (if (> ir-f-arguments-len state:ir-f-arguments-len)
            (status-require (sph-realloc ir-f-arguments-len &state:ir-f-arguments)))
          (if state:ir (free state:ir)))))
    (begin
      (sc-comment "new")
      (status-require (sph-malloc (sizeof sp-convolution-filter-state-t) &state))
      (status-require (sph-malloc ir-f-arguments-len &state:ir-f-arguments))
      (memreg-add state)
      (memreg-add state:ir-f-arguments)
      (set
        state:carryover-alloc-len 0
        state:carryover 0
        state:ir-f ir-f
        state:ir-f-arguments-len ir-f-arguments-len)))
  (memcpy state:ir-f-arguments ir-f-arguments ir-f-arguments-len)
  (sc-comment "assumes that ir-len is always greater zero")
  (status-require (ir-f ir-f-arguments &ir &ir-len))
  (sc-comment
    "eventually extend carryover array. the array is never shrunk."
    "carryover_len is at least ir_len - 1."
    "carryover_alloc_len is the length of the whole array."
    "new and extended areas must be set to zero")
  (set carryover-alloc-len (- ir-len 1))
  (if state:carryover
    (begin
      (set carryover state:carryover)
      (if (> ir-len state:ir-len)
        (begin
          (if (> carryover-alloc-len state:carryover-alloc-len)
            (begin
              (status-require (sph-realloc (* carryover-alloc-len (sizeof sp-sample-t)) &carryover))
              (set state:carryover-alloc-len carryover-alloc-len)))
          (sc-comment "in any case reset the extended area")
          (memset (+ (- state:ir-len 1) carryover) 0
            (* (- ir-len state:ir-len) (sizeof sp-sample-t))))))
    (begin
      (if carryover-alloc-len (status-require (sp-samples-new carryover-alloc-len &carryover))
        (set carryover 0))
      (set state:carryover-alloc-len carryover-alloc-len)))
  (set
    state:carryover carryover
    state:carryover-len (- ir-len 1)
    state:ir ir
    state:ir-len ir-len
    *out-state state)
  (label exit (if status-is-failure memreg-free) status-return))

(define
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t sp-sample-t* sp-time-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  "convolute samples \"in\", which can be a segment of a continuous stream, with an impulse response
   kernel created by ir_f with ir_f_arguments. can be used for many types of convolution with dynamic impulse response.
   ir_f is only used when ir_f_arguments changed.
   values that need to be carried over with calls are kept in out_state.
   * out_state: if zero then state will be allocated. owned by caller
   * out_samples: owned by the caller. length must be at least in_len and the number of output samples will be in_len"
  status-declare
  (define carryover-len sp-time-t (if* *out-state (: *out-state carryover-len) 0))
  (sc-comment "create/update the impulse response kernel")
  (status-require
    (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state))
  (sc-comment "convolve")
  (sp-convolve in in-len
    (: *out-state ir) (: *out-state ir-len) carryover-len (: *out-state carryover) out-samples)
  (label exit status-return))

(define (sp-bessel-i0 value) (sp-sample-t sp-sample-t)
  (declare
    absolute-value sp-sample-t
    squared-value sp-sample-t
    quarter-squared-value sp-sample-t
    term-value sp-sample-t
    sum-value sp-sample-t
    threshold-value sp-sample-t
    step-index int)
  (set
    absolute-value (fabs value)
    squared-value (* absolute-value absolute-value)
    quarter-squared-value (* 0.25 squared-value)
    sum-value 1.0
    term-value 1.0
    threshold-value 1.0e-16
    step-index 0)
  (do-while (>= term-value (* threshold-value sum-value))
    (set
      step-index (+ step-index 1)
      term-value (* term-value (/ quarter-squared-value (* step-index step-index)))
      sum-value (+ sum-value term-value)))
  (return sum-value))

(define (sp-kaiser-window-length transition-width beta-value) (sp-time-t sp-sample-t sp-sample-t)
  (declare
    attenuation-value sp-sample-t
    numerator-value sp-sample-t
    denominator-value sp-sample-t
    raw-length-value sp-sample-t
    window-length sp-time-t)
  (set
    attenuation-value (+ 8.7 (/ beta-value 0.1102))
    numerator-value (- attenuation-value 7.95)
    denominator-value (* 2.285 2.0 sp-pi transition-width)
    raw-length-value (/ numerator-value denominator-value)
    window-length (convert-type (ceil raw-length-value) sp-time-t)
    window-length (+ (* 2 (/ (+ window-length 2) 2)) 1))
  (return (if* (= 0 window-length) 9 window-length)))

(define (sp-kaiser-window sample-index window-length beta-value)
  (sp-sample-t sp-time-t sp-time-t sp-sample-t)
  (declare
    position-value sp-sample-t
    absolute-position-value sp-sample-t
    argument-value sp-sample-t
    numerator-value sp-sample-t
    denominator-value sp-sample-t)
  (set
    position-value (- (/ (* 2.0 sample-index) (- window-length 1.0)) 1.0)
    absolute-position-value (abs position-value))
  (if (>= absolute-position-value 1.0) (return 0.0))
  (set
    argument-value (* beta-value (sqrt (- 1.0 (* position-value position-value))))
    numerator-value (sp-bessel-i0 argument-value)
    denominator-value (sp-bessel-i0 beta-value))
  (return (/ numerator-value denominator-value)))

(define (sp-sinc-make-minimum-phase impulse-response sample-count)
  (status-t sp-sample-t* sp-time-t)
  status-declare
  (declare
    real-value-list sp-sample-t*
    imaginary-value-list sp-sample-t*
    sample-index sp-time-t
    magnitude-value sp-sample-t
    phase-value sp-sample-t)
  (set real-value-list impulse-response)
  (status-require (sp-calloc-type sample-count sp-sample-t &imaginary-value-list))
  (sp-fft sample-count real-value-list imaginary-value-list)
  (set sample-index 0)
  (while (< sample-index sample-count)
    (set
      magnitude-value
      (sqrt
        (+ (* (array-get real-value-list sample-index) (array-get real-value-list sample-index))
          (* (array-get imaginary-value-list sample-index)
            (array-get imaginary-value-list sample-index))))
      (array-get real-value-list sample-index) (log (+ magnitude-value 1.0e-20))
      (array-get imaginary-value-list sample-index) 0.0
      sample-index (+ sample-index 1)))
  (sp-ffti sample-count real-value-list imaginary-value-list)
  (set sample-index 0)
  (while (< sample-index sample-count)
    (set
      (array-get real-value-list sample-index)
      (/ (array-get real-value-list sample-index) (convert-type sample-count sp-sample-t))
      sample-index (+ sample-index 1)))
  (if (= (modulo sample-count 2) 0)
    (begin
      (set sample-index 1)
      (while (< sample-index (/ sample-count 2))
        (set
          (array-get real-value-list sample-index) (* (array-get real-value-list sample-index) 2.0)
          (array-get real-value-list (- sample-count sample-index)) 0.0
          sample-index (+ sample-index 1))))
    (begin
      (set sample-index 1)
      (while (<= sample-index (/ sample-count 2))
        (set
          (array-get real-value-list sample-index) (* (array-get real-value-list sample-index) 2.0)
          (array-get real-value-list (- sample-count sample-index)) 0.0
          sample-index (+ sample-index 1)))))
  (sp-fft sample-count real-value-list imaginary-value-list)
  (set sample-index 0)
  (while (< sample-index sample-count)
    (set
      magnitude-value (exp (array-get real-value-list sample-index))
      phase-value (array-get imaginary-value-list sample-index)
      (array-get real-value-list sample-index) (* magnitude-value (cos phase-value))
      (array-get imaginary-value-list sample-index) (* magnitude-value (sin phase-value))
      sample-index (+ sample-index 1)))
  (sp-ffti sample-count real-value-list imaginary-value-list)
  (set sample-index 0)
  (while (< sample-index sample-count)
    (set
      (array-get real-value-list sample-index)
      (/ (array-get real-value-list sample-index) (convert-type sample-count sp-sample-t))
      sample-index (+ sample-index 1)))
  (free imaginary-value-list)
  (label exit status-return))

(define (sp-resonator-normalize-unit-energy impulse-response sample-count)
  (void sp-sample-t* sp-time-t)
  (declare sample-index sp-time-t energy-value sp-sample-t gain-value sp-sample-t)
  (set energy-value 0.0 sample-index 0)
  (while (< sample-index sample-count)
    (set
      energy-value
      (+ energy-value
        (* (array-get impulse-response sample-index) (array-get impulse-response sample-index)))
      sample-index (+ sample-index 1)))
  (if (<= energy-value 0.0) (return))
  (set gain-value (/ 1.0 (sqrt energy-value)) sample-index 0)
  (while (< sample-index sample-count)
    (set
      (array-get impulse-response sample-index)
      (* (array-get impulse-response sample-index) gain-value)
      sample-index (+ sample-index 1))))

(define (sp-resonator-ir cutoff-low cutoff-high transition out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t** sp-time-t*)
  status-declare
  (declare
    beta-value sp-sample-t
    sample-count sp-time-t
    center-index sp-sample-t
    impulse-response sp-sample-t*
    sample-index sp-time-t
    delta-index sp-sample-t
    low-value sp-sample-t
    high-value sp-sample-t
    window-value sp-sample-t)
  (set beta-value 8.6)
  (if (< cutoff-low 0.0) (set cutoff-low 0.0))
  (if (> cutoff-high 0.5) (set cutoff-high 0.5))
  (if (<= transition 0.0) (set transition (- cutoff-high cutoff-low)))
  (set
    sample-count (sp-kaiser-window-length transition beta-value)
    center-index (* (- (convert-type sample-count sp-sample-t) 1.0) 0.5))
  (status-require (sph-malloc (* sample-count (sizeof sp-sample-t)) &impulse-response))
  (set sample-index 0)
  (while (< sample-index sample-count)
    (set
      delta-index (- (convert-type sample-index sp-sample-t) center-index)
      low-value (* 2.0 cutoff-low (sp-sinc (* 2.0 cutoff-low delta-index)))
      high-value (* 2.0 cutoff-high (sp-sinc (* 2.0 cutoff-high delta-index)))
      (array-get impulse-response sample-index) (- high-value low-value)
      window-value (sp-kaiser-window sample-index sample-count beta-value)
      (array-get impulse-response sample-index)
      (* (array-get impulse-response sample-index) window-value)
      sample-index (+ sample-index 1)))
  (sp-resonator-normalize-unit-energy impulse-response sample-count)
  (status-require (sp-sinc-make-minimum-phase impulse-response sample-count))
  (set *out-ir impulse-response *out-len sample-count)
  (label exit status-return))

(define (sp-resonator-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  (declare cutoff-low sp-sample-t cutoff-high sp-sample-t transition sp-sample-t)
  (set
    cutoff-low (pointer-get (convert-type arguments sp-sample-t*))
    cutoff-high (pointer-get (+ (convert-type arguments sp-sample-t*) 1))
    transition (pointer-get (+ (convert-type arguments sp-sample-t*) 2)))
  (return (sp-resonator-ir cutoff-low cutoff-high transition out-ir out-len)))
