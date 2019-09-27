(sc-comment
  "implementation of a windowed sinc low-pass and high-pass filter for continuous streams of sample arrays.
  sample-rate, cutoff frequency and transition band width is variable per call.
  build with the information from https://tomroelandts.com/articles/how-to-create-a-simple-low-pass-filter")

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
            (status-require (sph-helper-realloc ir-f-arguments-len &state:ir-f-arguments)))
          (if state:ir (free state:ir)))))
    (begin
      (sc-comment "new")
      (status-require (sph-helper-malloc (sizeof sp-convolution-filter-state-t) &state))
      (status-require (sph-helper-malloc ir-f-arguments-len &state:ir-f-arguments))
      (memreg-add state)
      (memreg-add state:ir-f-arguments)
      (set
        state:carryover-alloc-len 0
        state:carryover-len 0
        state:carryover 0
        state:ir-f ir-f
        state:ir-f-arguments-len ir-f-arguments-len)))
  (memcpy state:ir-f-arguments ir-f-arguments ir-f-arguments-len)
  (status-require (ir-f ir-f-arguments &ir &ir-len))
  (sc-comment "eventually extend carryover array. the array is never shrunk."
    "carryover-len is at least ir-len - 1." "carryover-alloc-len is the length of the whole array."
    "new and extended areas must be set to zero")
  (set carryover-alloc-len (- ir-len 1))
  (if state:carryover
    (begin
      (set carryover state:carryover)
      (if (> ir-len state:ir-len)
        (begin
          (if (> carryover-alloc-len state:carryover-alloc-len)
            (begin
              (status-require (sph-helper-realloc (* carryover-alloc-len (sizeof sp-sample-t)) &carryover))
              (set state:carryover-alloc-len carryover-alloc-len)))
          (sc-comment "in any case reset the extension area")
          (memset (+ (- state:ir-len 1) carryover) 0
            (* (- ir-len state:ir-len) (sizeof sp-sample-t))))))
    (begin
      (status-require (sph-helper-calloc (* carryover-alloc-len (sizeof sp-sample-t)) &carryover))
      (set state:carryover-alloc-len carryover-alloc-len)))
  (set state:carryover carryover state:ir ir state:ir-len ir-len *out-state state)
  (label exit (if status-is-failure memreg-free) status-return))

(define
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  "convolute samples \"in\", which can be a segment of a continuous stream, with an impulse response
  kernel created by ir-f with ir-f-arguments. can be used for many types of convolution with dynamic impulse response.
  ir-f is only used when ir-f-arguments changed.
  values that need to be carried over with calls are kept in out-state.
  * out-state: if zero then state will be allocated. owned by caller
  * out-samples: owned by the caller. length must be at least in-len and the number of output samples will be in-len"
  status-declare
  (declare carryover-len sp-time-t)
  (set carryover-len (if* *out-state (- (: *out-state ir-len) 1) 0))
  (sc-comment "create/update the impulse response kernel")
  (status-require (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state))
  (sc-comment "convolve")
  (sp-convolve in in-len
    (: *out-state ir) (: *out-state ir-len) carryover-len (: *out-state carryover) out-samples)
  (label exit status-return))

(define (sp-window-blackman a width) (sp-float-t sp-float-t sp-time-t)
  (return
    (+ (- 0.42 (* 0.5 (cos (/ (* 2 M_PI a) (- width 1)))))
      (* 0.08 (cos (/ (* 4 M_PI a) (- width 1)))))))

(define (sp-windowed-sinc-lp-hp-ir-length transition) (sp-time-t sp-float-t)
  "approximate impulse response length for a transition factor and
  ensure that the length is odd"
  (declare a sp-time-t)
  (set a (ceil (/ 4 transition)))
  (if (not (modulo a 2)) (set a (+ 1 a)))
  (return a))

