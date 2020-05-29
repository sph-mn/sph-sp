(sc-comment "this file contains basics and includes dependencies")
(pre-define M_PI 3.141592653589793)

(pre-include "stdio.h" "fcntl.h"
  "sndfile.h" "foreign/nayuki-fft/fft.c" "../main/sph-sp.h"
  "sph/spline-path.c" "sph/helper.c" "sph/memreg.c"
  "sph/quicksort.c" "sph/queue.c" "sph/thread-pool.c" "sph/futures.c")

(pre-define
  sp-status-declare (status-declare-group sp-s-group-sp)
  (max a b) (if* (> a b) a b)
  (min a b) (if* (< a b) a b)
  (sp-libc-s-id id) (if (< id 0) (status-set-goto sp-s-group-libc id))
  (sp-libc-s expression)
  (begin
    (set status.id expression)
    (if (< status.id 0) (begin (set status.group sp-s-group-libc) (goto exit)) (set status.id 0)))
  (define-sp-interleave name type body)
  (begin
    "define a deinterleave, interleave or similar routine.
     a: source
     b: target"
    (define (name a b a-size channel-count) (void type** type* sp-time-t sp-channels-t)
      (declare b-size sp-time-t channel sp-channels-t)
      (set b-size (* a-size channel-count))
      (while a-size
        (set a-size (- a-size 1) channel channel-count)
        (while channel (set channel (- channel 1) b-size (- b-size 1)) body))))
  sp-memory-error (status-set-goto sp-s-group-sp sp-s-id-memory))

(define-sp-interleave sp-interleave sp-sample-t
  (set (array-get b b-size) (array-get (array-get a channel) a-size)))

(define-sp-interleave sp-deinterleave sp-sample-t
  (set (array-get (array-get a channel) a-size) (array-get b b-size)))

(sph-random-define-x256p sp-random-samples sp-sample-t (- (* 2 (sph-random-f64-from-u64 a)) 1.0))
(sph-random-define-x256ss sp-random-times sp-time-t a)

(define (display-samples a len) (void sp-sample-t* sp-time-t)
  "display a sample array in one line"
  (declare i sp-time-t)
  (printf "%.17g" (array-get a 0))
  (for ((set i 1) (< i len) (set i (+ 1 i))) (printf " %.17g" (array-get a i)))
  (printf "\n"))

(define (sp-status-description a) (uint8-t* status-t)
  "get a string description for a status id in a status-t"
  (declare b uint8-t*)
  (cond
    ( (not (strcmp sp-s-group-sp a.group))
      (case = a.id
        (sp-s-id-eof (set b "end of file"))
        (sp-s-id-input-type (set b "input argument is of wrong type"))
        (sp-s-id-not-implemented (set b "not implemented"))
        (sp-s-id-memory (set b "memory allocation error"))
        (sp-s-id-file-incompatible
          (set b "file channel count or sample rate is different from what was requested"))
        (sp-s-id-file-incomplete (set b "incomplete write"))
        (else (set b ""))))
    ( (not (strcmp sp-s-group-sndfile a.group))
      (set b (convert-type (sf-error-number a.id) uint8-t*)))
    ((not (strcmp sp-s-group-sph a.group)) (set b (sph-helper-status-description a)))
    (else (set b "")))
  (return b))

(define (sp-status-name a) (uint8-t* status-t)
  "get a single word identifier for a status id in a status-t"
  (declare b uint8-t*)
  (cond
    ( (= 0 (strcmp sp-s-group-sp a.group))
      (case = a.id
        (sp-s-id-input-type (set b "input-type"))
        (sp-s-id-not-implemented (set b "not-implemented"))
        (sp-s-id-memory (set b "memory"))
        (else (set b "unknown"))))
    ((= 0 (strcmp sp-s-group-sndfile a.group)) (set b "sndfile")) (else (set b "unknown")))
  (return b))

(define (sp-block-new channels size out) (status-t sp-channels-t sp-time-t sp-block-t*)
  "return a newly allocated array for channels with data arrays for each channel"
  status-declare
  (memreg-init channels)
  (declare channel sp-sample-t* i sp-time-t)
  (for ((set i 0) (< i channels) (set i (+ 1 i)))
    (status-require (sph-helper-calloc (* size (sizeof sp-sample-t)) &channel))
    (memreg-add channel)
    (set (array-get out:samples i) channel))
  (set out:size size out:channels channels)
  (label exit (if status-is-failure memreg-free) status-return))

