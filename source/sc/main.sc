(sc-include-once sph "foreign/sph")

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

(sc-include-once sph-one "foreign/sph/one"
  guile "foreign/sph/guile"
  sph-status "foreign/sph/status"
  sph-local-memory "foreign/sph/local-memory" sp-config "config" sp-status "status" sp-io "io")

(define (scm-sp-fft a) (SCM SCM)
  status-init (define size b32 (/ (SCM-BYTEVECTOR-LENGTH a) 4))
  (define size-result b32 (+ 1 (* size 0.5))) (local-memory-init 2)
  (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc size 0 0 0)) (local-memory-add fftr-state)
  (sp-define-malloc out kiss-fft-cpx* (* size-result (sizeof kiss-fft-cpx))) (local-memory-add out)
  (kiss-fftr fftr-state (convert-type (SCM-BYTEVECTOR-CONTENTS a) sp-sample-t*) out)
  (define result SCM (scm-make-f32vector (scm-from-uint32 size-result) (scm-from-uint8 0)))
  (while size-result (decrement size-result)
    (set (deref (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*) size-result)
      (struct-get (array-get out size-result) r)))
  (label exit local-memory-free (status->scm-return result)))

(define (scm-sp-fft-inverse a) (SCM SCM)
  status-init (define size b32 (/ (SCM-BYTEVECTOR-LENGTH a) 4))
  (define size-result b32 (* (- size 1) 2)) (local-memory-init 2)
  (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc size-result 1 0 0)) (local-memory-add fftr-state)
  (sp-define-malloc in kiss-fft-cpx* (* size (sizeof kiss-fft-cpx))) (local-memory-add in)
  (while size (decrement size)
    (struct-set (array-get in size) r (deref (SCM-BYTEVECTOR-CONTENTS a) (* size 4))))
  (define result SCM (scm-make-f32vector (scm-from-uint32 size-result) (scm-from-uint8 0)))
  (kiss-fftri fftr-state in (convert-type (SCM-BYTEVECTOR-CONTENTS result) sp-sample-t*))
  (label exit local-memory-free (status->scm-return result)))

(pre-define (octets->samples a) (/ a (sizeof sample-t)))
(pre-define (samples->octets a) (* a (sizeof sample-t)))

#;(define (sp-moving-average! data data-next start end width state) (b0 sample-t* sample-t* b32 b32 b32)
  (define index b32 0) (while (< index size) (inc index)))

#;(define (scm-sp-moving-average! scm-segment-a scm-segment-b scm-width scm-state) (SCM SCM SCM SCM)
  (sp-moving-average! (SCM-BYTEVECTOR-CONTENTS scm-segment)
    (octets->samples (SCM-BYTEVECTOR-LENGTH scm-segment)) (scm->uint32 scm-width)
    (scm->uint32 scm-state)))

(define (sin-lq a) (double double)
  "faster, low precision version of sin()"
  (define b double (/ 4 M_PI)) (define c double (/ -4 (* M_PI M_PI)))
  (return (- (+ (* b a) (* c a (abs a))))))

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
    currently faster by a factor of about 2.6"))
