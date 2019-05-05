(pre-include
  "stdio.h"
  "fcntl.h"
  "sndfile.h"
  "foreign/nayuki-fft/fft.c"
  "../main/sph-sp.h"
  "../foreign/sph/float.c"
  "../foreign/sph/helper.c"
  "../foreign/sph/memreg.c"
  "../foreign/sph/quicksort.c"
  "../foreign/sph/queue.c" "../foreign/sph/thread-pool.c" "../foreign/sph/futures.c")

(pre-define
  sp-status-declare (status-declare-group sp-status-group-sp)
  (sp-libc-status-require-id id) (if (< id 0) (status-set-both-goto sp-status-group-libc id))
  (sp-libc-status-require expression)
  (begin
    (set status.id expression)
    (if (< status.id 0) (status-set-group-goto sp-status-group-libc)
      status-reset))
  (define-sp-interleave name type body)
  (begin
    "define a deinterleave, interleave or similar routine.
    a: source
    b: target"
    (define (name a b a-size channel-count) (void type** type* sp-count-t sp-channel-count-t)
      (declare
        b-size sp-count-t
        channel sp-channel-count-t)
      (set b-size (* a-size channel-count))
      (while a-size
        (set
          a-size (- a-size 1)
          channel channel-count)
        (while channel
          (set
            channel (- channel 1)
            b-size (- b-size 1))
          body)))))

(define-sp-interleave
  sp-interleave
  sp-sample-t
  (compound-statement (set (array-get b b-size) (array-get (array-get a channel) a-size))))

(define-sp-interleave
  sp-deinterleave
  sp-sample-t
  (compound-statement (set (array-get (array-get a channel) a-size) (array-get b b-size))))

(define (debug-display-sample-array a len) (void sp-sample-t* sp-count-t)
  "display a sample array in one line"
  (declare i sp-count-t)
  (printf "%.17g" (array-get a 0))
  (for ((set i 1) (< i len) (set i (+ 1 i)))
    (printf " %.17g" (array-get a i)))
  (printf "\n"))

(define (sp-status-description a) (uint8-t* status-t)
  "get a string description for a status id in a status-t"
  (declare b char*)
  (cond
    ( (not (strcmp sp-status-group-sp a.group))
      (case = a.id
        (sp-status-id-eof (set b "end of file"))
        (sp-status-id-input-type (set b "input argument is of wrong type"))
        (sp-status-id-not-implemented (set b "not implemented"))
        (sp-status-id-memory (set b "memory allocation error"))
        (sp-status-id-file-incompatible
          (set b "file channel count or sample rate is different from what was requested"))
        (sp-status-id-file-incomplete (set b "incomplete write"))
        (else (set b ""))))
    ( (not (strcmp sp-status-group-sndfile a.group))
      (set b (convert-type (sf-error-number a.id) char*)))
    ((not (strcmp sp-status-group-sph a.group)) (set b (sph-helper-status-description a)))
    (else (set b "")))
  (return b))

(define (sp-status-name a) (uint8-t* status-t)
  "get a single word identifier for a status id in a status-t"
  (declare b char*)
  (cond
    ( (= 0 (strcmp sp-status-group-sp a.group))
      (case = a.id
        (sp-status-id-input-type (set b "input-type"))
        (sp-status-id-not-implemented (set b "not-implemented"))
        (sp-status-id-memory (set b "memory"))
        (else (set b "unknown"))))
    ((= 0 (strcmp sp-status-group-sndfile a.group)) (set b "sndfile")) (else (set b "unknown")))
  (return b))

(define (sp-block-free a) (void sp-block-t)
  (declare i sp-count-t)
  (for ((set i 0) (< i a.channels) (set i (+ 1 i)))
    (free (array-get a.samples i))))

(define (sp-block-new channels size out) (status-t sp-channel-count-t sp-count-t sp-block-t*)
  "return a newly allocated array for channels with data arrays for each channel"
  status-declare
  (memreg-init channels)
  (declare
    channel sp-sample-t*
    i sp-count-t)
  (for ((set i 0) (< i channels) (set i (+ 1 i)))
    (status-require (sph-helper-calloc (* size (sizeof sp-sample-t)) &channel))
    (memreg-add channel)
    (set (array-get out:samples i) channel))
  (set
    out:size size
    out:channels channels)
  (label exit
    (if status-is-failure memreg-free)
    (return status)))

(define (sp-samples-new size out) (status-t sp-count-t sp-sample-t**)
  (return (sph-helper-calloc (* size (sizeof sp-sample-t)) out)))

(define (sp-counts-new size out) (status-t sp-count-t sp-count-t**)
  (return (sph-helper-calloc (* size (sizeof sp-count-t)) out)))

