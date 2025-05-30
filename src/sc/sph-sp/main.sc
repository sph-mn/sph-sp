(pre-define-if-not-defined _XOPEN_SOURCE)
(pre-define-if-not-defined _POSIX_C_SOURCE 199309L)

(pre-include "math.h" "errno.h"
  "arpa/inet.h" "nayuki-fft/fft.c" "../sph-sp/sph-sp.h"
  "sph/spline-path.c" "sph/quicksort.h" "sph/queue.h"
  "sph/random.c" "sph/float.c" "sph/thread-pool.h"
  "sph/thread-pool.c" "sph/futures.h" "sph/futures.c" "sph/helper.c")

(pre-define
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
  (sp-modvalue fixed array index)
  (begin
    "fixed if array is zero, otherwise array value at index"
    (if* array (array-get array index) fixed)))

(pre-include "sph-sp/arrays.c")
(pre-include "sph-sp/distributions.c")

(define-sp-interleave sp-interleave sp-sample-t
  (set (array-get b b-size) (array-get (array-get a channel) a-size)))

(define-sp-interleave sp-deinterleave sp-sample-t
  (set (array-get (array-get a channel) a-size) (array-get b b-size)))

(define (sp-sample-max a b) (sp-sample-t sp-sample-t sp-sample-t) (return (if* (> a b) a b)))
(define (sp-time-max a b) (sp-time-t sp-time-t sp-time-t) (return (if* (> a b) a b)))
(define (sp-sample-min a b) (sp-sample-t sp-sample-t sp-sample-t) (return (if* (> a b) b a)))
(define (sp-time-min a b) (sp-time-t sp-time-t sp-time-t) (return (if* (> a b) b a)))

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
        (sp-s-id-file-write (set b "invalid file write"))
        (sp-s-id-file-read (set b "invalid file read"))
        (sp-s-id-file-not-implemented (set b "unsupported format (only 32 bit float supported)"))
        (sp-s-id-file-eof (set b "end of file"))
        (sp-s-id-invalid-argument (set b "invalid argument"))
        (else (set b ""))))
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
        (sp-s-id-file-write (set b "invalid-file-write"))
        (sp-s-id-file-read (set b "invalid-file-read"))
        (sp-s-id-file-not-implemented (set b "not-implemented"))
        (sp-s-id-file-eof (set b "end-of-file"))
        (sp-s-id-invalid-argument (set b "invalid-argument"))
        (else (set b "unknown"))))
    (else (set b "unknown")))
  (return b))

(define (sp-block-new channel-count size out) (status-t sp-channel-count-t sp-time-t sp-block-t*)
  "return a newly allocated array for channel-count with data arrays for each channel"
  status-declare
  (memreg-init channel-count)
  (declare channel sp-sample-t*)
  (sp-for-each-index i channel-count
    (status-require (sph-helper-calloc (* size (sizeof sp-sample-t)) &channel))
    (memreg-add channel)
    (set (array-get out:samples i) channel))
  (set out:size size out:channel-count channel-count)
  (label exit (if status-is-failure memreg-free) status-return))

(define (sp-block-free a) (void sp-block-t*)
  (if a:size (sp-for-each-index i a:channel-count (free (array-get a:samples i)))))

(define (sp-block-with-offset a offset) (sp-block-t sp-block-t sp-time-t)
  "return a new block with offset added to all channel sample arrays"
  (sp-for-each-index i a.channel-count (set+ (array-get a.samples i) offset))
  (return a))

(define (sp-block-zero a) (void sp-block-t)
  (sp-for-each-index i a.channel-count (sp-samples-zero (array-get a.samples i) a.size)))

(define (sp-block-copy a b) (void sp-block-t sp-block-t)
  "copies all channel-count and samples from $a to $b.
   $b channel count and size must be equal or greater than $a"
  (sp-for-each-index ci a.channel-count
    (sp-for-each-index i a.size (set (array-get b.samples ci i) (array-get a.samples ci i)))))

