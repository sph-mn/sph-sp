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

(define (sp-sin-lq a) (sp-sample-t sp-float-t)
  "lower precision version of sin() that should be faster"
  (declare b sp-sample-t c sp-sample-t)
  (set b (/ 4 M_PI) c (/ -4 (* M_PI M_PI)))
  (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-phase current change cycle) (sp-time-t sp-time-t sp-time-t sp-time-t)
  (define a sp-time-t (+ current change))
  (return (if* (< a cycle) a (modulo a cycle))))

(define (sp-phase-float current change cycle) (sp-time-t sp-time-t sp-sample-t sp-time-t)
  "accumulate an integer phase with change given as a float value.
   change must be a positive value and is rounded to the next larger integer"
  (define a sp-time-t (+ current (sp-cheap-ceiling-positive change)))
  (return (if* (< a cycle) a (modulo a cycle))))

(define (sp-wave start duration state out) (void sp-time-t sp-time-t sp-wave-state-t* sp-block-t)
  "* sums into out
   * state.spd (speed): array with hertz values
   * state.wvf (waveform): array with waveform samples
   * state.wvf-size: size of state.wvf, should match sample rate
   * state.phs (phase): value per channel
   * state.amp (amplitude): array per channel"
  (declare amp sp-sample-t channel-i sp-time-t phs sp-time-t i sp-time-t)
  (for ((set channel-i 0) (< channel-i out.channels) (set channel-i (+ 1 channel-i)))
    (set phs (array-get state:phs channel-i))
    (for ((set i 0) (< i duration) (set i (+ 1 i)))
      (set+ (array-get out.samples channel-i i)
        (* (array-get state:amp channel-i i) (array-get state:wvf phs)) phs (array-get state:spd i))
      (if (>= phs state:wvf-size) (set phs (modulo phs state:wvf-size))))
    (set (array-get state:phs channel-i) phs)))

(pre-define (sp-wave-state-set-channel a channel amp-array phs-value)
  (set (array-get a.amp channel) amp-array (array-get a.phs channel) phs-value))

(define (sp-wave-state-1 wvf wvf-size spd amp phs)
  (sp-wave-state-t sp-sample-t* sp-time-t sp-time-t* sp-sample-t* sp-time-t)
  "setup a single channel wave config"
  (declare a sp-wave-state-t)
  (set a.spd spd a.wvf wvf a.wvf-size wvf-size)
  (sp-wave-state-set-channel a 0 amp phs)
  (return a))

(define (sp-wave-state-2 wvf wvf-size spd amp1 amp2 phs1 phs2)
  (sp-wave-state-t sp-sample-t* sp-time-t sp-time-t* sp-sample-t* sp-sample-t* sp-time-t sp-time-t)
  (declare a sp-wave-state-t)
  (set a.spd spd a.wvf wvf a.wvf-size wvf-size)
  (sp-wave-state-set-channel a 0 amp1 phs1)
  (sp-wave-state-set-channel a 1 amp2 phs2)
  (return a))

(define (sp-triangle t a b) (sp-sample-t sp-time-t sp-time-t sp-time-t)
  "return a sample for a triangular wave with center offsets a left and b right.
   creates sawtooth waves if either a or b is 0"
  (declare remainder sp-time-t)
  (set remainder (modulo t (+ a b)))
  (return
    (if* (< remainder a) (* remainder (/ 1 (convert-type a sp-sample-t)))
      (* (- (convert-type b sp-sample-t) (- remainder (convert-type a sp-sample-t)))
        (/ 1 (convert-type b sp-sample-t))))))

(define (sp-triangle-96 t) (sp-sample-t sp-time-t) (return (sp-triangle t 48000 48000)))

(define (sp-square-96 t) (sp-sample-t sp-time-t)
  (return (if* (< (modulo (* 2 t) (* 2 96000)) 96000) -1 1)))