(define (sp-sin-lq a) (sp-sample-t sp-float-t)
  "lower precision version of sin() that should be faster"
  (declare
    b sp-sample-t
    c sp-sample-t)
  (set
    b (/ 4 M_PI)
    c (/ -4 (* M_PI M_PI)))
  (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-sinc a) (sp-float-t sp-float-t)
  "the normalised sinc function"
  (return
    (if* (= 0 a) 1
      (/ (sin (* M_PI a)) (* M_PI a)))))

(define (sp-fft input-len input/output-real input/output-imag)
  (status-id-t sp-count-t double* double*)
  "all arrays should be input-len and are managed by the caller"
  (return (not (Fft_transform input/output-real input/output-imag input-len))))

(define (sp-ffti input-len input/output-real input/output-imag)
  (status-id-t sp-count-t double* double*)
  "[[real, imaginary], ...]:complex-numbers -> real-numbers
  input-length > 0
  output-length = input-length
  output is allocated and owned by the caller"
  (return (not (= 1 (Fft_inverseTransform input/output-real input/output-imag input-len)))))

(pre-define
  (max a b)
  (if* (> a b) a
    b)
  (min a b)
  (if* (< a b) a
    b))

(define
  (sp-moving-average in in-end in-window in-window-end prev prev-end next next-end radius out)
  (status-t
    sp-sample-t*
    sp-sample-t*
    sp-sample-t*
    sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-count-t sp-sample-t*)
  "apply a centered moving average filter to samples between in-window and in-window-end inclusively and write to out.
   removes high frequencies and smoothes data with little distortion in the time domain but the frequency response has large ripples.
   all memory is managed by the caller.
   * prev and next can be null pointers if not available
   * zero is used for unavailable values
   * rounding errors are kept low by using modified kahan neumaier summation"
  status-declare
  (declare
    in-left sp-sample-t*
    in-right sp-sample-t*
    outside sp-sample-t*
    sums (array sp-sample-t (3))
    outside-count sp-count-t
    in-missing sp-count-t
    width sp-count-t)
  (set width (+ 1 radius radius))
  (while (<= in-window in-window-end)
    (set
      (array-get sums 0) 0
      (array-get sums 1) 0
      (array-get sums 2) 0
      in-left (max in (- in-window radius))
      in-right (min in-end (+ in-window radius))
      (array-get sums 1) (sp-sample-sum in-left (+ 1 (- in-right in-left))))
    ;(printf "current center: %f\n" (pointer-get in-window))
    ;(printf "in: ")
    ;(debug-display-sample-array in-left (+ 1 (- in-right in-left)))
    (if (and (< (- in-window in-left) radius) prev)
      (begin
        (set
          in-missing (- radius (- in-window in-left))
          outside (max prev (- prev-end in-missing))
          outside-count (- prev-end outside)
          (array-get sums 0) (sp-sample-sum outside outside-count))
        ;(printf "prev: ")
        ;(debug-display-sample-array outside outside-count)
        ))
    (if (and (< (- in-right in-window) radius) next)
      (begin
        (set
          in-missing (- radius (- in-right in-window))
          outside next
          outside-count (min (- next-end next) in-missing)
          (array-get sums 2) (sp-sample-sum outside outside-count))
        ;(printf "next: ")
        ;(debug-display-sample-array outside outside-count)
        ))
    (set
      (pointer-get out) (/ (sp-sample-sum sums 3) width)
      out (+ 1 out)
      in-window (+ 1 in-window)))
  (return status))

(define (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-count-t)
  "modify an impulse response kernel for spectral inversion.
   a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response top to bottom"
  (declare
    center sp-count-t
    i sp-count-t)
  (for ((set i 0) (< i a-len) (set i (+ 1 i)))
    (set (array-get a i) (* -1 (array-get a i))))
  (set
    center (/ (- a-len 1) 2)
    (array-get a center) (+ 1 (array-get a center))))

(define (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-count-t)
  "inverts the sign for samples at odd indexes.
  a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response left to right"
  (while (> a-len 1)
    (set
      a-len (- a-len 2)
      (array-get a a-len) (* -1 (array-get a a-len)))))

(define (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-count-t sp-sample-t* sp-count-t sp-sample-t*)
  "discrete linear convolution.
  result-samples must be all zeros, its length must be at least a-len + b-len - 1.
  result-samples is owned and allocated by the caller"
  (declare
    a-index sp-count-t
    b-index sp-count-t)
  (set
    a-index 0
    b-index 0)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set
        (array-get result-samples (+ a-index b-index))
        (+
          (array-get result-samples (+ a-index b-index))
          (* (array-get a a-index) (array-get b b-index)))
        b-index (+ 1 b-index)))
    (set
      b-index 0
      a-index (+ 1 a-index))))

