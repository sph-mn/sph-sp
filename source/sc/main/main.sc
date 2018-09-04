(pre-include
  "stdio.h"
  "fcntl.h"
  "alsa/asoundlib.h"
  "sndfile.h"
  "./main/sph-sp.h"
  "./foreign/sph/one.c"
  "./foreign/sph/local-memory.c" "foreign/kiss_fft.h" "foreign/tools/kiss_fftr.h")

(pre-define
  sp-status-declare (status-declare-group sp-status-group-sp)
  (sp-system-status-require-id id) (if (< id 0) (status-set-both-goto sp-status-group-libc id))
  (sp-system-status-require expression)
  (begin
    (status-set-id expression)
    (if (< status.id 0) (status-set-group-goto sp-status-group-libc)
      status-reset))
  (sp-alsa-status-require expression)
  (begin
    (status-set-id expression)
    (if status-is-failure (status-set-group-goto sp-status-group-alsa)))
  (sp-status-require-alloc a)
  (if (not a) (status-set-both-goto sp-status-group-sp sp-status-id-memory)) (inc a)
  (set a (+ 1 a)) (dec a) (set a (- a 1)))

(enum
  (sp-status-group-alsa
    sp-status-group-libc
    sp-status-group-sndfile
    sp-status-group-sp
    sp-status-id-file-channel-mismatch
    sp-status-id-file-encoding
    sp-status-id-file-header
    sp-status-id-file-incompatible
    sp-status-id-file-incomplete
    sp-status-id-eof
    sp-status-id-input-type
    sp-status-id-memory
    sp-status-id-not-implemented
    sp-status-id-port-closed sp-status-id-port-position sp-status-id-port-type sp-status-id-undefined))

(define (sp-status-description a) (uint8-t* status-t)
  (return
    (case* = a.group
      (sp-status-group-sp
        (convert-type
          (case* = a.id
            (sp-status-id-eof "end of file")
            (sp-status-id-input-type "input argument is of wrong type")
            (sp-status-id-not-implemented "not implemented")
            (sp-status-id-memory "memory allocation error")
            (sp-status-id-file-incompatible
              "file channel count or sample rate is different from what was requested")
            (sp-status-id-file-incomplete "incomplete write")
            (sp-status-id-port-type "incompatible port type")
            (else ""))
          uint8-t*))
      (sp-status-group-alsa (convert-type (sf-error-number a.id) uint8-t*))
      (sp-status-group-sndfile (convert-type (sf-error-number a.id) uint8-t*))
      (else (convert-type "" uint8-t*)))))

(define (sp-status-name a) (uint8-t* status-t)
  (return
    (case* = a.group
      (sp-status-group-sp
        (convert-type
          (case* = a.id
            (sp-status-id-input-type "input-type")
            (sp-status-id-not-implemented "not-implemented")
            (sp-status-id-memory "memory")
            (else "unknown"))
          uint8-t*))
      (sp-status-group-alsa (convert-type "alsa" uint8-t*))
      (sp-status-group-sndfile (convert-type "sndfile" uint8-t*))
      (else (convert-type "unknown" uint8-t*)))))

(define (sp-alloc-channel-array channel-count sample-count) (sp-sample-t** uint32-t uint32-t)
  "return an array for channels with data arrays for each channel.
  returns zero if memory could not be allocated"
  (local-memory-init (+ channel-count 1))
  (declare
    channel sp-sample-t*
    result sp-sample-t**)
  (set result (malloc (* channel-count (sizeof sp-sample-t*))))
  (if (not result) (return 0))
  (local-memory-add result)
  (while channel-count
    (dec channel-count)
    (set channel (calloc (* sample-count (sizeof sp-sample-t)) 1))
    (if (not channel)
      (begin
        local-memory-free
        (return 0)))
    (local-memory-add channel)
    (set (array-get result channel-count) channel))
  (return result))