(define (sp-phase current change cycle) (sp-time-t sp-time-t sp-time-t sp-time-t)
  (define a sp-time-t (+ current change))
  (return (if* (< a cycle) a (modulo a cycle))))

(define (sp-phase-float current change cycle) (sp-time-t sp-time-t sp-sample-t sp-time-t)
  "accumulate an integer phase with change given as a float value.
   change must be a positive value and is rounded to the next larger integer"
  (define a sp-time-t (+ current (sp-cheap-ceiling-positive change)))
  (return (if* (< a cycle) a (modulo a cycle))))

(define (sp-sine-period size out) (void sp-time-t sp-sample-t*)
  "writes one full period of a sine wave into out. the sine has the frequency that makes it fit exactly into size.
   can be used to create lookup tables"
  (sp-for-each-index i size (set (array-get out i) (sin (* i (/ M_PI (/ size 2)))))))

(define (sp-wave size wvf wvf-size amp amod frq fmod phs-state out)
  (void sp-time-t sp-sample-t* sp-time-t sp-sample-t sp-sample-t* sp-time-t sp-time-t* sp-time-t* sp-sample-t*)
  "sums to out"
  (define phs sp-time-t (if* phs-state *phs-state 0))
  (sp-for-each-index i size
    (set+ (array-get out i) (* amp (sp-optional-array-get amod amp i) (array-get wvf phs))
      phs (sp-optional-array-get fmod frq i))
    (if (>= phs wvf-size) (set phs (modulo phs wvf-size))))
  (if phs-state (set *phs-state phs)))

(define (sp-sine size amp amod frq fmod phs-state out)
  (void sp-time-t sp-sample-t sp-sample-t* sp-time-t sp-time-t* sp-time-t* sp-sample-t*)
  (sp-wave size sp-sine-table sp-rate amp amod frq fmod phs-state out))

(define (sp-sine-lfo size amp amod frq fmod phs-state out)
  (void sp-time-t sp-sample-t sp-sample-t* sp-time-t sp-time-t* sp-time-t* sp-sample-t*)
  "supports frequencies below one hertz by using a larger lookup table.
   frq is (hertz / sp_sine_lfo_factor)"
  (sp-wave size sp-sine-table-lfo (* sp-rate sp-sine-lfo-factor) amp amod frq fmod phs-state out))

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
  (declare center sp-time-t)
  (sp-for-each-index i a-len (set* (array-get a i) -1))
  (set center (/ (- a-len 1) 2))
  (set+ (array-get a center) 1))

(define (sp-spectral-reversal-ir a a-len) (void sp-sample-t* sp-time-t)
  "inverts the sign for samples at odd indexes.
   a-len must be odd and \"a\" must have left-right symmetry.
   flips the frequency response left to right"
  (while (> a-len 1) (set- a-len 2) (set* (array-get a a-len) -1)))

(define (sp-convolve-one a a-len b b-len out)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-sample-t*)
  "discrete linear convolution.
   out must be all zeros, its length must be at least a-len + b-len - 1.
   out is owned and allocated by the caller"
  (define a-index sp-time-t 0)
  (define b-index sp-time-t 0)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set+ (array-get out (+ a-index b-index)) (* (array-get a a-index) (array-get b b-index))
        b-index 1))
    (set b-index 0)
    (set+ a-index 1)))

(define (sp-convolve a a-len b b-len carryover-len carryover out)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-time-t sp-time-t sp-sample-t* sp-sample-t*)
  "discrete linear convolution for sample arrays, possibly of a continuous stream. maps segments (a, a-len) to out
   using (b, b-len) as the impulse response. b-len must be greater than zero.
   all heap memory is owned and allocated by the caller.
   out length is a-len.
   carryover is previous carryover or an empty array.
   carryover length must at least b-len - 1.
   carryover-len should be zero for the first call or its content should be zeros.
   carryover-len for subsequent calls should be b-len - 1.
   if b-len changed it should be b-len - 1 from the previous call for the first call with the changed b-len.
   if b-len is one then there is no carryover.
   if a-len is smaller than b-len then, with the current implementation, additional performance costs ensue from shifting the carryover array each call.
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