(define (sp-convolve a a-len b b-len carryover-len result-carryover result-samples)
  (void sp-sample-t* sp-count-t sp-sample-t* sp-count-t sp-count-t sp-sample-t* sp-sample-t*)
  "discrete linear convolution for sample arrays, possibly of a continuous stream. maps segments (a, a-len) to result-samples
  using (b, b-len) as the impulse response. b-len must be greater than zero.
  all heap memory is owned and allocated by the caller.
  result-samples length is a-len.
  result-carryover is previous carryover or an empty array.
  result-carryover length must at least b-len - 1.
  carryover-len should be zero for the first call or its content should be zeros.
  carryover-len for subsequent calls should be b-len - 1 or if b-len changed b-len - 1  from the previous call.
  if b-len is one then there is no carryover.
  if a-len is smaller than b-len then, with the current implementation, additional performance costs ensue from shifting the carryover array each call.
  carryover is the extension of result-samples for generated values that dont fit"
  (declare
    size sp-count-t
    a-index sp-count-t
    b-index sp-count-t
    c-index sp-count-t)
  (if carryover-len
    (if (<= carryover-len a-len)
      (begin
        (sc-comment "copy all entries to result and reset")
        (memcpy result-samples result-carryover (* carryover-len (sizeof sp-sample-t)))
        (memset result-carryover 0 (* carryover-len (sizeof sp-sample-t)))
        (memset (+ carryover-len result-samples) 0 (* (- a-len carryover-len) (sizeof sp-sample-t))))
      (begin
        (sc-comment
          "carryover is larger. set result-samples to all carryover entries that fit."
          "shift remaining carryover to the left")
        (memcpy result-samples result-carryover (* a-len (sizeof sp-sample-t)))
        (memmove
          result-carryover
          (+ a-len result-carryover) (* (- carryover-len a-len) (sizeof sp-sample-t)))
        (memset (+ (- carryover-len a-len) result-carryover) 0 (* a-len (sizeof sp-sample-t)))))
    (memset result-samples 0 (* a-len (sizeof sp-sample-t))))
  (sc-comment "result values." "first process values that dont lead to carryover")
  (set size
    (if* (< a-len b-len) 0
      (- a-len (- b-len 1))))
  (if size (sp-convolve-one a size b b-len result-samples))
  (sc-comment "process values with carryover")
  (for ((set a-index size) (< a-index a-len) (set a-index (+ 1 a-index)))
    (for ((set b-index 0) (< b-index b-len) (set b-index (+ 1 b-index)))
      (set c-index (+ a-index b-index))
      (if (< c-index a-len)
        (set (array-get result-samples c-index)
          (+ (array-get result-samples c-index) (* (array-get a a-index) (array-get b b-index))))
        (set
          c-index (- c-index a-len)
          (array-get result-carryover c-index)
          (+ (array-get result-carryover c-index) (* (array-get a a-index) (array-get b b-index))))))))

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
    carryover-alloc-len sp-count-t
    ir sp-sample-t*
    ir-len sp-count-t
    state sp-convolution-filter-state-t*)
  (memreg-init 2)
  (sc-comment
    "create state if not exists. re-use if exists and return early if ir needs not be updated")
  (if *out-state
    (begin
      (sc-comment "existing")
      (set state *out-state)
      (if
        (and
          (= state:ir-f ir-f)
          (= ir-f-arguments-len state:ir-f-arguments-len)
          (= 0 (memcmp state:ir-f-arguments ir-f-arguments ir-f-arguments-len)))
        (begin
          (sc-comment "unchanged")
          (return status))
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
                (sph-helper-realloc (* carryover-alloc-len (sizeof sp-sample-t)) &carryover))
              (set state:carryover-alloc-len carryover-alloc-len)))
          (sc-comment "in any case reset the extension area")
          (memset
            (+ (- state:ir-len 1) carryover) 0 (* (- ir-len state:ir-len) (sizeof sp-sample-t))))))
    (begin
      (status-require (sph-helper-calloc (* carryover-alloc-len (sizeof sp-sample-t)) &carryover))
      (set state:carryover-alloc-len carryover-alloc-len)))
  (set
    state:carryover carryover
    state:ir ir
    state:ir-len ir-len
    *out-state state)
  (label exit
    (if status-is-failure memreg-free)
    (return status)))

(define
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t
    sp-sample-t*
    sp-count-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  "convolute samples \"in\", which can be a segment of a continuous stream, with an impulse response
  kernel created by ir-f with ir-f-arguments.
  ir-f is only used when ir-f-arguments changed.
  values that need to be carried over with calls are kept in out-state.
  * out-state: if zero then state will be allocated. owned by caller
  * out-samples: owned by the caller. length must be at least in-len and the number of output samples will be in-len"
  status-declare
  (declare carryover-len sp-count-t)
  (set carryover-len
    (if* *out-state (- (: *out-state ir-len) 1)
      0))
  (sc-comment "create/update the impulse response kernel")
  (status-require
    (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state))
  (sc-comment "convolve")
  (sp-convolve
    in
    in-len (: *out-state ir) (: *out-state ir-len) carryover-len (: *out-state carryover) out-samples)
  (label exit
    (return status)))

