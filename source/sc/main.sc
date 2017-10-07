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

(define (sin-lq a) (double double)
  "faster, lower precision version of sin()" (define b double (/ 4 M_PI))
  (define c double (/ -4 (* M_PI M_PI))) (return (- (+ (* b a) (* c a (abs a))))))

(define (sp-fft! result result-len source source-len) (status-t sp-sample-t* b32 sp-sample-t* b32)
  sp-status-init (local-memory-init 2)
  (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc result-len #f 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory)) (local-memory-add fftr-state)
  (sp-define-malloc out kiss-fft-cpx* (* result-len (sizeof kiss-fft-cpx))) (local-memory-add out)
  (kiss-fftr fftr-state source out)
  ; extract the real part
  (while result-len (dec result-len)
    (set (deref result result-len) (struct-get (array-get out result-len) r)))
  (label exit local-memory-free (return status)))

(define (sp-fft-inverse! result result-len source source-len)
  (status-t sp-sample-t* b32 sp-sample-t* b32) sp-status-init
  (local-memory-init 2) (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc source-len #t 0 0))
  (if (not fftr-state) (status-set-id-goto sp-status-id-memory)) (local-memory-add fftr-state)
  (sp-define-malloc in kiss-fft-cpx* (* source-len (sizeof kiss-fft-cpx))) (local-memory-add in)
  (while source-len (dec source-len)
    (struct-set (array-get in source-len) r (deref source (* source-len (sizeof sp-sample-t)))))
  (kiss-fftri fftr-state in result) (label exit local-memory-free (return status)))

(define (float-sum input len) (sp-sample-t sp-sample-t* b32)
  (define temp sp-sample-t element sp-sample-t) (define correction sp-sample-t 0)
  (dec len) (define result sp-sample-t (deref input len))
  (while len (dec len)
    (set element (deref input len)) (set temp (+ result element))
    (set correction
      (+ correction
        (if* (>= result element) (+ (- result temp) element) (+ (- element temp) result)))
      result temp))
  (return (+ correction result)))

(define
  (sp-moving-average! result source source-len prev prev-len next next-len distance start end)
  (boolean sp-sample-t* sp-sample-t* b32 sp-sample-t* b32 sp-sample-t* b32 b32 b32 b32)
  "apply a centered moving average filter to source at index start to end inclusively and write to result.
   removes higher frequencies with little distortion in the time domain.
   * only the result portion corresponding to the subvector from start to end is written to result
   * prev and next can be 0, for example for the beginning and end of a stream
   * since the result value for a sample is calculated from samples left and right from the sample,
     a previous and following part of a stream is eventually needed to reference values outside the source segment
     to create a valid continuous result. unavailable values outside the source segment are zero
   * values outside the start/end range are considered where needed to calculate averages
   * rounding errors are kept low by using modified kahan neumaier summation and not using a
     recursive implementation"
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

(define (sinc a) (double double) (return (if* (= 0 a) 1 (/ (sin a) a))))

(define (sp-blackman-window n) (sp-sample-t b32)
  (return
    (+ (- 0.42 (* 0.5 (cos (/ (* 2 M_PI n) (- n 1))))) (* 0.8 (cos (/ (* 4 M_PI n) (- n 1)))))))

(define (sp-sinc n cutoff) (sp-sample-t f32-s f32-s) (sinc (* 2 cutoff (- n (/ (- n 1) 2)))))

#;(define (sp-windowed-sinc! result result-len source source-len)
  (b0 sp-sample-t* b32 sp-sample-t* b32) #t)

;(define (sp-spectral-inversion))
;(define (sp-spectral-reversal))
;(define (sp-moving-average-high!))
;(define (sp-windowed-sinc-high))

(define (scm-sp-fft source) (SCM SCM)
  status-init (define result-len b32 (/ (* 3 (SCM-BYTEVECTOR-LENGTH source)) 2))
  (define result SCM (scm-make-f32vector (scm-from-uint32 result-len) (scm-from-uint8 0)))
  (status-require!
    (sp-fft! (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*) result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) (SCM-BYTEVECTOR-LENGTH source)))
  (label exit (status->scm-return result)))

(define (scm-sp-fft-inverse source) (SCM SCM)
  status-init (define result-len b32 (* (- (SCM-BYTEVECTOR-LENGTH source) 1) 2))
  (define result SCM (scm-make-f32vector (scm-from-uint32 result-len) (scm-from-uint8 0)))
  (status-require!
    (sp-fft-inverse! (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*) result-len
      (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) (SCM-BYTEVECTOR-LENGTH source)))
  (label exit (status->scm-return result)))

(pre-define (optional-samples a a-len scm)
  (if (scm-is-true scm)
    (set a (convert-type (SCM-BYTEVECTOR-CONTENTS scm) sp-sample-t*)
      a-len (octets->samples (SCM-BYTEVECTOR-LENGTH scm)))
    (set a 0 a-len 0)))

(pre-define (optional-index a default)
  (if* (and (not (scm-is-undefined start)) (scm-is-true start)) (scm->uint32 a) default))

(define (scm-sp-moving-average! result source scm-prev scm-next distance start end)
  (SCM SCM SCM SCM SCM SCM SCM SCM)
  (define source-len b32 (octets->samples (SCM-BYTEVECTOR-LENGTH source)))
  (define prev sp-sample-t* prev-len b32 next sp-sample-t* next-len b32)
  (optional-samples prev prev-len scm-prev) (optional-samples next next-len scm-next)
  (sp-moving-average! (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*)
    (convert-type (SCM-BYTEVECTOR-CONTENTS source) sp-sample-t*) source-len
    prev prev-len
    next next-len
    (scm->uint32 distance) (optional-index start 0) (optional-index end (- source-len 1)))
  (return SCM-UNSPECIFIED))

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
    f32vector f32vector f32vector f32vector integer integer integer [integer]"))
