(pre-include
  "stdio.h"
  "fcntl.h"
  "alsa/asoundlib.h"
  "sndfile.h"
  "../main/sph-sp.h"
  "../foreign/sph/helper.c" "../foreign/sph/memreg.c" "./kiss_fft.h" "./tools/kiss_fftr.h")

(pre-define
  sp-status-declare (status-declare-group sp-status-group-sp)
  (sp-libc-status-require-id id) (if (< id 0) (status-set-both-goto sp-status-group-libc id))
  (sp-libc-status-require expression)
  (begin
    (set status.id expression)
    (if (< status.id 0) (status-set-group-goto sp-status-group-libc)
      status-reset))
  (sp-alsa-status-require expression)
  (begin
    (set status.id expression)
    (if status-is-failure (status-set-group-goto sp-status-group-alsa)))
  (define-sp-sine id sin)
  (begin
    "write samples for a sine wave into result-samples.
    sample-duration: seconds
    freq: radian frequency
    phase: phase offset
    amp: amplitude. 0..1
    used to define sp-sine, sp-sine-lq and similar"
    ; assumes that sin() returns sp-sample-t
    (define (id len sample-duration freq phase amp result-samples)
      (void sp-sample-count-t sp-float-t sp-float-t sp-float-t sp-float-t sp-sample-t*)
      (define index sp-sample-count-t 0)
      (while (<= index len)
        (set
          (array-get result-samples index) (* amp (sin (* freq phase sample-duration)))
          phase (+ 1 phase)
          index (+ 1 index)))))
  (optional-set-number a default)
  (begin
    "set default if number is negative"
    (if (> 0 a) (set a default)))
  (optional-number a default)
  (begin
    "default if number is negative"
    (if* (> 0 a) default
      a))
  (define-sp-interleave name type body)
  (begin
    "define a deinterleave, interleave or similar routine.
    a: deinterleaved
    b: interleaved"
    (define (name a b a-size channel-count) (void type** type* size-t sp-channel-count-t)
      (declare
        b-size size-t
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

(define (sp-status-description a) (uint8-t* status-t)
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
        (sp-status-id-port-type (set b "incompatible port type"))
        (else (set b ""))))
    ((not (strcmp sp-status-group-alsa a.group)) (set b (convert-type (snd-strerror a.id) char*)))
    ( (not (strcmp sp-status-group-sndfile a.group))
      (set b (convert-type (sf-error-number a.id) char*)))
    ((not (strcmp sp-status-group-sph a.group)) (set b (sph-helper-status-description a)))
    (else (set b "")))
  (return b))

(define (sp-status-name a) (uint8-t* status-t)
  (declare b char*)
  (cond
    ( (not (strcmp sp-status-group-alsa a.group))
      (case = a.id
        (sp-status-id-input-type (set b "input-type"))
        (sp-status-id-not-implemented (set b "not-implemented"))
        (sp-status-id-memory (set b "memory"))
        (else (set b "unknown"))))
    ((not (strcmp sp-status-group-alsa a.group)) (set b "alsa"))
    ((not (strcmp sp-status-group-sndfile a.group)) (set b "sndfile")) (else (set b "unknown"))))

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

(define (sp-sin-lq a) (sp-sample-t sp-sample-t)
  "lower precision version of sin() that could be faster"
  (declare
    b sp-sample-t
    c sp-sample-t)
  (set
    b (/ 4 M_PI)
    c (/ -4 (* M_PI M_PI)))
  (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-sinc a) (sp-sample-t sp-sample-t)
  "the normalised sinc function"
  (return
    (if* (= 0 a) 1
      (/ (sin (* M_PI a)) (* M_PI a)))))

(define (sp-fft len source source-len result-samples)
  (status-t sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-t*)
  "result-samples is owned and allocated by the caller.
  fast fourier transform"
  sp-status-declare
  (declare
    fftr-state kiss-fftr-cfg
    out kiss-fft-cpx*)
  (memreg-init 2)
  (set fftr-state (kiss-fftr-alloc len #f 0 0))
  (if (not fftr-state) (status-set-both-goto sp-status-group-sp sp-status-id-memory))
  (memreg-add fftr-state)
  (status-require (sph-helper-malloc (* len (sizeof kiss-fft-cpx)) &out))
  (memreg-add out)
  (kiss-fftr fftr-state source out)
  (sc-comment "extract the real part")
  (while len
    (set
      len (- len 1)
      (array-get result-samples len) (struct-get (array-get out len) r)))
  (label exit
    memreg-free
    (return status)))

(define (sp-ifft len source source-len result-samples)
  (status-t sp-sample-count-t sp-sample-t* sp-sample-count-t sp-sample-t*)
  sp-status-declare
  (declare
    fftr-state kiss-fftr-cfg
    in kiss-fft-cpx*)
  (memreg-init 2)
  (set fftr-state (kiss-fftr-alloc source-len #t 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory))
  (memreg-add fftr-state)
  (status-require (sph-helper-malloc (* source-len (sizeof kiss-fft-cpx)) &in))
  (memreg-add in)
  (while source-len
    (set source-len (- source-len 1))
    (struct-set (array-get in source-len)
      r (array-get source (* source-len (sizeof sp-sample-t)))))
  (kiss-fftri fftr-state in result-samples)
  (label exit
    memreg-free
    (return status)))

(define
  (sp-moving-average source source-len prev prev-len next next-len start end radius result-samples)
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
     outside the source segment to create a valid continuous result.
     zero is used for unavailable values outside the source segment
   * available values outside the start/end range are considered where needed to calculate averages
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
  (sc-comment "not all required samples in source array")
  (if (not (and (>= start radius) (<= (+ start radius 1) source-len)))
    (begin
      (status-require (sph-helper-malloc (* width (sizeof sp-sample-t)) &window))
      (memreg-add window)))
  (while (<= start end)
    (sc-comment "all required samples are in source array")
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
        (set *result-samples (/ (sp-sample-sum window width) width))))
    (set
      result-samples (+ 1 result-samples)
      start (+ 1 start)))
  (label exit
    memreg-free
    (return status)))

(define (sp-spectral-inversion-ir a a-len) (void sp-sample-t* size-t)
  "modify an impulse response kernel for spectral inversion.
   a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response top to bottom"
  (declare center size-t)
  (while a-len
    (set
      a-len (- a-len 1)
      (array-get a a-len) (* -1 (array-get a a-len))))
  (set
    center (/ (- a-len 1) 2)
    (array-get a center) (+ 1 (array-get a center))))

(define (sp-spectral-reversal-ir a a-len) (void sp-sample-t* size-t)
  "inverts the sign for samples at odd indexes.
  a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response left to right"
  (while (> a-len 1)
    (set
      a-len (- a-len 2)
      (array-get a a-len) (* -1 (array-get a a-len)))))

(define (sp-convolve-one a a-len b b-len result-samples)
  (void sp-sample-t* size-t sp-sample-t* size-t sp-sample-t*)
  "discrete linear convolution.
  result length must be at least a-len + b-len - 1.
  result-samples is owned and allocated by the caller"
  (declare
    a-index size-t
    b-index size-t)
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
  (void sp-sample-t* size-t sp-sample-t* size-t size-t sp-sample-t* sp-sample-t*)
  "discrete linear convolution for segments of a continuous stream. maps segments (a, a-len) to result.
  result length is a-len, carryover length is b-len or previous b-len. b-len must be greater than zero.
  algorithm: copy results that overlap from previous call from carryover, add results that fit completely in result,
  add results that overlap with next segment to carryover"
  (declare
    size size-t
    a-index size-t
    b-index size-t
    c-index size-t)
  (memset result-samples 0 (* a-len (sizeof sp-sample-t)))
  (memcpy result-samples result-carryover (* carryover-len (sizeof sp-sample-t)))
  (memset result-carryover 0 (* b-len (sizeof sp-sample-t)))
  (sc-comment
    "result values" "restrict processed range to exclude input values that generate carryover")
  (set size
    (if* (< a-len b-len) 0
      (- a-len (- b-len 1))))
  (if size (sp-convolve-one a size b b-len result-samples))
  (sc-comment "carryover values")
  (set
    a-index size
    b-index 0)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set c-index (+ a-index b-index))
      (if (< c-index a-len)
        (set (array-get result-samples c-index)
          (+ (array-get result-samples c-index) (* (array-get a a-index) (array-get b b-index))))
        (set
          c-index (- c-index a-len)
          (array-get result-carryover c-index)
          (+ (array-get result-carryover c-index) (* (array-get a a-index) (array-get b b-index)))))
      (set b-index (+ 1 b-index)))
    (set
      b-index 0
      a-index (+ 1 a-index))))

(define-sp-interleave
  sp-interleave
  sp-sample-t
  (compound-statement (set (array-get b b-size) (array-get (array-get a channel) a-size))))

(define-sp-interleave
  sp-deinterleave
  sp-sample-t
  (compound-statement (set (array-get (array-get a channel) a-size) (array-get b b-size))))

(define-sp-sine sp-sine sin)
(define-sp-sine sp-sine-lq sp-sin-lq)
(pre-include "../main/windowed-sinc.c" "../main/io.c")