(pre-define (define-sp-state-variable-filter suffix transfer)
  (begin
    "samples real real pair [integer integer integer] -> state
    define a routine for a fast filter that supports multiple filter types in one.
    state must hold two elements and is allocated and owned by the caller.
    cutoff is as a fraction of the sample rate between 0 and 0.5.
    uses the state-variable filter described here:
    * http://www.cytomic.com/technical-papers
    * http://www.earlevel.com/main/2016/02/21/filters-for-synths-starting-out/"
    (define ((pre-concat sp-state-variable-filter_ suffix) out in in-count cutoff q-factor state)
      (void sp-sample-t* sp-sample-t* sp-float-t sp-float-t sp-count-t sp-sample-t*)
      (declare
        a1 sp-sample-t
        a2 sp-sample-t
        g sp-sample-t
        ic1eq sp-sample-t
        ic2eq sp-sample-t
        i sp-count-t
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
      (set
        (array-get state 0) ic1eq
        (array-get state 1) ic2eq))))

(define-sp-state-variable-filter lp v2)
(define-sp-state-variable-filter hp (- v0 (* k v1) v2))
(define-sp-state-variable-filter bp v1)
(define-sp-state-variable-filter br (- v0 (* k v1)))
(define-sp-state-variable-filter peak (+ (- (* 2 v2) v0) (* k v1)))
(define-sp-state-variable-filter all (- v0 (* 2 k v1)))

(define (sp-sine-table-new out size) (status-t sp-sample-t** sp-count-t)
  "writes a sine wave of size into out. can be used as a lookup table"
  status-declare
  (declare
    i sp-count-t
    out-array sp-sample-t*)
  (status-require (sph-helper-malloc (* size (sizeof sp-sample-t*)) &out-array))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set (array-get out-array i) (sin (* i (/ M_PI (/ size 2))))))
  (set *out out-array)
  (label exit
    (return status)))

(define (sp-initialise cpu-count) (status-t uint16-t)
  "fills the sine wave lookup table"
  status-declare
  (set status.id (future-init cpu-count))
  (if status.id (return status))
  (return (sp-sine-table-new &sp-sine-96-table 96000)))

(define (sp-phase-96 current change) (sp-count-t sp-count-t sp-count-t)
  "accumulate an integer phase and reset it after cycles.
  float value phases would be harder to reset"
  (declare result sp-count-t)
  (set result (+ current change))
  (return
    (if* (<= 96000 result) (modulo result 96000)
      result)))

(define (sp-phase-96-float current change) (sp-count-t sp-count-t double)
  "accumulate an integer phase with change given as a float value.
  change must be a positive value and is rounded to the next larger integer"
  (return (sp-phase-96 current (sp-cheap-ceiling-positive change))))

(define (sp-synth-state-new channel-count config-len config out-state)
  (status-t sp-count-t sp-synth-count-t sp-synth-partial-t* sp-count-t**)
  "contains the initial phase offsets per partial and channel
  as a flat array"
  status-declare
  (declare
    i sp-count-t
    channel-i sp-count-t)
  (status-require (sph-helper-calloc (* channel-count config-len (sizeof sp-count-t)) out-state))
  (for ((set i 0) (< i config-len) (set i (+ 1 i)))
    (for ((set channel-i 0) (< channel-i channel-count) (set channel-i (+ 1 channel-i)))
      (set (array-get *out-state (+ channel-i (* channel-count i)))
        (array-get (struct-get (array-get config i) phs) channel-i))))
  (label exit
    (return status)))