(define (sp-sine-table-new out size) (status-t sp-sample-t** sp-time-t)
  "writes a sine wave of size into out. can be used to create lookup tables"
  status-declare
  (declare i sp-time-t out-array sp-sample-t*)
  (status-require (sph-helper-malloc (* size (sizeof sp-sample-t*)) &out-array))
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set (array-get out-array i) (sin (* i (/ M_PI (/ size 2))))))
  (set *out out-array)
  (label exit status-return))

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

(pre-include "../main/arrays.c")

(define (sp-block-zero a) (void sp-block-t)
  (declare i sp-channels-t)
  (for ((set i 0) (< i a.channels) (set+ i 1))
    (sp-samples-zero (array-get a.samples i) a.size)))

(define (sp-path-samples segments size out) (status-t sp-path-segments-t sp-time-t sp-sample-t**)
  "out memory is allocated"
  status-declare
  (declare result sp-sample-t*)
  (status-require (sp-samples-new size &result))
  (if (spline-path-new-get segments.size segments.data 0 size result)
    (begin (free result) sp-memory-error))
  (set *out result)
  (label exit status-return))

(define (sp-path-times segments size out) (status-t sp-path-segments-t sp-time-t sp-time-t**)
  "create a new path from the given segments config.
   memory is allocated and ownership transferred to the caller"
  status-declare
  (declare result sp-time-t* temp sp-sample-t*)
  (status-require (sp-path-samples segments size &temp))
  (status-require (sp-times-new size &result))
  (sp-samples->times temp size result)
  (set *out result)
  (label exit status-return))

(define (sp-path-times-1 out size s1) (status-t sp-time-t** sp-time-t sp-path-segment-t)
  (declare segments sp-path-segments-t segments-data (array sp-path-segment-t 1))
  (set segments.size 1 segments.data segments-data)
  (array-set segments-data 0 s1)
  (return (sp-path-times segments size out)))

(define (sp-path-times-2 out size s1 s2)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t)
  (declare segments sp-path-segments-t segments-data (array sp-path-segment-t 2))
  (set segments.size 2 segments.data segments-data)
  (array-set segments-data 0 s1 1 s2)
  (return (sp-path-times segments size out)))

(define (sp-path-times-3 out size s1 s2 s3)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (declare segments sp-path-segments-t segments-data (array sp-path-segment-t 3))
  (set segments.size 3 segments.data segments-data)
  (array-set segments-data 0 s1 1 s2 2 s3)
  (return (sp-path-times segments size out)))

(define (sp-path-times-4 out size s1 s2 s3 s4)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (declare segments sp-path-segments-t segments-data (array sp-path-segment-t 4))
  (set segments.size 4 segments.data segments-data)
  (array-set segments-data 0 s1 1 s2 2 s3 3 s4)
  (return (sp-path-times segments size out)))

(define (sp-path-samples-1 out size s1) (status-t sp-sample-t** sp-time-t sp-path-segment-t)
  (declare segments sp-path-segments-t segments-data (array sp-path-segment-t 1))
  (set segments.size 1 segments.data segments-data)
  (array-set segments-data 0 s1)
  (return (sp-path-samples segments size out)))

(define (sp-path-samples-2 out size s1 s2)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t)
  (declare segments sp-path-segments-t segments-data (array sp-path-segment-t 2))
  (set segments.size 2 segments.data segments-data)
  (array-set segments-data 0 s1 1 s2)
  (return (sp-path-samples segments size out)))

(define (sp-path-samples-3 out size s1 s2 s3)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (declare segments sp-path-segments-t segments-data (array sp-path-segment-t 3))
  (set segments.size 3 segments.data segments-data)
  (array-set segments-data 0 s1 1 s2 2 s3)
  (return (sp-path-samples segments size out)))

(define (sp-path-samples-4 out size s1 s2 s3 s4)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (declare segments sp-path-segments-t segments-data (array sp-path-segment-t 4))
  (set segments.size 4 segments.data segments-data)
  (array-set segments-data 0 s1 1 s2 2 s3 3 s4)
  (return (sp-path-samples segments size out)))

(pre-include "../main/io.c" "../main/plot.c" "../main/filter.c" "../main/sequencer.c")

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