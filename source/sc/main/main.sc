(sc-include "sph-sp")
(pre-define kiss-fft-scalar sp-sample-t)

(pre-include-once stdio-h "stdio.h"
  kiss-fft-h "foreign/kissfft/kiss_fft.h"
  kiss-fftr-h "foreign/kissfft/tools/kiss_fftr.h" fcntl-h "fcntl.h")

(sc-include "foreign/sph/one" "foreign/sph/local-memory")

(pre-define (sp-define-malloc id type size) (define id type (malloc size))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-set-malloc id size) (set id (malloc size))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-define-calloc id type size) (define id type (calloc size 1))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-set-calloc id size) (set id (calloc size 1))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (inc a) (set a (+ 1 a)))
(pre-define (dec a) (set a (- a 1)))

(define (sp-alloc-channel-data channel-count sample-count) (sp-sample-t** b32 b32)
  "zero if memory could not be allocated" (local-memory-init (+ channel-count 1))
  (define result sp-sample-t** (malloc (* channel-count (sizeof sp-sample-t*))))
  (if (not result) (return 0)) (local-memory-add result)
  (define channel sp-sample-t*)
  (while channel-count (dec channel-count)
    (set channel (calloc (* sample-count (sizeof sp-sample-t)) 1)) (local-memory-add channel)
    (if (not channel) (begin local-memory-free (return 0)))
    (set (deref result channel-count) channel))
  (return result))