(define (sp-synth out start duration config-len config phases)
  (status-t sp-block-t sp-count-t sp-count-t sp-synth-count-t sp-synth-partial-t* sp-count-t*)
  "create sines that start and end at specific times and can optionally modulate the frequency of others.
  sp-synth output is summed into out.
  amplitude and wavelength can be controlled by arrays separately for each partial and channel.
  modulators can be modulated themselves in chains. state has to be allocated by the caller with sp-synth-state-new.
  modulator amplitude is relative to carrier wavelength.
  paths are relative to the start of partials.
  # requirements
  * modulators must come after carriers in config
  * config-len must not change between calls with the same state
  * all amplitude/wavelength arrays must be of sufficient size and available for all channels
  * sp-initialise must have been called once before using sp-synth
  # algorithm
  * read config from the end to the start
  * write modulator output to temporary buffers that are indexed by modified partial id
  * apply modulator output from the buffers and sum to output for final carriers
  * each partial has integer phases that are reset in cycles and kept in state between calls"
  ; config is evaluated from last to first.
  ; modulation is duration length, paths are samples relative to the partial start
  status-declare
  (declare
    amp sp-sample-t
    carrier sp-sample-t*
    channel-i sp-count-t
    end sp-count-t
    i sp-count-t
    modulated-wvl sp-sample-t
    modulation-index (array sp-sample-t* (sp-synth-partial-limit sp-synth-channel-limit))
    modulation sp-sample-t*
    phs sp-count-t
    prt-duration sp-count-t
    prt-i sp-synth-count-t
    prt-offset-right sp-count-t
    prt-offset sp-count-t
    prt sp-synth-partial-t
    prt-start sp-count-t
    wvl sp-count-t)
  (sc-comment
    "modulation blocks (channel array + data. at least one is carrier and writes only to output)")
  (memreg-init (* (- config-len 1) out.channels))
  (memset modulation-index 0 (sizeof modulation-index))
  (set
    end (+ start duration)
    prt-i config-len)
  (while prt-i
    (set
      prt-i (- prt-i 1)
      prt (array-get config prt-i))
    (if (< end prt.start) break)
    (if (<= prt.end start) continue)
    (set
      prt-start
      (if* (< prt.start start) (- start prt.start)
        0)
      prt-offset
      (if* (> prt.start start) (- prt.start start)
        0)
      prt-offset-right
      (if* (> prt.end end) 0
        (- end prt.end))
      prt-duration (- duration prt-offset prt-offset-right))
    (if prt.modifies
      (for ((set channel-i 0) (< channel-i out.channels) (set channel-i (+ 1 channel-i)))
        (set carrier (array-get modulation-index (- prt.modifies 1) channel-i))
        (if (not carrier)
          (begin
            (status-require (sph-helper-calloc (* duration (sizeof sp-sample-t)) &carrier))
            (memreg-add carrier)))
        (set
          phs (array-get phases (+ channel-i (* out.channels prt-i)))
          modulation (array-get modulation-index prt-i channel-i))
        (for ((set i 0) (< i prt-duration) (set i (+ 1 i)))
          (set
            amp (array-get prt.amp channel-i (+ prt-start i))
            (array-get carrier (+ prt-offset i))
            (+ (array-get carrier (+ prt-offset i)) (* amp (sp-sine-96 phs))) wvl
            (array-get prt.wvl channel-i (+ prt-start i)) modulated-wvl
            (if* modulation (+ wvl (* wvl (array-get modulation (+ prt-offset i))))
              wvl)
            phs (sp-phase-96-float phs (/ 48000 modulated-wvl))))
        (set
          (array-get phases (+ channel-i (* out.channels prt-i))) phs
          (array-get modulation-index (- prt.modifies 1) channel-i) carrier))
      (for ((set channel-i 0) (< channel-i out.channels) (set channel-i (+ 1 channel-i)))
        (set
          phs (array-get phases (+ channel-i (* out.channels prt-i)))
          modulation (array-get modulation-index prt-i channel-i))
        (for ((set i 0) (< i prt-duration) (set i (+ 1 i)))
          (set
            amp (array-get prt.amp channel-i (+ prt-start i))
            wvl (array-get prt.wvl channel-i (+ prt-start i))
            modulated-wvl
            (if* modulation (+ wvl (* wvl (array-get modulation (+ prt-offset i))))
              wvl)
            phs (sp-phase-96-float phs (/ 48000 modulated-wvl))
            (array-get (array-get out.samples channel-i) (+ prt-offset i))
            (+
              (array-get (array-get out.samples channel-i) (+ prt-offset i)) (* amp (sp-sine-96 phs)))))
        (set (array-get phases (+ channel-i (* out.channels prt-i))) phs))))
  (label exit
    memreg-free
    (return status)))

(pre-include "../main/windowed-sinc.c" "../main/io.c")

(define (sp-event-sort-swap a b) (void void* void*)
  (declare c sp-event-t)
  (set
    c (pointer-get (convert-type a sp-event-t*))
    (pointer-get (convert-type b sp-event-t*)) (pointer-get (convert-type a sp-event-t*))
    (pointer-get (convert-type b sp-event-t*)) c))

(define (sp-event-sort-less? a b) (uint8-t void* void*)
  (return (< (: (convert-type a sp-event-t*) start) (: (convert-type b sp-event-t*) start))))

(define (sp-seq-events-prepare a size) (void sp-event-t* sp-count-t)
  (quicksort sp-event-sort-less? sp-event-sort-swap (sizeof sp-event-t) a size))

(define (sp-block-with-offset a offset) (sp-block-t sp-block-t sp-count-t)
  "add offset to the all channel sample arrays in block"
  (declare i sp-count-t)
  (for ((set i 0) (< i a.channels) (set i (+ 1 i)))
    (set (array-get a.samples i) (+ offset (array-get a.samples i))))
  (return a))