(pre-include "sph-sp/plot.c" "sph-sp/filter.c"
  "sph-sp/sequencer.c" "sph-sp/statistics.c" "sph-sp/file.c")

(define (sp-render-config channel-count rate block-size display-progress)
  (sp-render-config-t sp-channel-count-t sp-time-t sp-time-t sp-bool-t)
  (declare a sp-render-config-t)
  (struct-set a
    channel-count channel-count
    rate rate
    block-size block-size
    display-progress display-progress)
  (return a))

(define (sp-render-range-file event start end config path)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t uint8-t*)
  "render an event with sp_seq to a file. the file is created or overwritten"
  status-declare
  (declare block-end sp-time-t remainder sp-time-t i sp-time-t file sp-file-t)
  (sp-block-declare block)
  (sp-declare-event-list events)
  (status-require (sp-event-list-add &events event))
  (status-require (sp-block-new config.channel-count config.block-size &block))
  (status-require (sp-file-open-write path config.channel-count config.rate &file))
  (set
    remainder (modulo (- end start) config.block-size)
    block-end (* config.block-size (/ (- end start) config.block-size)))
  (for ((set i 0) (< i block-end) (set+ i config.block-size))
    (if config.display-progress
      (printf "%.1f%\n" (* 100 (/ i (convert-type block-end sp-sample-t)))))
    (status-require (sp-seq i (+ i config.block-size) block &events))
    (status-require (sp-file-write &file block.samples config.block-size))
    (sp-block-zero block))
  (if remainder
    (begin
      (status-require (sp-seq i (+ i remainder) block &events))
      (status-require (sp-file-write &file block.samples remainder))))
  (sp-event-list-free &events)
  (label exit (sp-block-free &block) (sp-file-close-write &file) status-return))

(define (sp-render-range-block event start end config out)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t sp-block-t*)
  "render a single event with sp_seq to sample arrays in sp_block_t.
   events should have been prepared with sp-seq-events-prepare.
   block will be allocated"
  status-declare
  (declare block sp-block-t)
  (sp-declare-event-list events)
  (status-require (sp-event-list-add &events event))
  (status-require (sp-block-new config.channel-count (- end start) &block))
  (status-require (sp-seq start end block &events))
  (sp-event-list-free &events)
  (set *out block)
  (label exit status-return))

