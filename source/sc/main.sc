(sc-include-once sph "foreign/sph" sp-config "config")
(pre-define kiss-fft-scalar sp-sample-t)

(pre-include-once stdio-h "stdio.h"
  libguile-h "libguile.h"
  kiss-fft-h "foreign/kissfft/kiss_fft.h"
  kiss-fftr-h "foreign/kissfft/tools/kiss_fftr.h" fcntl-h "fcntl.h" asoundlib-h "alsa/asoundlib.h")

(pre-define (sp-define-malloc id type size) (define id type (malloc size))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-set-malloc id size) (set id (malloc size))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-define-calloc id type size) (define id type (calloc size 1))
  (if (not id) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (inc a) (set a (+ 1 a)))
(pre-define (dec a) (set a (- a 1)))
(pre-define (octets->samples a) (/ a (sizeof sp-sample-t)))
(pre-define (samples->octets a) (* a (sizeof sp-sample-t)))

(sc-include-once sph-one "foreign/sph/one"
  guile "foreign/sph/guile"
  sph-status "foreign/sph/status"
  sph-local-memory "foreign/sph/local-memory" sp-status "status" sp-io "io")

(pre-define (optional-samples a a-len scm)
  (if (scm-is-true scm)
    (set a (convert-type (SCM-BYTEVECTOR-CONTENTS scm) sp-sample-t*)
      a-len (octets->samples (SCM-BYTEVECTOR-LENGTH scm)))
    (set a 0 a-len 0)))

(pre-define (optional-index a default)
  (if* (and (not (scm-is-undefined start)) (scm-is-true start)) (scm->uint32 a) default))

(define (sin-lq a) (f32-s f32-s)
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

(define (sp-fft-inverse result result-len source source-len)
  (status-t sp-sample-t* b32 sp-sample-t* b32) sp-status-init
  (local-memory-init 2) (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc source-len #t 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory)) (local-memory-add fftr-state)
  (sp-define-malloc in kiss-fft-cpx* (* source-len (sizeof kiss-fft-cpx))) (local-memory-add in)
  (while source-len (dec source-len)
    (struct-set (array-get in source-len) r (deref source (* source-len (sizeof sp-sample-t)))))
  (kiss-fftri fftr-state in result) (label exit local-memory-free (return status)))

(define (float-sum numbers len) (f32-s f32-s* b32)
  "sum numbers with rounding error compensation using kahan summation with neumaier modification"
  (define temp f32-s element f32-s) (define correction f32-s 0)
  (dec len) (define result f32-s (deref numbers len))
  (while len (dec len)
    (set element (deref numbers len)) (set temp (+ result element))
    (set correction
      (+ correction
        (if* (>= result element) (+ (- result temp) element) (+ (- element temp) result)))
      result temp))
  (return (+ correction result)))

(define (float-nearly-equal? a b margin) (boolean f32-s f32-s f32-s)
  "approximate float comparison. margin is a factor and is low for low accepted differences.
   http://floating-point-gui.de/errors/comparison/"
  (if (= a b) (return #t)
    (begin (define diff f32-s (fabs (- a b)))
      (return
        (if* (or (= 0 a) (= 0 b) (< diff DBL_MIN)) (< diff (* margin DBL_MIN))
          (< (/ diff (fmin (+ (fabs a) (fabs b)) DBL_MAX)) margin))))))

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

(define (sinc a) (f32-s f32-s)
  "the normalised sinc function" (return (if* (= 0 a) 1 (/ (sin (* M_PI a)) (* M_PI a)))))

(define (sp-blackman a width) (f32-s f32-s size-t)
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
  (while a-len (set a-len (- a-len 2)) (set (deref a a-len) (* -1 (deref a a-len)))))

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

(define (sp-convolve result a a-len b b-len state state-len)
  (b0 sp-sample-t* sp-sample-t* size-t sp-sample-t* size-t sp-sample-t* size-t*)
  "discrete linear convolution for segments of a continuous stream.
  result length is a-len.
  state length is b-len"
  ; previous values
  (define size size-t (deref state-len)) (if (not (= size b-len)) (set (deref state-len) b-len))
  (while size (dec size) (set (deref result size) (deref state size)))
  ; result values
  (set size (if* (> a-len b-len) a-len (- a-len b-len))) (sp-convolve-one result a size b b-len)
  ; next values
  (define a-index size-t size) (define b-index size-t 0)
  (while (< a-index a-len)
    (while (< b-index b-len) (set size (+ a-index b-index))
      (if (>= size a-len)
        (set size (- a-len (+ a-index b-index))
          (deref state size) (+ (deref state size) (* (deref a a-index) (deref b b-index))))
        (set (deref result size) (+ (deref result size) (* (deref a a-index) (deref b b-index)))))
      (inc b-index))
    (set b-index 0) (inc a-index)))

(define (scm-sp-convolve! result a b state) (SCM SCM SCM SCM SCM)
  "state: (size . data)" (define a-len b32 (octets->samples (SCM-BYTEVECTOR-LENGTH a)))
  (define b-len b32 (octets->samples (SCM-BYTEVECTOR-LENGTH b)))
  (define scm-state-len SCM (scm-first state)) (define scm-state-data SCM (scm-tail state))
  (define state-len size-t (scm->size-t scm-state-len))
  (sp-convolve (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS a) sp-sample-t*) a-len
    (convert-type (SCM-BYTEVECTOR-CONTENTS b) sp-sample-t*) b-len
    (convert-type (SCM-BYTEVECTOR-CONTENTS scm-state-data) sp-sample-t*) (address-of state-len))
  (return (if* (= b-len state-len) state (scm-cons (scm-from-size-t state-len) scm-state-data))))

(define (scm-sp-fft source) (SCM SCM)
  status-init (define result-len b32 (/ (* 3 (SCM-BYTEVECTOR-LENGTH source)) 2))
  (define result SCM (scm-make-f32vector (scm-from-uint32 result-len) (scm-from-uint8 0)))
  (status-require!
    (sp-fft (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*) result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) (SCM-BYTEVECTOR-LENGTH source)))
  (label exit (status->scm-return result)))

(define (scm-sp-fft-inverse source) (SCM SCM)
  status-init (define result-len b32 (* (- (SCM-BYTEVECTOR-LENGTH source) 1) 2))
  (define result SCM (scm-make-f32vector (scm-from-uint32 result-len) (scm-from-uint8 0)))
  (status-require!
    (sp-fft-inverse (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*) result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) (SCM-BYTEVECTOR-LENGTH source)))
  (label exit (status->scm-return result)))