(define (sp-block-free a) (void sp-block-t)
  (declare i sp-time-t)
  (for ((set i 0) (< i a.channels) (set i (+ 1 i))) (free (array-get a.samples i))))

(define (sp-block-with-offset a offset) (sp-block-t sp-block-t sp-time-t)
  "return a new block with offset added to all channel sample arrays"
  (declare i sp-time-t)
  (for ((set i 0) (< i a.channels) (set i (+ 1 i)))
    (set (array-get a.samples i) (+ offset (array-get a.samples i))))
  (return a))

(define (sp-samples-new size out) (status-t sp-time-t sp-sample-t**)
  (return (sph-helper-calloc (* size (sizeof sp-sample-t)) out)))

(define (sp-times-new size out) (status-t sp-time-t sp-time-t**)
  (return (sph-helper-calloc (* size (sizeof sp-time-t)) out)))

(define (sp-sin-lq a) (sp-sample-t sp-float-t)
  "lower precision version of sin() that should be faster"
  (declare b sp-sample-t c sp-sample-t)
  (set b (/ 4 M_PI) c (/ -4 (* M_PI M_PI)))
  (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-sinc a) (sp-float-t sp-float-t)
  "the normalised sinc function"
  (return (if* (= 0 a) 1 (/ (sin (* M_PI a)) (* M_PI a)))))

(define (sp-fft input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  "all arrays should be input-len and are managed by the caller"
  (return (not (Fft_transform input/output-real input/output-imag input-len))))

(define (sp-ffti input-len input/output-real input/output-imag) (int sp-time-t double* double*)
  "[[real, imaginary], ...]:complex-numbers -> real-numbers
   input-length > 0
   output-length = input-length
   output is allocated and owned by the caller"
  (return (not (= 1 (Fft_inverseTransform input/output-real input/output-imag input-len)))))

(define
  (sp-moving-average in in-end in-window in-window-end prev prev-end next next-end radius out)
  (status-t sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-sample-t* sp-time-t sp-sample-t*)
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
    outside-count sp-time-t
    in-missing sp-time-t
    width sp-time-t)
  (set width (+ 1 radius radius))
  (while (<= in-window in-window-end)
    (set
      (array-get sums 0) 0
      (array-get sums 1) 0
      (array-get sums 2) 0
      in-left (max in (- in-window radius))
      in-right (min in-end (+ in-window radius))
      (array-get sums 1) (sp-samples-sum in-left (+ 1 (- in-right in-left))))
    (if (and (< (- in-window in-left) radius) prev)
      (begin
        (set
          in-missing (- radius (- in-window in-left))
          outside (max prev (- prev-end in-missing))
          outside-count (- prev-end outside)
          (array-get sums 0) (sp-samples-sum outside outside-count))))
    (if (and (< (- in-right in-window) radius) next)
      (begin
        (set
          in-missing (- radius (- in-right in-window))
          outside next
          outside-count (min (- next-end next) in-missing)
          (array-get sums 2) (sp-samples-sum outside outside-count))))
    (set
      (pointer-get out) (/ (sp-samples-sum sums 3) width)
      out (+ 1 out)
      in-window (+ 1 in-window)))
  status-return)

(define (sp-spectral-inversion-ir a a-len) (void sp-sample-t* sp-time-t)
  "modify an impulse response kernel for spectral inversion.
   a-len must be odd and \"a\" must have left-right symmetry.
   flips the frequency response top to bottom"
  (declare center sp-time-t i sp-time-t)
  (for ((set i 0) (< i a-len) (set i (+ 1 i))) (set (array-get a i) (* -1 (array-get a i))))
  (set center (/ (- a-len 1) 2) (array-get a center) (+ 1 (array-get a center))))

(define (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-time-t)
  "inverts the sign for samples at odd indexes.
   a-len must be odd and \"a\" must have left-right symmetry.
   flips the frequency response left to right"
  (while (> a-len 1) (set a-len (- a-len 2) (array-get a a-len) (* -1 (array-get a a-len)))))

