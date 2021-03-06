(sc-comment "this file contains basics and includes dependencies")
(pre-define M_PI 3.141592653589793)

(pre-include "stdio.h" "fcntl.h"
  "sndfile.h" "foreign/nayuki-fft/fft.c" "../main/sph-sp.h"
  "sph/spline-path.c" "sph/quicksort.c" "sph/queue.c" "sph/thread-pool.c" "sph/futures.c")

(pre-define
  sp-status-declare (status-declare-group sp-s-group-sp)
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
    (define (name a b a-size channel-count) (void type** type* sp-time-t sp-channel-count-t)
      (declare b-size sp-time-t channel sp-channel-count-t)
      (set b-size (* a-size channel-count))
      (while a-size
        (set a-size (- a-size 1) channel channel-count)
        (while channel (set channel (- channel 1) b-size (- b-size 1)) body))))
  sp-memory-error (status-set-goto sp-s-group-sp sp-s-id-memory)
  (sp-modvalue fixed array index) (if* array (array-get array index) fixed)
  (sp-array-or-fixed array fixed index) (if* array (array-get array index) fixed))

(define-sp-interleave sp-interleave sp-sample-t
  (set (array-get b b-size) (array-get (array-get a channel) a-size)))

(define-sp-interleave sp-deinterleave sp-sample-t
  (set (array-get (array-get a channel) a-size) (array-get b b-size)))

(define (sp-status-description a) (uint8-t* status-t)
  "get a string description for a status id in a status_t"
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
  "get a one word identifier for status id in status_t"
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

(define (sp-block-new channels size out) (status-t sp-channel-count-t sp-time-t sp-block-t*)
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

(define (sp-block-free a) (void sp-block-t*)
  (declare i sp-time-t)
  (if a:size (for ((set i 0) (< i a:channels) (set+ i 1)) (free (array-get a:samples i)))))

(define (sp-block-with-offset a offset) (sp-block-t sp-block-t sp-time-t)
  "return a new block with offset added to all channel sample arrays"
  (declare i sp-time-t)
  (for ((set i 0) (< i a.channels) (set i (+ 1 i)))
    (set (array-get a.samples i) (+ offset (array-get a.samples i))))
  (return a))

(define (sp-sin-lq a) (sp-sample-t sp-sample-t)
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

(define (sp-triangle t a b) (sp-sample-t sp-time-t sp-time-t sp-time-t)
  "return a sample for a triangular wave with center offsets a left and b right.
   creates sawtooth waves if either a or b is 0"
  (declare remainder sp-time-t)
  (set remainder (modulo t (+ a b)))
  (return
    (if* (< remainder a) (* remainder (/ 1 (convert-type a sp-sample-t)))
      (* (- (convert-type b sp-sample-t) (- remainder (convert-type a sp-sample-t)))
        (/ 1 (convert-type b sp-sample-t))))))

(define (sp-square t size) (sp-sample-t sp-time-t sp-time-t)
  (return (if* (< (modulo (* 2 t) (* 2 size)) size) -1 1)))

(define (sp-sine-period size out) (void sp-time-t sp-sample-t*)
  "writes one full period of a sine wave into out. can be used to create lookup tables"
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set (array-get out i) (sin (* i (/ M_PI (/ size 2)))))))

(define (sp-sinc a) (sp-sample-t sp-sample-t)
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

(define (sp-time-expt base exp) (sp-time-t sp-time-t sp-time-t)
  (define a sp-time-t 1)
  (for ((begin) (begin) (begin))
    (if (bit-and exp 1) (set* a base))
    (set exp (bit-shift-right exp 1))
    (if (not exp) break)
    (set* base base))
  (return a))

(define (sp-time-factorial a) (sp-time-t sp-time-t)
  (declare result sp-time-t)
  (set result 1)
  (while (> a 0) (set result (* result a) a (- a 1)))
  (return result))

(define (sp-set-sequence-max set-size selection-size) (sp-time-t sp-time-t sp-time-t)
  "return the maximum number of possible distinct selections from a set with length \"set-size\""
  (return (if* (= 0 set-size) 0 (sp-time-expt set-size selection-size))))

(define (sp-permutations-max set-size selection-size) (sp-time-t sp-time-t sp-time-t)
  (return (/ (sp-time-factorial set-size) (- set-size selection-size))))

(define (sp-compositions-max sum) (sp-time-t sp-time-t) (return (sp-time-expt 2 (- sum 1))))
(pre-include "../main/arrays.c")

(define (sp-block-zero a) (void sp-block-t)
  (declare i sp-channel-count-t)
  (for ((set i 0) (< i a.channels) (set+ i 1)) (sp-samples-zero (array-get a.samples i) a.size)))

(define (sp-block-copy a b) (void sp-block-t sp-block-t)
  "copies all channels and samples from $a to $b.
   $b channel count and size must be equal or greater than $a"
  (declare ci sp-channel-count-t i sp-time-t)
  (for ((set ci 0) (< ci a.channels) (set+ ci 1))
    (for ((set i 0) (< i a.size) (set+ i 1))
      (set (array-get b.samples ci i) (array-get a.samples ci i)))))

(pre-include "../main/path.c" "../main/io.c"
  "../main/plot.c" "../main/filter.c" "../main/sequencer.c" "../main/statistics.c")

(define (sp-render-config channels rate block-size)
  (sp-render-config-t sp-channel-count-t sp-time-t sp-time-t)
  (declare a sp-render-config-t)
  (struct-set a channels channels rate rate block-size block-size)
  (return a))

(define (sp-render-file event start end config path)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t uint8-t*)
  "render an event with sp_seq to a file. the file is created or overwritten"
  status-declare
  (declare block-end sp-time-t remainder sp-time-t i sp-time-t written sp-time-t)
  (sp-block-declare block)
  (sp-file-declare file)
  (sp-declare-event-list events)
  (status-require (sp-event-list-add &events event))
  (status-require (sp-block-new config.channels config.block-size &block))
  (status-require (sp-file-open path sp-file-mode-write config.channels config.rate &file))
  (set
    remainder (modulo (- end start) config.block-size)
    block-end (* config.block-size (/ (- end start) config.block-size)))
  (for ((set i 0) (< i block-end) (set+ i config.block-size))
    (status-require (sp-seq i (+ i config.block-size) block &events))
    (status-require (sp-file-write &file block.samples config.block-size &written))
    (sp-block-zero block))
  (if remainder
    (begin
      (status-require (sp-seq i (+ i remainder) block &events))
      (status-require (sp-file-write &file block.samples remainder &written))))
  (label exit (sp-block-free &block) (sp-file-close file) status-return))

(define (sp-render-block event start end config out)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t sp-block-t*)
  "render a single event with sp_seq to sample arrays in sp_block_t.
   events should have been prepared with sp-seq-events-prepare.
   block will be allocated"
  status-declare
  (declare block sp-block-t)
  (sp-declare-event-list events)
  (status-require (sp-event-list-add &events event))
  (status-require (sp-block-new config.channels (- end start) &block))
  (status-require (sp-seq start end block &events))
  (set *out block)
  (label exit status-return))

