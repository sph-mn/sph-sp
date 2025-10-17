(define (sp-convolution-filter-state-free state) (void sp-convolution-filter-state-t*)
  (if (not state) return)
  (free state:ir)
  (free state:carryover)
  (free state:ir-f-arguments)
  (free state))

(define (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  "create or update a previously created state object.
   impulse response array properties are calculated with ir-f using ir-f-arguments.
   eventually frees state.ir.
   the state object is used to store the impulse response, the parameters that where used to create it and
   overlapping data that has to be carried over between calls.
   ir-f-arguments can be stack allocated and will be copied to state on change"
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
    "carryover-len is at least ir-len - 1."
    "carryover-alloc-len is the length of the whole array."
    "new and extended areas must be set to zero")
  (set carryover-alloc-len (- ir-len 1))
  (if state:carryover
    (begin
      (set carryover state:carryover)
      (if (> ir-len state:ir-len)
        (begin
          (if (> carryover-alloc-len state:carryover-alloc-len)
            (begin
              (status-require
                (sph-realloc (* carryover-alloc-len (sizeof sp-sample-t)) &carryover))
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
   kernel created by ir-f with ir-f-arguments. can be used for many types of convolution with dynamic impulse response.
   ir-f is only used when ir-f-arguments changed.
   values that need to be carried over with calls are kept in out-state.
   * out-state: if zero then state will be allocated. owned by caller
   * out-samples: owned by the caller. length must be at least in-len and the number of output samples will be in-len"
  status-declare
  (define carryover-len sp-time-t (if* *out-state (: *out-state carryover-len) 0))
  (sc-comment "create/update the impulse response kernel")
  (status-require
    (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state))
  (sc-comment "convolve")
  (sp-convolve in in-len
    (: *out-state ir) (: *out-state ir-len) carryover-len (: *out-state carryover) out-samples)
  (label exit status-return))

(define (sp-window-blackman a width) (sp-sample-t sp-sample-t sp-time-t)
  (return
    (+ (- 0.42 (* 0.5 (cos (/ (* 2 M_PI a) (- width 1)))))
      (* 0.08 (cos (/ (* 4 M_PI a) (- width 1)))))))

(define (sp-windowed-sinc-lp-hp-ir-length transition) (sp-time-t sp-sample-t)
  "approximate impulse response length for a transition factor and
   ensure that the length is odd"
  (declare a sp-time-t)
  (set a (ceil (/ 4 transition)))
  (if (= 0 (modulo a 2)) (set+ a 1))
  (return a))

(define (sp-null-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (set *out-len 1)
  (return (sp-samples-new 1 out-ir)))

(define (sp-passthrough-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  status-declare
  (status-require (sph-malloc (sizeof sp-sample-t) out-ir))
  (set **out-ir 1 *out-len 1)
  (label exit status-return))

(define (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-bool-t sp-sample-t** sp-time-t*)
  "create an impulse response kernel for a windowed sinc low-pass or high-pass filter.
   uses a truncated blackman window.
   allocates out-ir, sets out-len.
   cutoff and transition are as fraction 0..0.5 of the sampling rate"
  status-declare
  (declare center-index sp-sample-t i sp-time-t ir sp-sample-t* len sp-time-t sum sp-sample-t)
  (set len (sp-windowed-sinc-lp-hp-ir-length transition) center-index (/ (- len 1.0) 2.0))
  (status-require (sph-malloc (* len (sizeof sp-sample-t)) &ir))
  (sc-comment
    "set the windowed sinc"
    "nan can be set here if the freq and transition values are invalid")
  (for ((set i 0) (< i len) (set i (+ 1 i)))
    (set (array-get ir i) (* (sp-window-blackman i len) (sp-sinc (* 2 cutoff (- i center-index))))))
  (sc-comment "scale to get unity gain. adjust sum of ir values to be 1.0")
  (set sum (sp-samples-sum ir len))
  (for ((set i 0) (< i len) (set+ i 1)) (set/ (array-get ir i) sum))
  (if is-high-pass (sp-spectral-inversion-ir ir len))
  (set *out-ir ir *out-len len)
  (label exit status-return))

(define
  (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-bool-t sp-sample-t** sp-time-t*)
  "like sp-windowed-sinc-ir-lp but for a band-pass or band-reject filter.
   if one cutoff is at or above maximum then create only either low-pass or high-pass"
  status-declare
  (declare
    hp-ir sp-sample-t*
    hp-len sp-time-t
    lp-ir sp-sample-t*
    lp-len sp-time-t
    i sp-time-t
    start sp-time-t
    end sp-time-t
    out sp-sample-t*
    over sp-sample-t*)
  (if is-reject
    (begin
      (if (>= 0.0 cutoff-l)
        (if (<= 0.5 cutoff-h) (return (sp-null-ir out-ir out-len))
          (return (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #t out-ir out-len)))
        (if (<= 0.5 cutoff-h)
          (return (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #f out-ir out-len))))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #f &lp-ir &lp-len))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #t &hp-ir &hp-len))
      (sc-comment "prepare to add the shorter ir to the longer one center-aligned")
      (if (> lp-len hp-len)
        (set
          start (- (/ (- lp-len 1) 2) (/ (- hp-len 1) 2))
          end (+ hp-len start)
          out lp-ir
          over hp-ir
          *out-len lp-len)
        (set
          start (- (/ (- hp-len 1) 2) (/ (- lp-len 1) 2))
          end (+ lp-len start)
          out hp-ir
          over lp-ir
          *out-len hp-len))
      (sc-comment "sum lp and hp ir samples")
      (for ((set i start) (< i end) (set i (+ 1 i)))
        (set (array-get out i) (+ (array-get over (- i start)) (array-get out i))))
      (free over)
      (set *out-ir out))
    (begin
      (sc-comment "meaning of cutoff low/high is switched. cutoff-l leaves higher frequencies")
      (if (>= 0.0 cutoff-l)
        (if (<= 0.5 cutoff-h) (return (sp-passthrough-ir out-ir out-len))
          (return (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #f out-ir out-len)))
        (if (<= 0.5 cutoff-h)
          (return (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #t out-ir out-len))))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #t &hp-ir &hp-len))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #f &lp-ir &lp-len))
      (sc-comment "convolve lp and hp ir samples")
      (set *out-len (- (+ lp-len hp-len) 1))
      (status-require (sp-samples-new *out-len out-ir))
      (sp-convolve-one lp-ir lp-len hp-ir hp-len *out-ir)))
  (label exit status-return))

(define (sp-windowed-sinc-lp-hp-ir-f arguments out-ir out-len)
  (status-t void* sp-sample-t** sp-time-t*)
  "maps arguments from the generic ir-f-arguments array.
   arguments is (sp-sample-t:cutoff sp-sample-t:transition sp-bool-t:is-high-pass)"
  (declare cutoff sp-sample-t transition sp-sample-t is-high-pass sp-bool-t)
  (set
    cutoff (pointer-get (convert-type arguments sp-sample-t*))
    transition (pointer-get (+ 1 (convert-type arguments sp-sample-t*)))
    is-high-pass (pointer-get (convert-type (+ 2 (convert-type arguments sp-sample-t*)) sp-bool-t*)))
  (return (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)))

(define (sp-windowed-sinc-bp-br-ir-f arguments out-ir out-len)
  (status-t void* sp-sample-t** sp-time-t*)
  "maps arguments from the generic ir-f-arguments array.
   arguments is (sp-sample-t:cutoff-l sp-sample-t:cutoff-h sp-sample-t:transition sp-bool-t:is-reject)"
  (declare
    cutoff-l sp-sample-t
    cutoff-h sp-sample-t
    transition-l sp-sample-t
    transition-h sp-sample-t
    is-reject sp-bool-t)
  (set
    cutoff-l (pointer-get (convert-type arguments sp-sample-t*))
    cutoff-h (pointer-get (+ 1 (convert-type arguments sp-sample-t*)))
    transition-l (pointer-get (+ 2 (convert-type arguments sp-sample-t*)))
    transition-h (pointer-get (+ 3 (convert-type arguments sp-sample-t*)))
    is-reject (pointer-get (convert-type (+ 4 (convert-type arguments sp-sample-t*)) sp-bool-t*)))
  (return
    (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)))

(define (sp-windowed-sinc-lp-hp in in-len cutoff transition is-high-pass out-state out-samples)
  (status-t sp-sample-t* sp-time-t sp-sample-t sp-sample-t sp-bool-t sp-convolution-filter-state-t** sp-sample-t*)
  "a windowed sinc low-pass or high-pass filter for segments of continuous streams with
   variable sample-rate, frequency, transition and impulse response type per call.
   reduces volume, might want to use unity gain after processing.
   * cutoff: as a fraction of the sample rate, 0..0.5
   * transition: like cutoff
   * is-high-pass: if true then it will reduce low frequencies
   * out-state: if zero then state will be allocated.
   * out-samples: owned by the caller. length must be at least in-len"
  status-declare
  (declare a (array uint8-t ((+ (sizeof sp-bool-t) (* 2 (sizeof sp-sample-t))))) a-len uint8-t)
  (sc-comment "set arguments array for ir-f")
  (set
    a-len (+ (sizeof sp-bool-t) (* 2 (sizeof sp-sample-t)))
    (pointer-get (convert-type a sp-sample-t*)) cutoff
    (pointer-get (+ 1 (convert-type a sp-sample-t*))) transition
    (pointer-get (convert-type (+ 2 (convert-type a sp-sample-t*)) sp-bool-t*)) is-high-pass)
  (sc-comment "apply filter")
  (status-require
    (sp-convolution-filter in in-len sp-windowed-sinc-lp-hp-ir-f a a-len out-state out-samples))
  (label exit status-return))

(define
  (sp-windowed-sinc-bp-br in in-len cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-bool-t sp-convolution-filter-state-t** sp-sample-t*)
  "like sp-windowed-sinc-lp-hp but for a band-pass or band-reject filter"
  status-declare
  (sc-comment "setup arguments array for ir-f")
  (declare a (array uint8-t ((+ (sizeof sp-bool-t) (* 4 (sizeof sp-sample-t))))) a-len uint8-t)
  (set
    a-len (+ (sizeof sp-bool-t) (* 4 (sizeof sp-sample-t)))
    (pointer-get (convert-type a sp-sample-t*)) cutoff-l
    (pointer-get (+ 1 (convert-type a sp-sample-t*))) cutoff-h
    (pointer-get (+ 2 (convert-type a sp-sample-t*))) transition-l
    (pointer-get (+ 3 (convert-type a sp-sample-t*))) transition-h
    (pointer-get (convert-type (+ 4 (convert-type a sp-sample-t*)) sp-bool-t*)) is-reject)
  (sc-comment "apply filter")
  (status-require
    (sp-convolution-filter in in-len sp-windowed-sinc-bp-br-ir-f a a-len out-state out-samples))
  (label exit status-return))

(define (sp-moving-average in in-size prev next radius out)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t* sp-time-t sp-sample-t*)
  "centered moving average balanced for complete data arrays and seamless for continuous data.
   width: radius * 2 + 1.
   width must be smaller than in-size.
   prev can be 0 or an array with size equal to in-size.
   next can be 0 or an array with size of at least radius.
   for outside values where prev/next is not available, reflections over the x and y axis are used
   so that the first/last value stay the same. for example, [0 1 2 3 0] without prev and
   without next would be interpreted as [-0 -3 -2 -1 0 1 2 3 0 -3 -2 -1 -0]. if only zero were used
   then middle values would have stronger influence on edge values.
   use case: smoothing time domain data arrays, for example amplitude envelopes or input control data"
  (sc-comment
    "offsets to calculate outside values include an increment to account for the first or last index,
     across which values are reflected.
     the for loops correspond to: initial sum, with preceeding outside values, middle values, with succeeding outside values.
     the subtracted value is the first value of the previous window and is therefore
     at an index one less than the first value of the current window")
  (declare i sp-time-t sum sp-sample-t width sp-time-t)
  (set width (+ (* radius 2) 1) sum (array-get in 0))
  (if prev
    (for ((set i 0) (< i radius) (set+ i 1))
      (set+ sum (+ (array-get prev (- in-size i 1)) (array-get in (+ i 1)))))
    (set sum (array-get in 0)))
  (set (array-get out 0) (/ sum width))
  (for ((set i 1) (<= i radius) (set+ i 1))
    (set
      sum
      (- (+ sum (array-get in (+ i radius)))
        (if* prev (array-get prev (+ (- in-size radius 1) i))
          (* (array-get in (+ (- radius i) 1)) -1)))
      (array-get out i) (/ sum width)))
  (for ((set i (+ radius 1)) (< i (- in-size radius)) (set+ i 1))
    (set
      sum (- (+ sum (array-get in (+ i radius))) (array-get in (- i radius 1)))
      (array-get out i) (/ sum width)))
  (for ((set i (- in-size radius)) (< i in-size) (set+ i 1))
    (set
      sum
      (-
        (+ sum
          (if* next (array-get next (- i (- in-size radius)))
            (* (array-get in (- (+ i radius 1) in-size)) -1)))
        (array-get in (- i radius 1)))
      (array-get out i) (/ sum width))))