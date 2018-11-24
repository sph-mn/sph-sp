(pre-include
  "stdio.h"
  "fcntl.h"
  "alsa/asoundlib.h"
  "sndfile.h"
  "../main/sph-sp.h"
  "../foreign/sph/float.c"
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

(define (sp-sin-lq a) (sp-sample-t sp-sample-t)
  "lower precision version of sin() that could be faster"
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

(define (sp-fftr input input-len output) (status-t sp-sample-t* sp-sample-count-t sp-sample-t*)
  "real-numbers -> [[real, imaginary] ...]:complex-numbers
  write to output real and imaginary part alternatingly
  output-len will be set to the count of complex numbers, (+ 1 (/ input-len 2)).
  output is allocated and owned by the caller"
  sp-status-declare
  (declare
    cfg kiss-fftr-cfg
    out kiss-fft-cpx*
    out-len sp-sample-count-t
    i sp-sample-count-t)
  (memreg-init 2)
  (set cfg (kiss-fftr-alloc input-len #f 0 0))
  (if (not cfg) (status-set-both-goto sp-status-group-sp sp-status-id-memory))
  (memreg-add cfg)
  ; out must be input-len or it segfaults but only the first half of the result will be non-zero
  (status-require (sph-helper-calloc (* input-len (sizeof kiss-fft-cpx)) &out))
  (memreg-add out)
  (kiss-fftr cfg input out)
  (set out-len (sp-fftr-output-len input-len))
  (for ((set i 0) (< i out-len) (set i (+ 1 i)))
    (set
      (array-get output (* 2 i)) (struct-get (array-get out i) r)
      (array-get output (+ 1 (* 2 i))) (struct-get (array-get out i) i)))
  (label exit
    memreg-free
    (return status)))

(define (sp-fftri input input-len output) (status-t sp-sample-t* sp-sample-count-t sp-sample-t*)
  "[[real, imaginary], ...]:complex-numbers -> real-numbers
  input-length > 0"
  sp-status-declare
  (declare
    cfg kiss-fftr-cfg
    in kiss-fft-cpx*
    i sp-sample-count-t)
  (memreg-init 2)
  (set cfg (kiss-fftr-alloc input-len #t 0 0))
  (if (not cfg) (status-set-id-goto sp-status-id-memory))
  (memreg-add cfg)
  (status-require (sph-helper-malloc (* input-len (sizeof kiss-fft-cpx)) &in))
  (memreg-add in)
  (for ((set i 0) (< i input-len) (set i (+ 1 i)))
    (set
      (struct-get (array-get in i) r) (array-get input (* 2 i))
      (struct-get (array-get in i) i) (array-get input (+ 1 (* 2 i)))))
  (kiss-fftri cfg in output)
  (label exit
    memreg-free
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
  (sc-comment "not all required samples in source array")
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
  result length must be at least a-len + b-len - 1.
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
  "discrete linear convolution for segments of a continuous stream. maps segments (a, a-len) to result-samples
  using (b, b-len) as the impulse response. b-len must be greater than zero.
  result-samples length is a-len.
  carryover length must at least b-len.
  carryover-len should be zero for the first call, b-len or if b-len changed b-len from the previous call.
  all heap memory is owned and allocated by the caller"
  (declare
    size sp-sample-count-t
    a-index sp-sample-count-t
    b-index sp-sample-count-t
    c-index sp-sample-count-t)
  (memset result-samples 0 (* a-len (sizeof sp-sample-t)))
  (if carryover-len (memcpy result-samples result-carryover (* carryover-len (sizeof sp-sample-t))))
  (memset result-carryover 0 (* b-len (sizeof sp-sample-t)))
  (sc-comment
    "result values." "restrict processed range to exclude input values that generate carryover")
  (set size
    (if* (< a-len b-len) 0
      (- a-len (- b-len 1))))
  (if size (sp-convolve-one a size b b-len result-samples))
  (sc-comment "carryover values")
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

(define-sp-sine sp-sine sin)
(define-sp-sine sp-sine-lq sp-sin-lq)
(pre-include "../main/windowed-sinc.c" "../main/io.c")