(define (sp-null-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  (set *out-len 1)
  (return (sph-helper-calloc (sizeof sp-sample-t) out-ir)))

(define (sp-passthrough-ir out-ir out-len) (status-t sp-sample-t** sp-time-t*)
  status-declare
  (status-require (sph-helper-malloc (sizeof sp-sample-t) out-ir))
  (set **out-ir 1 *out-len 1)
  (label exit status-return))

(define (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-float-t sp-float-t boolean sp-sample-t** sp-time-t*)
  "create an impulse response kernel for a windowed sinc low-pass or high-pass filter.
  uses a truncated blackman window.
  allocates out-ir, sets out-len.
  cutoff and transition are as fraction 0..0.5 of the sampling rate"
  status-declare
  (declare center-index sp-float-t i sp-time-t ir sp-sample-t* len sp-time-t sum sp-float-t)
  (set len (sp-windowed-sinc-lp-hp-ir-length transition) center-index (/ (- len 1.0) 2.0))
  (status-require (sph-helper-malloc (* len (sizeof sp-sample-t)) &ir))
  (sc-comment "set the windowed sinc"
    "nan can be set here if the freq and transition values are invalid")
  (for ((set i 0) (< i len) (set i (+ 1 i)))
    (set (array-get ir i) (* (sp-window-blackman i len) (sp-sinc (* 2 cutoff (- i center-index))))))
  (sc-comment "scale to get unity gain")
  (set sum (sp-sample-sum ir len))
  (for ((set i 0) (< i len) (set i (+ 1 i))) (set (array-get ir i) (/ (array-get ir i) sum)))
  (if is-high-pass (sp-spectral-inversion-ir ir len))
  (set *out-ir ir *out-len len)
  (label exit status-return))

(define
  (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)
  (status-t sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-sample-t** sp-time-t*)
  "like sp-windowed-sinc-ir-lp but for a band-pass or band-reject filter.
  optimisation: if one cutoff is at or above maximum then create only either low-pass or high-pass"
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
      (sc-comment "meaning of cutoff high/low is switched.")
      (if (>= 0.0 cutoff-l)
        (if (<= 0.5 cutoff-h) (return (sp-passthrough-ir out-ir out-len))
          (return (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #f out-ir out-len)))
        (if (<= 0.5 cutoff-h)
          (return (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #t out-ir out-len))))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-l transition-l #t &hp-ir &hp-len))
      (status-require (sp-windowed-sinc-lp-hp-ir cutoff-h transition-h #f &lp-ir &lp-len))
      (sc-comment "convolve lp and hp ir samples")
      (set *out-len (- (+ lp-len hp-len) 1))
      (status-require (sph-helper-calloc (* *out-len (sizeof sp-sample-t)) out-ir))
      (sp-convolve-one lp-ir lp-len hp-ir hp-len *out-ir)))
  (label exit status-return))

(define (sp-windowed-sinc-lp-hp-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  "maps arguments from the generic ir-f-arguments array.
  arguments is (sp-float-t:cutoff sp-float-t:transition boolean:is-high-pass)"
  (declare cutoff sp-float-t transition sp-float-t is-high-pass boolean)
  (set
    cutoff (pointer-get (convert-type arguments sp-float-t*))
    transition (pointer-get (+ 1 (convert-type arguments sp-float-t*)))
    is-high-pass (pointer-get (convert-type (+ 2 (convert-type arguments sp-float-t*)) boolean*)))
  (return (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)))

(define (sp-windowed-sinc-bp-br-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  "maps arguments from the generic ir-f-arguments array.
  arguments is (sp-float-t:cutoff-l sp-float-t:cutoff-h sp-float-t:transition boolean:is-reject)"
  (declare
    cutoff-l sp-float-t
    cutoff-h sp-float-t
    transition-l sp-float-t
    transition-h sp-float-t
    is-reject boolean)
  (set
    cutoff-l (pointer-get (convert-type arguments sp-float-t*))
    cutoff-h (pointer-get (+ 1 (convert-type arguments sp-float-t*)))
    transition-l (pointer-get (+ 2 (convert-type arguments sp-float-t*)))
    transition-h (pointer-get (+ 3 (convert-type arguments sp-float-t*)))
    is-reject (pointer-get (convert-type (+ 4 (convert-type arguments sp-float-t*)) boolean*)))
  (return
    (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)))

(define (sp-windowed-sinc-lp-hp in in-len cutoff transition is-high-pass out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  "a windowed sinc low-pass or high-pass filter for segments of continuous streams with
  variable sample-rate, frequency, transition and impulse response type per call.
  * cutoff: as a fraction of the sample rate, 0..0.5
  * transition: like cutoff
  * is-high-pass: if true then it will reduce low frequencies
  * out-state: if zero then state will be allocated.
  * out-samples: owned by the caller. length must be at least in-len"
  status-declare
  (declare a (array uint8-t ((+ (sizeof boolean) (* 2 (sizeof sp-float-t))))) a-len uint8-t)
  (sc-comment "set arguments array for ir-f")
  (set
    a-len (+ (sizeof boolean) (* 2 (sizeof sp-float-t)))
    (pointer-get (convert-type a sp-float-t*)) cutoff
    (pointer-get (+ 1 (convert-type a sp-float-t*))) transition
    (pointer-get (convert-type (+ 2 (convert-type a sp-float-t*)) boolean*)) is-high-pass)
  (sc-comment "apply filter")
  (status-require (sp-convolution-filter in in-len sp-windowed-sinc-lp-hp-ir-f a a-len out-state out-samples))
  (label exit status-return))

(define
  (sp-windowed-sinc-bp-br in in-len
    cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-convolution-filter-state-t** sp-sample-t*)
  "like sp-windowed-sinc-lp-hp but for a band-pass or band-reject filter"
  status-declare
  (declare a (array uint8-t ((+ (sizeof boolean) (* 3 (sizeof sp-float-t))))) a-len uint8-t)
  (sc-comment "set arguments array for ir-f")
  (set
    a-len (+ (sizeof boolean) (* 4 (sizeof sp-float-t)))
    (pointer-get (convert-type a sp-float-t*)) cutoff-l
    (pointer-get (+ 1 (convert-type a sp-float-t*))) cutoff-h
    (pointer-get (+ 2 (convert-type a sp-float-t*))) transition-l
    (pointer-get (+ 3 (convert-type a sp-float-t*))) transition-h
    (pointer-get (convert-type (+ 4 (convert-type a sp-float-t*)) boolean*)) is-reject)
  (sc-comment "apply filter")
  (status-require (sp-convolution-filter in in-len sp-windowed-sinc-bp-br-ir-f a a-len out-state out-samples))
  (label exit status-return))

(pre-define (define-sp-state-variable-filter suffix transfer)
  (begin
    "samples real real pair [integer integer integer] -> state
    define a routine for a fast filter that also supports multiple filter types in one.
    state must hold two elements and is to be allocated and owned by the caller.
    cutoff is as a fraction of the sample rate between 0 and 0.5.
    uses the state-variable filter described here:
    * http://www.cytomic.com/technical-papers
    * http://www.earlevel.com/main/2016/02/21/filters-for-synths-starting-out/"
    (define ((pre-concat sp-state-variable-filter_ suffix) out in in-count cutoff q-factor state)
      (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-time-t sp-sample-t*)
      (declare
        a1 sp-sample-t
        a2 sp-sample-t
        g sp-sample-t
        ic1eq sp-sample-t
        ic2eq sp-sample-t
        i sp-time-t
        k sp-sample-t
        v0 sp-sample-t
        v1 sp-sample-t
        v2 sp-sample-t)
      (set
        ic1eq (array-get state 0)
        ic2eq (array-get state 1)
        g (tan (* M_PI cutoff))
        k (- 2 (* 2 q-factor))
        a1 (/ 1 (+ 1 (* g (+ g k))))
        a2 (* g a1))
      (for ((set i 0) (< i in-count) (set i (+ 1 i)))
        (set
          v0 (array-get in i)
          v1 (+ (* a1 ic1eq) (* a2 (- v0 ic2eq)))
          v2 (+ ic2eq (* g v1))
          ic1eq (- (* 2 v1) ic1eq)
          ic2eq (- (* 2 v2) ic2eq)
          (array-get out i) transfer))
      (set (array-get state 0) ic1eq (array-get state 1) ic2eq))))

(define-sp-state-variable-filter lp v2)
(define-sp-state-variable-filter hp (- v0 (* k v1) v2))
(define-sp-state-variable-filter bp v1)
(define-sp-state-variable-filter br (- v0 (* k v1)))
(define-sp-state-variable-filter peak (+ (- (* 2 v2) v0) (* k v1)))
(define-sp-state-variable-filter all (- v0 (* 2 k v1)))

(define
  (sp-filter in in-size cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-float-t sp-float-t sp-float-t sp-float-t boolean sp-filter-state-t** sp-sample-t*)
  "the sph-sp default precise filter. processing intensive if parameters are change frequently.
  memory for out-state will be allocated and has to be freed with sp-filter-state-free"
  (sp-windowed-sinc-bp-br in in-size
    cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples))

(define (sp-cheap-filter-state-new max-size max-passes out-state)
  (status-t sp-time-t sp-time-t sp-cheap-filter-state-t*)
  (sc-comment "allocates and prepares memory and checks if max-passes doesnt exceed the limit"
    "heap memory is to be freed with sp-cheap-filter-state-free but only allocated if max-passes is greater than one")
  status-declare
  (define in-temp sp-sample-t* 0)
  (define out-temp sp-sample-t* 0)
  (if (< 1 max-passes)
    (if (< sp-cheap-filter-passes-limit max-passes)
      (status-set-goto sp-s-group-sp sp-s-id-not-implemented)
      (begin
        (status-require (sp-sample-array-new max-size &in-temp))
        (status-require (sp-sample-array-new max-size &out-temp)))))
  (set out-state:in-temp in-temp out-state:out-temp out-temp)
  (memset out-state:svf-state 0 (* (sizeof sp-sample-t) 2 sp-cheap-filter-passes-limit))
  (label exit (if status-is-failure (free in-temp)) status-return))

(define (sp-cheap-filter-state-free a) (void sp-cheap-filter-state-t*)
  (free a:in-temp)
  (free a:out-temp))

(define (sp-cheap-filter type in in-size cutoff passes q-factor unity-gain state out)
  (void sp-state-variable-filter-t sp-sample-t*
    sp-time-t sp-float-t sp-time-t sp-float-t uint8-t sp-cheap-filter-state-t* sp-sample-t*)
  "the sph-sp default fast filter. caller has to manage the state object with sp-cheap-filter-state-new sp-cheap-filter-state-free"
  status-declare
  (declare in-swap sp-sample-t* in-temp sp-sample-t* out-temp sp-sample-t*)
  (if (= 1 passes) (type out in in-size cutoff q-factor state:svf-state)
    (begin
      (type state:in-temp in in-size cutoff q-factor state:svf-state)
      (set passes (- passes 1) in-temp state:in-temp out-temp state:out-temp)
      (label loop
        (if (< 1 passes)
          (begin
            (type out-temp in-temp in-size cutoff q-factor (+ passes state:svf-state))
            (sp-sample-array-zero in-temp in-size)
            (set passes (- passes 1) in-swap in-temp in-temp out-temp out-temp in-swap)
            (goto loop))
          (type out in-temp in-size cutoff q-factor (+ passes state:svf-state))))))
  (sc-comment "reset unused state values")
  (if (> sp-cheap-filter-passes-limit passes)
    (memset state:svf-state 0 (* (sizeof sp-sample-t) 2 (- sp-cheap-filter-passes-limit passes))))
  (if unity-gain (sp-set-unity-gain in in-size out)))