(define (sp-sin-lq a) (f32-s f32-s)
  "lower precision version of sin() that is faster to compute" (define b f32-s (/ 4 M_PI))
  (define c f32-s (/ -4 (* M_PI M_PI))) (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-fft result result-len source source-len) (status-t sp-sample-t* b32 sp-sample-t* b32)
  sp-status-init (local-memory-init 2)
  (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc result-len #f 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory)) (local-memory-add fftr-state)
  (sp-define-malloc out kiss-fft-cpx* (* result-len (sizeof kiss-fft-cpx))) (local-memory-add out)
  (kiss-fftr fftr-state source out)
  ; extract the real part
  (while result-len (dec result-len)
    (set (deref result result-len) (struct-get (array-get out result-len) r)))
  (label exit local-memory-free (return status)))

(define (sp-ifft result result-len source source-len) (status-t sp-sample-t* b32 sp-sample-t* b32)
  sp-status-init (local-memory-init 2)
  (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc source-len #t 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory)) (local-memory-add fftr-state)
  (sp-define-malloc in kiss-fft-cpx* (* source-len (sizeof kiss-fft-cpx))) (local-memory-add in)
  (while source-len (dec source-len)
    (struct-set (array-get in source-len) r (deref source (* source-len (sizeof sp-sample-t)))))
  (kiss-fftri fftr-state in result) (label exit local-memory-free (return status)))

(define (sp-moving-average result source source-len prev prev-len next next-len start end distance)
  (boolean sp-sample-t* sp-sample-t* b32 sp-sample-t* b32 sp-sample-t* b32 b32 b32 b32)
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
  (if (not source-len) (return 1)) (define left b32 right b32)
  (define width b32 (+ 1 (* 2 distance))) (define window sp-sample-t* 0)
  (define window-index b32)
  (if (not (and (>= start distance) (<= (+ start distance 1) source-len)))
    (begin (set window (malloc (* width (sizeof sp-sample-t)))) (if (not window) (return 1))))
  (while (<= start end)
    (if (and (>= start distance) (<= (+ start distance 1) source-len))
      (set (deref result) (/ (float-sum (- (+ source start) distance) width) width))
      (begin (set window-index 0)
        ; prev
        (if (< start distance)
          (begin (set right (- distance start))
            (if prev
              (begin (set left (if* (> right prev-len) 0 (- prev-len right)))
                (while (< left prev-len) (set (deref window window-index) (deref prev left))
                  (inc window-index) (inc left))))
            (while (< window-index right) (set (deref window window-index) 0) (inc window-index))
            (set left 0))
          (set left (- start distance)))
        ; source
        (set right (+ start distance)) (if (>= right source-len) (set right (- source-len 1)))
        (while (<= left right) (set (deref window window-index) (deref source left))
          (inc window-index) (inc left))
        ; next
        (set right (+ start distance))
        (if (and (>= right source-len) next)
          (begin (set left 0 right (- right source-len))
            (if (>= right next-len) (set right (- next-len 1)))
            (while (<= left right) (set (deref window window-index) (deref next left))
              (inc window-index) (inc left))))
        (while (< window-index width) (set (deref window window-index) 0) (inc window-index))
        (set (deref result) (/ (float-sum window width) width))))
    (inc result) (inc start))
  (free window) (return 0))

(define (sp-sinc a) (f32-s f32-s)
  "the normalised sinc function" (return (if* (= 0 a) 1 (/ (sin (* M_PI a)) (* M_PI a)))))

(define (sp-window-blackman a width) (f32-s f32-s size-t)
  (return
    (+ (- 0.42 (* 0.5 (cos (/ (* 2 M_PI a) (- width 1)))))
      (* 0.8 (cos (/ (* 4 M_PI a) (- width 1)))))))

(define (sp-spectral-inversion-ir a a-len) (b0 sp-sample-t* size-t)
  "modify an impulse response kernel for spectral inversion.
   a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response top to bottom"
  (while a-len (dec a-len) (set (deref a a-len) (* -1 (deref a a-len))))
  (define center size-t (/ (- a-len 1) 2)) (inc (deref a center)))

(define (sp-spectral-reversal-ir a a-len) (b0 sp-sample-t* size-t)
  "inverts the sign for samples at odd indexes.
  a-len must be odd and \"a\" must have left-right symmetry.
  flips the frequency response left to right"
  (while (> a-len 1) (set a-len (- a-len 2)) (set (deref a a-len) (* -1 (deref a a-len)))))

(define (sp-convolve-one result a a-len b b-len)
  (b0 sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t)
  "discrete linear convolution.
  result length must be at least a-len + b-len - 1"
  (define a-index size-t 0) (define b-index size-t 0)
  (while (< a-index a-len)
    (while (< b-index b-len)
      (set (deref result (+ a-index b-index))
        (+ (deref result (+ a-index b-index)) (* (deref a a-index) (deref b b-index))))
      (inc b-index))
    (set b-index 0) (inc a-index)))

(define (sp-convolve result a a-len b b-len carryover carryover-len)
  (b0 sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t sp-sample-t* size-t)
  "discrete linear convolution for segments of a continuous stream.
  result length is a-len.
  carryover length is b-len or previous b-len"
  ; algorithm: copy results that overlap from previous call from carryover, add results that fit completely in result,
  ;   add results that overlap with next segment to carryover.
  ; previous values. carryover-len should differ if b-len changed between calls
  (while carryover-len (dec carryover-len)
    (set (deref result carryover-len) (deref carryover carryover-len)))
  ; result values
  (define size size-t) (set size (if* (> a-len b-len) a-len (- a-len b-len)))
  (sp-convolve-one result a size b b-len)
  ; next values
  (define a-index size-t size) (define b-index size-t 0)
  (while (< a-index a-len)
    (while (< b-index b-len) (set size (+ a-index b-index))
      (if (>= size a-len)
        (set size (- a-len (+ a-index b-index))
          (deref carryover size) (+ (deref carryover size) (* (deref a a-index) (deref b b-index))))
        (set (deref result size) (+ (deref result size) (* (deref a a-index) (deref b b-index)))))
      (inc b-index))
    (set b-index 0) (inc a-index)))

(sc-comment
  "write samples for a sine wave into data between start at end.
 defines sp-sine, sp-sine-lq")

(pre-define (define-sp-sine id sin)
  (define (id data start end sample-duration freq phase amp)
    (b0 sp-sample-t* b32 b32 f32_s f32_s f32_s f32_s)
    (while (<= start end) (set (deref data start) (* amp (sin (* freq phase sample-duration))))
      (inc phase) (inc start))))

(define-sp-sine sp-sine sin)
(define-sp-sine sp-sine-lq sp-sin-lq)
(sc-include "windowed-sinc" "io")