(define (sp-render-quick event file-or-plot) (status-t sp-event-t uint8-t)
  "render the full duration of events with defaults to /tmp/sp-out.wav or plot the result.
   example: sp_render_quick(event, 2, 48000, 1)"
  status-declare
  (declare block sp-block-t config sp-render-config-t start sp-time-t end sp-time-t)
  (set config (sp-render-config sp-channels sp-rate sp-rate) start event.start end event.end)
  (printf "rendering %lu seconds to %s\n" (sp-cheap-round-positive (/ (- end start) config.rate))
    (if* file-or-plot "plot" "file"))
  (if end
    (if file-or-plot
      (begin
        (status-require (sp-render-block event 0 end config &block))
        (sp-plot-samples (array-get block.samples 0) end))
      (status-require (sp-render-file event 0 end config "/tmp/sp-out.wav"))))
  (label exit status-return))

(define (sp-random-state-new seed) (sp-random-state-t sp-time-t)
  (define result sp-random-state-t (sph-random-state-new seed))
  (sc-comment "random state needs warm-up for some reason")
  (sp-time-random &result)
  (sp-time-random &result)
  (return result))

(define (sp-initialize cpu-count channels rate) (status-t uint16-t sp-channel-count-t sp-time-t)
  "fills the sine wave lookup table.
   rate and channels are used to set sp_rate and sp_channels,
   which are used as defaults in a few cases"
  status-declare
  (if cpu-count (begin (set status.id (future-init cpu-count)) (if status.id status-return)))
  (set
    sp-cpu-count cpu-count
    sp-rate rate
    sp-channels channels
    sp-random-state (sp-random-state-new sp-random-seed)
    sp-sine-lfo-factor 10)
  (status-require (sp-samples-new sp-rate &sp-sine-table))
  (status-require (sp-samples-new (* sp-rate sp-sine-lfo-factor) &sp-sine-table-lfo))
  (sp-sine-period sp-rate sp-sine-table)
  (sp-sine-period (* sp-rate sp-sine-lfo-factor) sp-sine-table-lfo)
  (label exit status-return))