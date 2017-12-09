(sc-comment
  "routines that take sample arrays as input and process them.
  depends on base.sc")

(sc-include "generic/transform.h")

(pre-include-once
  kiss-fft "kiss_fft.h"
  kiss-fftr "tools/kiss_fftr.h")

(define (sp-fft result result-len source source-len) (status-t sp-sample-t* b32 sp-sample-t* b32)
  sp-status-init
  (local-memory-init 2)
  (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc result-len #f 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory))
  (local-memory-add fftr-state)
  (sp-alloc-define out kiss-fft-cpx* (* result-len (sizeof kiss-fft-cpx)))
  (local-memory-add out)
  (kiss-fftr fftr-state source out)
  ; extract the real part
  (while result-len
    (dec result-len)
    (set (deref result result-len) (struct-get (array-get out result-len) r)))
  (label exit
    local-memory-free
    (return status)))

(define (sp-ifft result result-len source source-len) (status-t sp-sample-t* b32 sp-sample-t* b32)
  sp-status-init
  (local-memory-init 2)
  (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc source-len #t 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory))
  (local-memory-add fftr-state)
  (sp-alloc-define in kiss-fft-cpx* (* source-len (sizeof kiss-fft-cpx)))
  (local-memory-add in)
  (while source-len
    (dec source-len)
    (struct-set (array-get in source-len) r (deref source (* source-len (sizeof sp-sample-t)))))
  (kiss-fftri fftr-state in result)
  (label exit
    local-memory-free
    (return status)))

(define (sp-moving-average result source source-len prev prev-len next next-len start end radius)
  (status-i-t sp-sample-t* sp-sample-t* b32 sp-sample-t* b32 sp-sample-t* b32 b32 b32 b32)
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
  (if (not source-len) (return 1))
  (define
    left b32
    right b32)
  (define width b32 (+ 1 (* 2 radius)))
  (define window sp-sample-t* 0)
  (define window-index b32)
  (if (not (and (>= start radius) (<= (+ start radius 1) source-len)))
    ; not all required samples in source array
    (begin
      (set window (malloc (* width (sizeof sp-sample-t))))
      (if (not window) (return 1))))
  (while (<= start end)
    (if (and (>= start radius) (<= (+ start radius 1) source-len))
      ; all required samples are in source array
      (set (deref result) (/ (sp-sample-sum (- (+ source start) radius) width) width))
      (begin
        (set window-index 0)
        ; get samples from previous segment
        (if (< start radius)
          (begin
            (set right (- radius start))
            (if prev
              (begin
                (set left (if* (> right prev-len) 0 (- prev-len right)))
                (while (< left prev-len)
                  (set (deref window window-index) (deref prev left))
                  (inc window-index)
                  (inc left))))
            (while (< window-index right)
              (set (deref window window-index) 0)
              (inc window-index))
            (set left 0))
          (set left (- start radius)))
        ; get samples from source segment
        (set right (+ start radius))
        (if (>= right source-len) (set right (- source-len 1)))
        (while (<= left right)
          (set (deref window window-index) (deref source left))
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
              (set (deref window window-index) (deref next left))
              (inc window-index)
              (inc left))))
        ; fill unset values in window with zero
        (while (< window-index width)
          (set (deref window window-index) 0)
          (inc window-index))
        (set (deref result) (/ (sp-sample-sum window width) width))))
    (inc result)
    (inc start))
  (free window)
  (return 0))

(define (sp-window-blackman a width) (sp-float-t sp-float-t size-t)
  (return
    (+
      (- 0.42 (* 0.5 (cos (/ (* 2 M_PI a) (- width 1))))) (* 0.8 (cos (/ (* 4 M_PI a) (- width 1)))))))

(define (sp-spectral-inversion-ir a a-len) (b0 sp-sample-t* size-t)
  "modify an impulse response kernel for spectral inversion.
   a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response top to bottom"
  (while a-len
    (dec a-len)
    (set (deref a a-len) (* -1 (deref a a-len))))
  (define center size-t (/ (- a-len 1) 2))
  (inc (deref a center)))

(define (sp-spectral-reversal-ir a a-len) (b0 sp-sample-t* size-t)
  "inverts the sign for samples at odd indexes.
  a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response left to right"
  (while (> a-len 1)
    (set a-len (- a-len 2))
    (set (deref a a-len) (* -1 (deref a a-len)))))

(define (sp-convolve-one result a a-len b b-len)
  (b0 sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t)
  "discrete linear convolution.
  result length must be at least a-len + b-len - 1"
  (define a-index size-t 0)
  (define b-index size-t 0)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set (deref result (+ a-index b-index))
        (+ (deref result (+ a-index b-index)) (* (deref a a-index) (deref b b-index))))
      (inc b-index))
    (set b-index 0)
    (inc a-index)))

(define (sp-convolve result a a-len b b-len carryover carryover-len)
  (b0 sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t sp-sample-t* size-t)
  "discrete linear convolution for segments of a continuous stream. maps segments (a, a-len) to result.
  result length is a-len, carryover length is b-len or previous b-len. b-len must be greater than zero"
  ; algorithm: copy results that overlap from previous call from carryover, add results that fit completely in result,
  ;   add results that overlap with next segment to carryover.
  (memset result 0 (* a-len (sizeof sp-sample-t)))
  (memcpy result carryover (* carryover-len (sizeof sp-sample-t)))
  (memset carryover 0 (* b-len (sizeof sp-sample-t)))
  ;-- result values
  ; restrict processed range to exclude input values that generate carryover
  (define size size-t (if* (< a-len b-len) 0 (- a-len (- b-len 1))))
  (if size (sp-convolve-one result a size b b-len))
  ;-- carryover values
  (define a-index size-t size)
  (define b-index size-t 0)
  (define c-index size-t)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set c-index (+ a-index b-index))
      (if (< c-index a-len)
        (set (deref result c-index)
          (+ (deref result c-index) (* (deref a a-index) (deref b b-index))))
        (set
          c-index (- c-index a-len)
          (deref carryover c-index)
          (+ (deref carryover c-index) (* (deref a a-index) (deref b b-index)))))
      (inc b-index))
    (set b-index 0)
    (inc a-index)))

(sc-include "generic/transform/windowed-sinc")
