(pre-include
  "stdio.h"
  "fcntl.h"
  "sndfile.h"
  "../main/sph-sp.h"
  "../foreign/sph/float.c"
  "../foreign/sph/helper.c" "../foreign/sph/memreg.c" "foreign/nayuki-fft/fft.c")

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
    (define (name a b a-size channel-count)
      (void type** type* sp-sample-count-t sp-channel-count-t)
      (declare
        b-size sp-sample-count-t
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

(define (debug-display-sample-array a len) (void sp-sample-t* sp-sample-count-t)
  "display a sample array in one line"
  (declare i sp-sample-count-t)
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

(define (sp-channel-data-free a channel-count) (void sp-sample-t** sp-channel-count-t)
  (while channel-count
    (set channel-count (- channel-count 1))
    (free (array-get a channel-count)))
  (free a))

(define (sp-alloc-channel-array channel-count sample-count result-array)
  (status-t sp-channel-count-t sp-sample-count-t sp-sample-t***)
  "return a newly allocated array for channels with data arrays for each channel.
  returns zero if memory could not be allocated"
  status-declare
  (memreg-init (+ channel-count 1))
  (declare
    channel sp-sample-t*
    result sp-sample-t**)
  (status-require (sph-helper-malloc (* channel-count (sizeof sp-sample-t*)) &result))
  (memreg-add result)
  (while channel-count
    (set channel-count (- channel-count 1))
    (status-require (sph-helper-calloc (* sample-count (sizeof sp-sample-t)) &channel))
    (memreg-add channel)
    (set (array-get result channel-count) channel))
  (set *result-array result)
  (label exit
    (if status-is-failure memreg-free)
    (return status)))

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

(define (sp-fft input-len input-real input-imag output-real output-imag)
  (status-t sp-sample-count-t sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t*)
  "all arrays should be input-len and are managed by the caller"
  sp-status-declare
  (memcpy output-real input-real input-len)
  (memcpy output-imag input-imag input-len)
  (status-id-require (not (Fft_transform output-real output-imag input-len)))
  (label exit
    (return status)))

(define (sp-ffti input-len input-real input-imag output-real output-imag)
  (status-t sp-sample-count-t sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t*)
  "[[real, imaginary], ...]:complex-numbers -> real-numbers
  input-length > 0
  output-length = input-length
  output is allocated and owned by the caller"
  sp-status-declare
  (memcpy output-real input-real input-len)
  (memcpy output-imag input-imag input-len)
  (status-id-require (not (= 1 (Fft_inverseTransform output-real output-imag input-len))))
  (label exit
    (return status)))

(define
  (sp-moving-average source source-len prev prev-len next next-len radius start end result-samples)
  (status-t
    sp-sample-t*
    sp-sample-count-t
    sp-sample-t*
    sp-sample-count-t
    sp-sample-t* sp-sample-count-t sp-sample-count-t sp-sample-count-t sp-sample-count-t sp-sample-t*)
  "apply a centered moving average filter to source at index start to end inclusively and write to result.
  removes higher frequencies with little distortion in the time domain.
  result-samples is owned and allocated by the caller.
   * only the result portion corresponding to the subvector from start to end is written to result
   * prev and next are unprocessed segments and can be null pointers,
     for example at the beginning and end of a stream
   * since the result value for a sample is calculated using samples left and right of it,
     a previous and following part of a stream is eventually needed to reference values
     outside the source segment to create an accurate continuous result.
     zero is used for unavailable values outside the source segment
   * available values outside the start/end range are still considered when needed to calculate averages
   * rounding errors are kept low by using modified kahan neumaier summation and not using a
     recursive implementation. both properties which make it much slower than many other implementations"
  status-declare
  (memreg-init 1)
  (declare
    left sp-sample-count-t
    right sp-sample-count-t
    width sp-sample-count-t
    window sp-sample-t*
    window-index sp-sample-count-t)
  (if (not source-len) (goto exit))
  (set
    width (+ 1 (* 2 radius))
    window 0)
  (sc-comment "check if all required samples are in source array")
  (if (not (and (>= start radius) (<= (+ start radius 1) source-len)))
    (begin
      (status-require (sph-helper-malloc (* width (sizeof sp-sample-t)) &window))
      (memreg-add window)))
  (while (<= start end)
    (if (and (>= start radius) (<= (+ start radius 1) source-len))
      (set *result-samples (/ (sp-sample-sum (- (+ source start) radius) width) width))
      (begin
        (set window-index 0)
        (sc-comment "get samples from previous segment")
        (if (< start radius)
          (begin
            (set right (- radius start))
            (if prev
              (begin
                (set left
                  (if* (> right prev-len) 0
                    (- prev-len right)))
                (while (< left prev-len)
                  (set
                    (array-get window window-index) (array-get prev left)
                    window-index (+ 1 window-index)
                    left (+ 1 left)))))
            (while (< window-index right)
              (set
                (array-get window window-index) 0
                window-index (+ 1 window-index)))
            (set left 0))
          (set left (- start radius)))
        (sc-comment "get samples from source segment")
        (set right (+ start radius))
        (if (>= right source-len) (set right (- source-len 1)))
        (while (<= left right)
          (set
            (array-get window window-index) (array-get source left)
            window-index (+ 1 window-index)
            left (+ 1 left)))
        (sc-comment "get samples from next segment")
        (set right (+ start radius))
        (if (and (>= right source-len) next)
          (begin
            (set
              left 0
              right (- right source-len))
            (if (>= right next-len) (set right (- next-len 1)))
            (while (<= left right)
              (set
                (array-get window window-index) (array-get next left)
                window-index (+ 1 window-index)
                left (+ 1 left)))))
        (sc-comment "fill unset values in window with zero")
        (while (< window-index width)
          (set
            (array-get window window-index) 0
            window-index (+ 1 window-index)))
        (sc-comment "set current value to the window average")
        (set *result-samples (/ (sp-sample-sum window width) width))))
    (set
      result-samples (+ 1 result-samples)
      start (+ 1 start)))
  (label exit
    memreg-free
    (return status)))

(define (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-sample-count-t)
  "modify an impulse response kernel for spectral inversion.
   a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response top to bottom"
  (declare
    center sp-sample-count-t
    i sp-sample-count-t)
  (for ((set i 0) (< i a-len) (set i (+ 1 i)))
    (set (array-get a i) (* -1 (array-get a i))))
  (set
    center (/ (- a-len 1) 2)
    (array-get a center) (+ 1 (array-get a center))))

(define (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-sample-count-t)
  "inverts the sign for samples at odd indexes.
  a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response left to right"
  (while (> a-len 1)
    (set
      a-len (- a-len 2)
      (array-get a a-len) (* -1 (array-get a a-len)))))

(define (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-t*)
  "discrete linear convolution.
  result-samples must be all zeros, its length must be at least a-len + b-len - 1.
  result-samples is owned and allocated by the caller"
  (declare
    a-index sp-sample-count-t
    b-index sp-sample-count-t)
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
  (void
    sp-sample-t*
    sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-count-t sp-sample-t* sp-sample-t*)
  "discrete linear convolution for sample arrays, possibly of a continuous stream. maps segments (a, a-len) to result-samples
  using (b, b-len) as the impulse response. b-len must be greater than zero.
  all heap memory is owned and allocated by the caller.
  result-samples length is a-len.
  result-carryover is previous carryover or an empty array.
  result-carryover length must at least b-len - 1.
  continuous processing does not work correctly if result-samples is smaller than b-len - 1, in this case
  result-carryover will contain values after index a-len - 1 that will not be carried over to the next call.
  carryover-len should be zero for the first call or its content should be zeros.
  carryover-len for subsequent calls should be b-len - 1 or if b-len changed b-len - 1  from the previous call.
  if b-len is one then there is no carryover.
  if a-len is smaller than b-len then, with the current implementation, additional performance costs ensue from shifting the carryover array each call"
  (declare
    size sp-sample-count-t
    a-index sp-sample-count-t
    b-index sp-sample-count-t
    c-index sp-sample-count-t)
  (if carryover-len
    (if (<= carryover-len a-len)
      (begin
        (sc-comment "copy all entries to result and reset")
        (memcpy result-samples result-carryover (* carryover-len (sizeof sp-sample-t)))
        (memset result-carryover 0 (* carryover-len (sizeof sp-sample-t)))
        (memset (+ carryover-len result-samples) 0 (* (- a-len carryover-len) (sizeof sp-sample-t))))
      (begin
        (sc-comment "copy as many entries as fit into result and shift remaining to the left")
        (memcpy result-samples result-carryover (* a-len (sizeof sp-sample-t)))
        (memmove
          result-carryover
          (+ a-len result-carryover) (* (- carryover-len a-len) (sizeof sp-sample-t)))
        (memset (+ (- carryover-len a-len) result-samples) 0 (* a-len (sizeof sp-sample-t)))))
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
    carryover-alloc-len sp-sample-count-t
    ir sp-sample-t*
    ir-len sp-sample-count-t
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
          (= state:ir-f ir-f) (= 0 (memcmp state:ir-f-arguments ir-f-arguments ir-f-arguments-len)))
        (begin
          (sc-comment "unchanged")
          (return status))
        (begin
          (sc-comment "changed")
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
        state:ir-f ir-f)))
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
    sp-sample-count-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  "convolute samples \"in\", which can be a segment of a continuous stream, with an impulse response
  kernel created by ir-f with ir-f-arguments.
  ir-f is only used when ir-f-arguments changed.
  values that need to be carried over with calls are kept in out-state.
  * out-state: if zero then state will be allocated. owned by caller. the state can currently not be reused with varying ir-f-argument sizes.
  * out-samples: owned by the caller. length must be at least in-len and the number of output samples will be in-len"
  status-declare
  (declare carryover-len sp-sample-count-t)
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

(pre-include "../main/windowed-sinc.c" "../main/io.c")