(define (sp-seq start end out out-start events events-size)
  (void sp-count-t sp-count-t sp-block-t sp-count-t sp-event-t* sp-count-t)
  "event arrays must have been prepared/sorted with sp-seq-event-prepare for seq to work correctly"
  (declare
    e-out-start sp-count-t
    e sp-event-t
    e-start sp-count-t
    e-end sp-count-t
    i sp-count-t)
  (if out-start (set out (sp-block-with-offset out out-start)))
  (for ((set i 0) (< i events-size) (set i (+ 1 i)))
    (set e (array-get events i))
    (cond
      ((<= e.end start) continue)
      ((<= end e.start) break)
      (else
        (set
          e-out-start
          (if* (> e.start start) (- e.start start)
            0)
          e-start
          (if* (> start e.start) (- start e.start)
            0)
          e-end
          (-
            (if* (< e.end end) e.end
              end)
            e.start))
        (e.f
          e-start e-end
          (if* e-out-start (sp-block-with-offset out e-out-start)
            out)
          &e)))))

(declare sp-seq-future-t
  (type
    (struct
      (start sp-count-t)
      (end sp-count-t)
      (out-start sp-count-t)
      (out sp-block-t)
      (event sp-event-t*)
      (future future-t))))

(define (sp-seq-parallel-future-f data) (void* void*)
  (define a sp-seq-future-t* data)
  (a:event:f a:start a:end a:out a:event)
  (return data))

(define (sp-seq-parallel start end out out-start events events-size)
  (status-t sp-count-t sp-count-t sp-block-t sp-count-t sp-event-t* sp-count-t)
  "like sp-seq but evaluates events in parallel"
  status-declare
  (declare
    e-out-start sp-count-t
    e sp-event-t
    e-start sp-count-t
    e-end sp-count-t
    channel-i sp-channel-count-t
    events-start sp-count-t
    events-count sp-count-t
    seq-futures sp-seq-future-t*
    sf sp-seq-future-t*
    i sp-count-t
    e-i sp-count-t)
  (set seq-futures 0)
  (if out-start (set out (sp-block-with-offset out out-start)))
  (sc-comment "select active events")
  (for
    ( (set
        i 0
        events-start 0
        events-count 0)
      (< i events-size) (set i (+ 1 i)))
    (set e (array-get events i))
    (cond
      ((<= e.end start) (set events-start (+ 1 events-start)))
      ((<= end e.start) break) (else (set events-count (+ 1 events-count)))))
  (status-require (sph-helper-malloc (* events-count (sizeof sp-seq-future-t)) &seq-futures))
  (sc-comment "parallelise")
  (for ((set i 0) (< i events-count) (set i (+ 1 i)))
    (set
      e (array-get events (+ events-start i))
      sf (+ i seq-futures))
    (set
      e-out-start
      (if* (> e.start start) (- e.start start)
        0)
      e-start
      (if* (> start e.start) (- start e.start)
        0)
      e-end
      (-
        (if* (< e.end end) e.end
          end)
        e.start))
    (status-require (sp-block-new out.channels (- e-end e-start) &sf:out))
    (set
      sf:start e-start
      sf:end e-end
      sf:out-start e-out-start
      sf:event (+ events-start i events))
    (future-new sp-seq-parallel-future-f sf &sf:future))
  (sc-comment "merge")
  (for ((set e-i 0) (< e-i events-count) (set e-i (+ 1 e-i)))
    (set sf (+ e-i seq-futures))
    (touch &sf:future)
    (for ((set channel-i 0) (< channel-i out.channels) (set channel-i (+ 1 channel-i)))
      (for ((set i 0) (< i sf:out.size) (set i (+ 1 i)))
        (set (array-get (array-get out.samples channel-i) (+ sf:out-start i))
          (+
            (array-get (array-get out.samples channel-i) (+ sf:out-start i))
            (array-get (array-get sf:out.samples channel-i) i)))))
    (sp-block-free sf:out))
  (label exit
    (free seq-futures)
    (return status)))

(define (sp-synth-event-f start end out event) (void sp-count-t sp-count-t sp-block-t sp-event-t*)
  (define s sp-synth-event-state-t* event:state)
  (sp-synth out start (- end start) s:config-len s:config s:state))

(define (sp-synth-event start end channel-count config-len config out-event)
  (status-t sp-count-t sp-count-t sp-count-t sp-count-t sp-synth-partial-t* sp-event-t*)
  "memory for event.state will be allocated and then owned by the caller.
  config is copied into event.state"
  status-declare
  (declare
    e sp-event-t
    state sp-synth-event-state-t*)
  (status-require (sph-helper-malloc (* channel-count (sizeof sp-synth-event-state-t)) &state))
  (status-require (sp-synth-state-new channel-count config-len config &state:state))
  (memcpy state:config config (* config-len (sizeof sp-synth-partial-t)))
  (set
    state:config-len config-len
    e.start start
    e.end end
    e.f sp-synth-event-f
    e.state state)
  (set *out-event e)
  (label exit
    (return status)))