(define (sp-sin-lq a) (sp-sample-t sp-sample-t)
  "lower precision version of sin() that is faster to compute"
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

(sc-comment
  "write samples for a sine wave into dest.
   sample-duration: in seconds
   freq: radian frequency
   phase: phase offset
   amp: amplitude. 0..1
   defines sp-sine, sp-sine-lq")

(pre-define
  (define-sp-sine id sin)
  ; todo: assumes that sin() returns sp-sample-t
  (define (id dest len sample-duration freq phase amp)
    (void sp-sample-t* uint32-t sp-float-t sp-float-t sp-float-t sp-float-t)
    (define index uint32-t 0)
    (while (<= index len)
      (set (array-get dest index) (* amp (sin (* freq phase sample-duration))))
      (inc phase)
      (inc index))))

(define-sp-sine sp-sine sin)
(define-sp-sine sp-sine-lq sp-sin-lq)

(define (sp-fft result result-len source source-len) (status-t sp-sample-t* uint32-t sp-sample-t* uint32-t)
  sp-status-declare
  (declare fftr-state kiss-fftr-cfg)
  (local-memory-init 2)
  (set fftr-state (kiss-fftr-alloc result-len #f 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory))
  (local-memory-add fftr-state)
  (sp-alloc-define out kiss-fft-cpx* (* result-len (sizeof kiss-fft-cpx)))
  (local-memory-add out)
  (kiss-fftr fftr-state source out)
  ; extract the real part
  (while result-len
    (dec result-len)
    (set (array-get result result-len) (struct-get (array-get out result-len) r)))
  (label exit
    local-memory-free
    (return status)))

(define (sp-ifft result result-len source source-len) (status-t sp-sample-t* uint32-t sp-sample-t* uint32-t)
  sp-status-declare
  (declare fftr-state kiss-fftr-cfg)
  (local-memory-init 2)
  (set fftr-state (kiss-fftr-alloc source-len #t 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory))
  (local-memory-add fftr-state)
  (sp-alloc-define in kiss-fft-cpx* (* source-len (sizeof kiss-fft-cpx)))
  (local-memory-add in)
  (while source-len
    (dec source-len)
    (struct-set (array-get in source-len)
      r (array-get source (* source-len (sizeof sp-sample-t)))))
  (kiss-fftri fftr-state in result)
  (label exit
    local-memory-free
    (return status)))

(define (sp-moving-average result source source-len prev prev-len next next-len start end radius)
  (status-i-t sp-sample-t* sp-sample-t* uint32-t sp-sample-t* uint32-t sp-sample-t* uint32-t uint32-t uint32-t uint32-t)
  "apply a centered moving average filter to source at index start to end inclusively and write to result.
  removes higher frequencies with little distortion in the time domain.
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
  (declare
    left uint32-t
    right uint32-t
    width uint32-t
    window sp-sample-t*
    window-index uint32-t)
  (if (not source-len) (return 1))
  (set
    width (+ 1 (* 2 radius))
    window 0)
  (if (not (and (>= start radius) (<= (+ start radius 1) source-len)))
    ; not all required samples in source array
    (begin
      (set window (malloc (* width (sizeof sp-sample-t))))
      (if (not window) (return 1))))
  (while (<= start end)
    (if (and (>= start radius) (<= (+ start radius 1) source-len))
      ; all required samples are in source array
      (set *result (/ (sp-sample-sum (- (+ source start) radius) width) width))
      (begin
        (set window-index 0)
        ; get samples from previous segment
        (if (< start radius)
          (begin
            (set right (- radius start))
            (if prev
              (begin
                (set left
                  (if* (> right prev-len) 0
                    (- prev-len right)))
                (while (< left prev-len)
                  (set (array-get window window-index) (array-get prev left))
                  (inc window-index)
                  (inc left))))
            (while (< window-index right)
              (set (array-get window window-index) 0)
              (inc window-index))
            (set left 0))
          (set left (- start radius)))
        ; get samples from source segment
        (set right (+ start radius))
        (if (>= right source-len) (set right (- source-len 1)))
        (while (<= left right)
          (set (array-get window window-index) (array-get source left))
          (inc window-index)
          (inc left))
        ; get samples from next segment
        (set right (+ start radius))
        (if (and (>= right source-len) next)
          (begin
            (set
              left 0
              right (- right source-len))
            (if (>= right next-len) (set right (- next-len 1)))
            (while (<= left right)
              (set (array-get window window-index) (array-get next left))
              (inc window-index)
              (inc left))))
        ; fill unset values in window with zero
        (while (< window-index width)
          (set (array-get window window-index) 0)
          (inc window-index))
        (set *result (/ (sp-sample-sum window width) width))))
    (inc result)
    (inc start))
  (free window)
  (return 0))

(define (sp-window-blackman a width) (sp-float-t sp-float-t size-t)
  (return
    (+
      (- 0.42 (* 0.5 (cos (/ (* 2 M_PI a) (- width 1))))) (* 0.8 (cos (/ (* 4 M_PI a) (- width 1)))))))

(define (sp-spectral-inversion-ir a a-len) (void sp-sample-t* size-t)
  "modify an impulse response kernel for spectral inversion.
   a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response top to bottom"
  (declare center size-t)
  (while a-len
    (dec a-len)
    (set (array-get a a-len) (* -1 (array-get a a-len))))
  (set center (/ (- a-len 1) 2))
  (inc (array-get a center)))

(define (sp-spectral-reversal-ir a a-len) (void sp-sample-t* size-t)
  "inverts the sign for samples at odd indexes.
  a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response left to right"
  (while (> a-len 1)
    (set a-len (- a-len 2))
    (set (array-get a a-len) (* -1 (array-get a a-len)))))

(define (sp-convolve-one result a a-len b b-len)
  (void sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t)
  "discrete linear convolution.
  result length must be at least a-len + b-len - 1"
  (declare
    a-index size-t
    b-index size-t)
  (set
    a-index 0
    b-index 0)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set (array-get result (+ a-index b-index))
        (+ (array-get result (+ a-index b-index)) (* (array-get a a-index) (array-get b b-index))))
      (inc b-index))
    (set b-index 0)
    (inc a-index)))

(define (sp-convolve result a a-len b b-len carryover carryover-len)
  (void sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t sp-sample-t* size-t)
  "discrete linear convolution for segments of a continuous stream. maps segments (a, a-len) to result.
  result length is a-len, carryover length is b-len or previous b-len. b-len must be greater than zero"
  ; algorithm: copy results that overlap from previous call from carryover, add results that fit completely in result,
  ; add results that overlap with next segment to carryover.
  (declare
    size size-t
    a-index size-t
    b-index size-t
    c-index size-t)
  (memset result 0 (* a-len (sizeof sp-sample-t)))
  (memcpy result carryover (* carryover-len (sizeof sp-sample-t)))
  (memset carryover 0 (* b-len (sizeof sp-sample-t)))
  ;-- result values
  ; restrict processed range to exclude input values that generate carryover
  (set size
    (if* (< a-len b-len) 0
      (- a-len (- b-len 1))))
  (if size (sp-convolve-one result a size b b-len))
  ;-- carryover values
  (set
    a-index size
    b-index 0)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set c-index (+ a-index b-index))
      (if (< c-index a-len)
        (set (array-get result c-index)
          (+ (array-get result c-index) (* (array-get a a-index) (array-get b b-index))))
        (set
          c-index (- c-index a-len)
          (array-get carryover c-index)
          (+ (array-get carryover c-index) (* (array-get a a-index) (array-get b b-index)))))
      (inc b-index))
    (set b-index 0)
    (inc a-index)))

(pre-include "./main/windowed-sinc.c" "./main/io.sc")