(define (scm-sp-moving-average! result source scm-prev scm-next distance start end)
  (SCM SCM SCM SCM SCM SCM SCM SCM)
  (define source-len b32 (octets->samples (SCM-BYTEVECTOR-LENGTH source)))
  (define prev sp-sample-t* prev-len b32 next sp-sample-t* next-len b32)
  (optional-samples prev prev-len scm-prev) (optional-samples next next-len scm-next)
  (sp-moving-average (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) source-len
    prev prev-len
    next next-len
    (optional-index start 0) (optional-index end (- source-len 1)) (scm->uint32 distance))
  (return SCM-UNSPECIFIED))

(define (scm-float-nearly-equal? a b margin) (SCM SCM SCM SCM)
  (return
    (scm-from-bool (float-nearly-equal? (scm->double a) (scm->double b) (scm->double margin)))))

(define (scm-f32vector-sum a start end) (SCM SCM SCM SCM)
  (return
    (scm-from-double
      (float-sum
        (+ (if* (scm-is-undefined start) 0 (scm->uint32 start))
          (convert-type (SCM-BYTEVECTOR-CONTENTS a) f32-s*))
        (* (if* (scm-is-undefined end) (SCM-BYTEVECTOR-LENGTH a) (- end (+ 1 start)))
          (sizeof f32-s))))))