(define (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-sample-t*)
  "discrete linear convolution.
   result-samples must be all zeros, its length must be at least a-len + b-len - 1.
   result-samples is owned and allocated by the caller"
  (declare a-index sp-time-t b-index sp-time-t)
  (set a-index 0 b-index 0)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set
        (array-get result-samples (+ a-index b-index))
        (+ (array-get result-samples (+ a-index b-index))
          (* (array-get a a-index) (array-get b b-index)))
        b-index (+ 1 b-index)))
    (set b-index 0 a-index (+ 1 a-index))))

(define (sp-convolve a a-len b b-len carryover-len result-carryover result-samples)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-time-t sp-sample-t* sp-sample-t*)
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
  (declare size sp-time-t a-index sp-time-t b-index sp-time-t c-index sp-time-t)
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
        (memmove result-carryover (+ a-len result-carryover)
          (* (- carryover-len a-len) (sizeof sp-sample-t)))
        (memset (+ (- carryover-len a-len) result-carryover) 0 (* a-len (sizeof sp-sample-t)))))
    (memset result-samples 0 (* a-len (sizeof sp-sample-t))))
  (sc-comment "result values." "first process values that dont lead to carryover")
  (set size (if* (< a-len b-len) 0 (- a-len (- b-len 1))))
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

(define (sp-samples-absolute-max in in-size) (sp-sample-t sp-sample-t* sp-time-t)
  "get the maximum value in samples array, disregarding sign"
  (declare result sp-sample-t a sp-sample-t i sp-time-t)
  (for ((set i 0 result 0) (< i in-size) (set i (+ 1 i)))
    (set a (fabs (array-get in i)))
    (if (> a result) (set result a)))
  (return result))

(define (sp-set-unity-gain in in-size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  "adjust amplitude of out to match the one of in"
  (declare
    i sp-time-t
    in-max sp-sample-t
    out-max sp-sample-t
    difference sp-sample-t
    correction sp-sample-t)
  (set in-max (sp-samples-absolute-max in in-size) out-max (sp-samples-absolute-max out in-size))
  (if (or (= 0 in-max) (= 0 out-max)) return)
  (set difference (/ out-max in-max) correction (+ 1 (/ (- 1 difference) difference)))
  (for ((set i 0) (< i in-size) (set i (+ 1 i)))
    (set (array-get out i) (* correction (array-get out i)))))

(define (sp-samples->time in in-size out) (void sp-sample-t* sp-time-t sp-time-t*)
  (declare i sp-time-t)
  (for ((set i 0) (< i in-size) (set i (+ 1 i)))
    (set (array-get out i) (sp-cheap-round-positive (array-get in i)))))

(pre-include "../main/io.c" "../main/plot.c"
  "../main/filter.c" "../main/synthesiser.c" "../main/sequencer.c" "../main/path.c")

(define (sp-render-file event start duration config path)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t uint8-t*)
  "render a single event to file. event can be a group"
  status-declare
  (declare
    block sp-block-t
    end sp-time-t
    file-is-open uint8-t
    file sp-file-t
    i sp-time-t
    written sp-time-t)
  (set file-is-open 0)
  (status-require (sp-block-new config.channels config.block-size &block))
  (status-require (sp-file-open path sp-file-mode-write config.channels config.rate &file))
  (set
    file-is-open 1
    end (/ duration config.block-size)
    end (if* (> 1 end) config.block-size (* end config.block-size)))
  (for ((set i 0) (< i end) (set+ i config.block-size))
    (sp-seq i (+ i config.block-size) block &event 1)
    (status-require (sp-file-write &file block.samples config.block-size &written))
    (sp-block-zero block))
  (label exit (if file-is-open (sp-file-close &file)) status-return))

(define (sp-initialise cpu-count) (status-t uint16-t)
  "fills the sine wave lookup table"
  status-declare
  (if cpu-count (begin (set status.id (future-init cpu-count)) (if status.id status-return)))
  (set sp-cpu-count cpu-count sp-default-random-state (sp-random-state-new 1557083953))
  (return (sp-sine-table-new &sp-sine-96-table 96000)))