(define (sp-render-file event path) (status-t sp-event-t uint8-t*)
  "render the full duration of event to file at path and write information to standard output.
   uses channel count from global variable sp_channel_count and block size sp_rate"
  status-declare
  (if (not event.end) (sp-event-prepare-optional-srq event))
  (printf "rendering %lu seconds to file %s\n" (/ event.end sp-rate) path)
  (status-require
    (sp-render-range-file event 0
      event.end (sp-render-config sp-channel-count sp-rate (* sp-render-block-seconds sp-rate) #t)
      path))
  (label exit status-return))

(define (sp-render-plot event) (status-t sp-event-t)
  "render the full duration of event to file at path and write information to standard output.
   uses channel count from global variable sp_channel_count and block size sp_rate"
  status-declare
  (declare block sp-block-t)
  (if (not event.end) (sp-event-prepare-optional-srq event))
  (if (and event.end (< event.end sp-rate))
    (printf "rendering %lu milliseconds to plot\n" (/ event.end sp-rate 1000))
    (printf "rendering %lu seconds to plot\n" (/ event.end sp-rate)))
  (status-require
    (sp-render-range-block event 0
      event.end (sp-render-config sp-channel-count sp-rate (* sp-render-block-seconds sp-rate) #t)
      &block))
  (sp-plot-samples (array-get block.samples 0) event.end)
  (label exit status-return))

(define (sp-random-state-new seed) (sp-random-state-t sp-time-t)
  (define result sp-random-state-t (sph-random-state-new seed))
  (sc-comment "random state needs warm-up for some reason")
  (sp-time-random-primitive &result)
  (sp-time-random-primitive &result)
  (return result))

(define (sp-initialize cpu-count channel-count rate)
  (status-t uint16-t sp-channel-count-t sp-time-t)
  "fills the sine wave lookup table.
   rate and channel-count are used to set sp_rate and sp_channel-count,
   which are used as defaults in a few cases"
  status-declare
  (if cpu-count (begin (set status.id (sph-future-init cpu-count)) (if status.id status-return)))
  (set
    sp-sine-table 0
    sp-sine-table-lfo 0
    sp-cpu-count cpu-count
    sp-rate rate
    sp-channel-count channel-count
    sp-random-state (sp-random-state-new sp-random-seed)
    sp-sine-lfo-factor 100)
  (status-require (sp-samples-new sp-rate &sp-sine-table))
  (status-require (sp-samples-new (* sp-rate sp-sine-lfo-factor) &sp-sine-table-lfo))
  (sp-sine-period sp-rate sp-sine-table)
  (sp-sine-period (* sp-rate sp-sine-lfo-factor) sp-sine-table-lfo)
  (label exit (if status-is-failure (sp-deinitialize)) status-return))

(define (sp-deinitialize) void
  (if sp-cpu-count (sph-future-deinit))
  (free sp-sine-table)
  (free sp-sine-table-lfo))

(pre-include "sph-sp/path.c")
(sc-comment "extra")

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

(define (sp-pan->amp value channel) (sp-sample-t sp-sample-t sp-channel-count-t)
  "convert a pan value between 0..channel_count to a volume factor.
   values are interpreted as split between even and odd numbers, from channel..(channel + 1),
   0: second channel muted
   0.5: no channel muted
   1: first channel muted
   0.75: first channel 50%
   0.25: second channel 50%"
  (return
    (if* (bit-and 1 channel) (/ (sp-inline-limit value (- channel 1) (- channel 0.5)) 0.5)
      (- 1 (/ (- (sp-inline-limit value (+ channel 0.5) (+ channel 1)) 0.5) 0.5)))))

(define (sp-normal-random min max) (sp-time-t sp-time-t sp-time-t)
  "untested. return normally distributed numbers in range"
  (declare samples (array sp-time-t 32) result sp-sample-t)
  (sp-times-random-bounded (- max min) 32 samples)
  (sp-times-add samples 32 min)
  (sp-stat-times-mean samples 32 &result)
  (return (convert-type result sp-time-t)))

(define (sp-time-harmonize a base amount) (sp-time-t sp-time-t sp-time-t sp-sample-t)
  "untested. round value amount distance to nearest multiple of base.
   for example, amount 1.0 rounds fully, amount 0.0 does not round at all, amount 0.5 rounds half-way"
  (declare nearest sp-time-t)
  (set nearest (* (/ (- (+ a base) 1) base) base))
  (return (if* (> a nearest) (- a (* amount (- a nearest))) (+ a (* amount (- nearest a))))))

(define (sp-time-deharmonize a base amount) (sp-time-t sp-time-t sp-time-t sp-sample-t)
  "untested. the nearer values are to the multiple, the further move them randomly up to half base away"
  (declare nearest sp-time-t distance-ratio sp-sample-t)
  (set
    nearest (* (/ (- (+ a base) 1) base) base)
    distance-ratio
    (/ (- base (sp-inline-absolute-difference a nearest)) (convert-type base sp-sample-t))
    amount (* amount distance-ratio (+ 1 (sp-time-random-bounded (/ base 2)))))
  (if (or (> a nearest) (< a amount)) (return (+ a amount))
    (if (< a nearest) (return (- a amount))
      (if (bit-and 1 (sp-time-random)) (return (- a amount)) (return (+ a amount))))))

(define (sp-modulo-match index divisors divisor-count) (size-t size-t size-t* size-t)
  "return the index in divisors where partial_number modulo divisor is zero.
   returns the last divisors index if none were matched.
   for example, if divisors are 3 and 2 and index starts with 1 then every third partial will map to 0 and every second partial to 1.
   for selecting ever nth index"
  (for-each-index i size-t
    divisor-count (if (not (modulo index (array-get divisors i))) (return i)))
  (return (- divisor-count 1)))

(define (sp-time-expt base exp) (sp-time-t sp-time-t sp-time-t)
  (define a sp-time-t 1)
  (while 1
    (if (bit-and exp 1) (set* a base))
    (set exp (bit-shift-right exp 1))
    (if (not exp) break)
    (set* base base))
  (return a))

(define (sp-time-factorial n) (sp-time-t sp-time-t)
  (define result sp-time-t 1)
  (while (> n 0) (set* result n) (set- n 1))
  (return result))

(define (sp-set-sequence-max set-size selection-size) (sp-size-t sp-size-t sp-size-t)
  "return the maximum number of possible distinct selections from a set of length \"set-size\""
  (return (if* (= 0 set-size) 0 (sp-time-expt set-size selection-size))))

(define (sp-permutations-max set-size selection-size) (sp-size-t sp-size-t sp-size-t)
  (return (/ (sp-time-factorial set-size) (- set-size selection-size))))

(define (sp-compositions-max sum) (sp-size-t sp-size-t) (return (sp-time-expt 2 (- sum 1))))

(define (sp-scale-mask divisions) ((static inline sp-scale-t) sp-time-t)
  "return a mask with the lower divisions bits set"
  (return
    (if* (>= divisions (convert-type (* 8 (sizeof sp-scale-t)) sp-time-t))
      (bit-not (convert-type 0 sp-scale-t))
      (- (bit-shift-left (convert-type 1 sp-scale-t) divisions) 1))))

(define (sp-scale-make pitch-classes count divisions)
  ((static inline sp-scale-t) sp-time-t* sp-time-t sp-time-t)
  "build a scale bitset from an array of pitch-class indices"
  (define scale sp-scale-t 0)
  (sp-for-each-index i count
    (set scale
      (bit-or scale
        (bit-shift-left (convert-type 1 sp-scale-t) (modulo (array-get pitch-classes i) divisions)))))
  (return (bit-and scale (sp-scale-mask divisions))))

(define (sp-scale-rotate scale steps divisions)
  ((static inline sp-scale-t) sp-scale-t sp-time-t sp-time-t)
  "rotate (transpose) the scale by steps within divisions slots"
  (if (not divisions) (set divisions (convert-type (* 8 (sizeof sp-scale-t)) sp-time-t)))
  (set steps (modulo (+ (modulo steps divisions) divisions) divisions))
  (if (not steps) (return (bit-and scale (sp-scale-mask divisions))))
  (if (= divisions (* 8 (sizeof sp-scale-t)))
    (return
      (bit-or (bit-shift-left scale steps)
        (bit-shift-right scale (- (convert-type (* 8 (sizeof sp-scale-t)) sp-time-t) steps)))))
  (return
    (bit-and (bit-or (bit-shift-left scale steps) (bit-shift-right scale (- divisions steps)))
      (sp-scale-mask divisions))))

(define (sp-scale-divisions scale) ((static inline sp-time-t) sp-scale-t)
  "count how many notes (bits) are set in the scale"
  (define count sp-time-t 0)
  (while scale (set scale (bit-and scale (- scale 1))) (set+ count 1))
  (return count))

(define (sp-scale-first-index scale) ((static inline sp-time-t) sp-scale-t)
  "return index of the least-significant set bit (first note)"
  (define i sp-time-t 0)
  (while (not (bit-and scale 1)) (set scale (bit-shift-right scale 1)) (set+ i 1))
  (return i))

(define (sp-scale-canonical scale divisions) ((static inline sp-scale-t) sp-scale-t sp-time-t)
  "rotate so the first note sits at bit 0 (canonical form)"
  (return (sp-scale-rotate scale (* -1 (sp-scale-first-index scale)) divisions)))