(sc-comment
  "write samples for a sine wave into data between start at end.
  also defines scm-sp-sine!, scm-sp-sine-lq!")

(pre-define (define-sp-sine! id sin)
  (define (id data start end sample-duration freq phase amp)
    (b0 sp-sample-t* b32 b32 f32_s f32_s f32_s f32_s)
    (while (<= start end) (set (deref data start) (* amp (sin (* freq phase sample-duration))))
      (inc phase) (inc start)))
  (define ((pre-concat scm_ id) data start end sample-duration freq phase amp)
    (SCM SCM SCM SCM SCM SCM SCM SCM)
    (id (convert-type (SCM-BYTEVECTOR-CONTENTS data) sp-sample-t*) (scm->uint32 start)
      (scm->uint32 end) (scm->double sample-duration)
      (scm->double freq) (scm->double phase) (scm->double amp))
    (return SCM-UNSPECIFIED)))

(define-sp-sine! sp-sine! sin)
(define-sp-sine! sp-sine-lq! sin-lq)
(include-sc "windowed-sinc")

(define (init-sp) b0
  sp-port-scm-type-init (define scm-module SCM (scm-c-resolve-module "sph sp"))
  (set scm-sp-port-type-alsa (scm-from-latin1-symbol "alsa")
    scm-sp-port-type-file (scm-from-latin1-symbol "file")
    scm-rnrs-raise (scm-c-public-ref "rnrs exceptions" "raise"))
  (scm-c-module-define scm-module "sp-port-type-alsa" scm-sp-port-type-alsa)
  (scm-c-module-define scm-module "sp-port-type-file" scm-sp-port-type-file)
  scm-c-define-procedure-c-init
  (scm-c-define-procedure-c "sp-port-close" 1 0 0 scm-sp-port-close "sp-port -> boolean")
  (scm-c-define-procedure-c "sp-port-input?" 1 0 0 scm-sp-port-input? "sp-port -> boolean")
  (scm-c-define-procedure-c "sp-port-position?" 1 0 0 scm-sp-port-position? "sp-port -> boolean")
  (scm-c-define-procedure-c "sp-port-position" 1
    0 0 scm-sp-port-position "sp-port -> integer/boolean")
  (scm-c-define-procedure-c "sp-port-channel-count" 1
    0 0 scm-sp-port-channel-count "sp-port -> integer")
  (scm-c-define-procedure-c "sp-port-sample-rate" 1
    0 0 scm-sp-port-sample-rate "sp-port -> integer/boolean")
  (scm-c-define-procedure-c "sp-port?" 1 0 0 scm-sp-port? "sp-port -> boolean")
  (scm-c-define-procedure-c "sp-port-type" 1 0 0 scm-sp-port-type "sp-port -> integer")
  (scm-c-define-procedure-c "sp-file-open-input" 1
    2 0 scm-sp-file-open-input
    "string -> sp-port
    path -> sp-port")
  (scm-c-define-procedure-c "sp-file-open-output" 1
    2 0
    scm-sp-file-open-output
    "string [integer integer] -> sp-port
    path [channel-count sample-rate] -> sp-port")
  (scm-c-define-procedure-c "sp-file-write" 2
    1 0
    scm-sp-file-write
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean
    write sample data to the channels of a file port")
  (scm-c-define-procedure-c "sp-file-set-position" 2
    0 0
    scm-sp-file-set-position
    "sp-port integer:sample-offset -> boolean
    sample-offset can be negative, in which case it is from the end of the file")
  (scm-c-define-procedure-c "sp-file-read" 2
    0 0 scm-sp-file-read "sp-port integer:sample-count -> (f32vector ...):channel-data")
  (scm-c-define-procedure-c "sp-alsa-open-input" 0
    4 0
    scm-sp-alsa-open-input
    "[string integer integer integer] -> sp-port
    [device-name channel-count sample-rate latency] -> sp-port")
  (scm-c-define-procedure-c "sp-alsa-open-output" 0
    4 0
    scm-sp-alsa-open-output
    "[string integer integer integer] -> sp-port
    [device-name channel-count sample-rate latency] -> sp-port")
  (scm-c-define-procedure-c "sp-alsa-write" 2
    1 0
    scm-sp-alsa-write
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean
    write sample data to the channels of an alsa port - to the sound card for sound output for example")
  (scm-c-define-procedure-c "sp-alsa-read" 2
    0 0 scm-sp-alsa-read
    "port sample-count -> channel-data
    sp-port integer -> (f32vector ...)")
  (scm-c-define-procedure-c "sp-fft" 1
    0 0
    scm-sp-fft
    "f32vector:value-per-time -> f32vector:frequencies-per-time
    discrete fourier transform on the input data")
  (scm-c-define-procedure-c "sp-fft-inverse" 1
    0 0
    scm-sp-fft-inverse
    "f32vector:frequencies-per-time -> f32vector:value-per-time
    inverse discrete fourier transform on the input data")
  (scm-c-define-procedure-c "sp-sine!" 7
    0 0
    scm-sp-sine!
    "data start end sample-duration freq phase amp -> unspecified
    f32vector integer integer rational rational rational rational")
  (scm-c-define-procedure-c "sp-sine-lq!" 7
    0 0
    scm-sp-sine-lq!
    "data start end sample-duration freq phase amp -> unspecified
    f32vector integer integer rational rational rational rational
    faster, lower precision version of sp-sine!.
    currently faster by a factor of about 2.6")
  (scm-c-define-procedure-c "sp-moving-average!" 5
    2 0
    scm-sp-moving-average!
    "result source previous next distance [start end] -> unspecified
  f32vector f32vector f32vector f32vector integer integer integer [integer]")
  (scm-c-define-procedure-c "sp-windowed-sinc!" 7
    2 0
    scm-sp-windowed-sinc!
    "result source previous next sample-rate freq transition [start end] -> unspecified
    f32vector f32vector f32vector f32vector number number integer integer -> boolean")
  (scm-c-define-procedure-c "f32vector-sum" 1
    2 0 scm-f32vector-sum "f32vector [start end] -> number")
  (scm-c-define-procedure-c "float-nearly-equal?" 3
    0 0 scm-float-nearly-equal?
    "a b margin -> boolean
    number number number -> boolean")
  (scm-c-define-procedure-c "sp-convolve!" 3
    0 0 scm-sp-convolve! "a b state:(integer . f32vector) -> state"))