(pre-define (sp-synth-partial-set-channel prt channel amp-array wvl-array phs-array)
  (set
    (array-get prt.amp channel) amp-array
    (array-get prt.wvl channel) wvl-array
    (array-get prt.phs channel) phs-array))

(define (sp-synth-partial-1 start end modifies amp wvl phs)
  (sp-synth-partial-t sp-count-t sp-count-t sp-synth-count-t sp-sample-t* sp-count-t* sp-count-t)
  "setup a synth partial with one channel"
  (declare prt sp-synth-partial-t)
  (set
    prt.start start
    prt.end end
    prt.modifies modifies)
  (sp-synth-partial-set-channel prt 0 amp wvl phs)
  (return prt))

(define (sp-synth-partial-2 start end modifies amp1 amp2 wvl1 wvl2 phs1 phs2)
  (sp-synth-partial-t
    sp-count-t
    sp-count-t
    sp-synth-count-t sp-sample-t* sp-sample-t* sp-count-t* sp-count-t* sp-count-t sp-count-t)
  "setup a synth partial with two channels"
  (declare prt sp-synth-partial-t)
  (set
    prt.start start
    prt.end end
    prt.modifies modifies)
  (sp-synth-partial-set-channel prt 0 amp1 wvl1 phs1)
  (sp-synth-partial-set-channel prt 1 amp2 wvl2 phs2)
  (return prt))

(define sp-plot-temp-file-index uint32-t 0)

(pre-define
  sp-plot-temp-path "/tmp/sp-plot"
  sp-plot-temp-file-index-maxlength 10
  sp-plot-command-pattern-lines
  "gnuplot --persist -e 'set key off; set size ratio 0.5; plot \"%s\" with lines lc rgb \"blue\"'"
  sp-plot-command-pattern-steps
  "gnuplot --persist -e 'set key off; set size ratio 0.5; plot \"%s\" with histeps lc rgb \"blue\"'")

(define (sp-plot-samples->file a a-size path) (void sp-sample-t* sp-count-t uint8-t*)
  (declare
    file FILE*
    i sp-count-t)
  (set file (fopen path "w"))
  (for ((set i 0) (< i a-size) (set i (+ 1 i)))
    (fprintf file "%.3f\n" (array-get a i)))
  (fclose file))

(define (sp-plot-counts->file a a-size path) (void sp-count-t* sp-count-t uint8-t*)
  (declare
    file FILE*
    i sp-count-t)
  (set file (fopen path "w"))
  (for ((set i 0) (< i a-size) (set i (+ 1 i)))
    (fprintf file "%lu\n" (array-get a i)))
  (fclose file))

(define (sp-plot-samples-file path use-steps) (void uint8-t* uint8-t)
  (declare
    command uint8-t*
    command-pattern uint8-t*
    command-size size-t)
  (set
    command-pattern
    (if* use-steps sp-plot-command-pattern-steps
      sp-plot-command-pattern-lines)
    command-size (+ (strlen path) (strlen command-pattern))
    command (malloc command-size))
  (if (not command) return)
  (snprintf command command-size command-pattern path)
  (system command)
  (free command))

(define (sp-plot-samples a a-size) (void sp-sample-t* sp-count-t)
  (define path-size uint8-t (+ 1 sp-plot-temp-file-index-maxlength (strlen sp-plot-temp-path)))
  (define path uint8-t* (calloc path-size 1))
  (if (not path) return)
  (snprintf path path-size "%s-%lu" sp-plot-temp-path sp-plot-temp-file-index)
  (set sp-plot-temp-file-index (+ 1 sp-plot-temp-file-index))
  (sp-plot-samples->file a a-size path)
  (sp-plot-samples-file path #t)
  (free path))

(define (sp-plot-counts a a-size) (void sp-count-t* sp-count-t)
  (define path-size uint8-t (+ 1 sp-plot-temp-file-index-maxlength (strlen sp-plot-temp-path)))
  (define path uint8-t* (calloc path-size 1))
  (if (not path) return)
  (snprintf path path-size "%s-%lu" sp-plot-temp-path sp-plot-temp-file-index)
  (set sp-plot-temp-file-index (+ 1 sp-plot-temp-file-index))
  (sp-plot-counts->file a a-size path)
  (sp-plot-samples-file path #t)
  (free path))

(define (sp-plot-spectrum->file a a-size path) (void sp-sample-t* sp-count-t uint8-t*)
  "take the fft for given samples, convert complex values to magnitudes and write plot data to file"
  (declare
    file FILE*
    i sp-count-t
    imag double*
    real double*)
  (set imag (calloc a-size (sizeof sp-sample-t)))
  (if (not imag) return)
  (set real (malloc (* (sizeof sp-sample-t) a-size)))
  (if (not real) return)
  (memcpy real a (* (sizeof sp-sample-t) a-size))
  (if (sp-fft a-size real imag) return)
  (set file (fopen path "w"))
  (for ((set i 0) (< i a-size) (set i (+ 1 i)))
    (fprintf
      file
      "%.3f\n"
      (*
        2
        (/
          (sqrt
            (+ (* (array-get real i) (array-get real i)) (* (array-get imag i) (array-get imag i))))
          a-size))))
  (fclose file)
  (free imag)
  (free real))

(define (sp-plot-spectrum-file path) (void uint8-t*) (sp-plot-samples-file path #t))

(define (sp-plot-spectrum a a-size) (void sp-sample-t* sp-count-t)
  (define path-size uint8-t (+ 1 sp-plot-temp-file-index-maxlength (strlen sp-plot-temp-path)))
  (define path uint8-t* (calloc path-size 1))
  (if (not path) return)
  (snprintf path path-size "%s-%lu" sp-plot-temp-path sp-plot-temp-file-index)
  (set sp-plot-temp-file-index (+ 1 sp-plot-temp-file-index))
  (sp-plot-spectrum->file a a-size path)
  (sp-plot-spectrum-file path)
  (free path))

(define (sp-triangle t a b) (sp-sample-t sp-count-t sp-count-t sp-count-t)
  "return a sample for a triangular wave with center offsets a left and b right.
   creates sawtooth waves if either a or b is 0"
  (declare remainder sp-count-t)
  (set remainder (modulo t (+ a b)))
  (return
    (if* (< remainder a) (* remainder (/ 1 (convert-type a sp-sample-t)))
      (*
        (- (convert-type b sp-sample-t) (- remainder (convert-type a sp-sample-t)))
        (/ 1 (convert-type b sp-sample-t))))))

(define (sp-triangle-96 t) (sp-sample-t sp-count-t) (return (sp-triangle t 48000 48000)))

(define (sp-square-96 t) (sp-sample-t sp-count-t)
  (return
    (if* (< (modulo (* 2 t) (* 2 96000)) 96000) -1
      1)))

(pre-define
  (double-from-uint64 a)
  (begin
    "guarantees that all dyadic rationals of the form (k / 2**−53) will be equally likely. this conversion prefers the high bits of x.
    from http://xoshiro.di.unimi.it/"
    (* (bit-shift-right a 11) (/ 1.0 (bit-shift-left (UINT64_C 1) 53))))
  (rotl x k) (bit-or (bit-shift-left x k) (bit-shift-right x (- 64 k))))

(define (sp-random-state-new seed) (sp-random-state-t uint64-t)
  "use the given uint64 as a seed and set state with splitmix64 results.
  the same seed will lead to the same series of random numbers from sp-random"
  (declare
    i uint8-t
    z uint64-t
    result sp-random-state-t)
  (for ((set i 0) (< i 4) (set i (+ 1 i)))
    (set
      seed (+ seed (UINT64_C 11400714819323198485))
      z seed
      z (* (bit-xor z (bit-shift-right z 30)) (UINT64_C 13787848793156543929))
      z (* (bit-xor z (bit-shift-right z 27)) (UINT64_C 10723151780598845931))
      (array-get result.data i) (bit-xor z (bit-shift-right z 31))))
  (return result))

(define (sp-random state size out) (sp-random-state-t sp-random-state-t sp-count-t sp-sample-t*)
  "return uniformly distributed random real numbers in the range -1 to 1.
   implements xoshiro256plus from http://xoshiro.di.unimi.it/
   referenced by https://nullprogram.com/blog/2017/09/21/"
  (declare
    result-plus uint64-t
    i sp-count-t
    t uint64-t)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set
      result-plus (+ (array-get state.data 0) (array-get state.data 3))
      t (bit-shift-left (array-get state.data 1) 17)
      (array-get state.data 2) (bit-xor (array-get state.data 2) (array-get state.data 0))
      (array-get state.data 3) (bit-xor (array-get state.data 3) (array-get state.data 1))
      (array-get state.data 1) (bit-xor (array-get state.data 1) (array-get state.data 2))
      (array-get state.data 0) (bit-xor (array-get state.data 0) (array-get state.data 3))
      (array-get state.data 2) (bit-xor (array-get state.data 2) t)
      (array-get state.data 3) (rotl (array-get state.data 3) 45)
      (array-get out i) (- (* 2 (double-from-uint64 result-plus)) 1.0)))
  (return state))