(define-macro debug-log? #t)
(includep "stdio.h")
(includep "libguile.h")
(includep "alsa/asoundlib.h")
(include-sc "../../foreign/sph")
(include-sc "../../foreign/sph/scm")
(include-sc "../../foreign/sph/one")
(define-macro init-status (define s b8-s))
(include-sc "io")
(include "../../foreign/kissfft/kiss_fft.h")
(include "../../foreign/kissfft/tools/kiss_fftr.h")

(define (scm-sp-fft a) (SCM SCM)
  scm-c-local-error-init (scm-c-local-error-assert "type-check" (scm-is-true (scm-f32vector? a)))
  (define size b32 (/ (SCM-BYTEVECTOR-LENGTH a) 4)) (define size-result b32 (+ 1 (* size 0.5)))
  (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc size 0 0 0))
  (define out kiss-fft-cpx* (malloc (* size-result (sizeof kiss-fft-cpx))))
  (kiss-fftr fftr-state (convert-type (SCM-BYTEVECTOR-CONTENTS a) f32-s*) out)
  (define r SCM (scm-make-f32vector (scm-from-uint32 size-result) (scm-from-uint8 0)))
  (while size-result (decrement-one size-result)
    (set (deref (convert-type (SCM-BYTEVECTOR-CONTENTS r) f32-s*) size-result)
      (struct-ref out[size-result] r)))
  (free fftr-state) (free out) (return r) (label error scm-c-local-error-return))

(define (scm-sp-fft-inverse a) (SCM SCM)
  scm-c-local-error-init (scm-c-local-error-assert "type-check" (scm-is-true (scm-f32vector? a)))
  (define size b32 (/ (SCM-BYTEVECTOR-LENGTH a) 4)) (define size-result b32 (* (- size 1) 2))
  (define fftr-state kiss-fftr-cfg (kiss-fftr-alloc size-result 1 0 0))
  (define inp kiss-fft-cpx* (malloc (* size (sizeof kiss-fft-cpx))))
  (while size (decrement-one size)
    (struct-set inp[size] r (deref (SCM-BYTEVECTOR-CONTENTS a) (* size 4))))
  (define r SCM (scm-make-f32vector (scm-from-uint32 size-result) (scm-from-uint8 0)))
  (kiss-fftri fftr-state inp (convert-type (SCM-BYTEVECTOR-CONTENTS r) f32-s*)) (free fftr-state)
  (return r) (label error scm-c-local-error-return))

(define (init-sp) b0
  (init-scm) sp-port-scm-type-init
  (define t SCM) (define scm-module SCM (scm-c-resolve-module "sph sp"))
  (set scm-sp-port-type-alsa (scm-from-uint8 sp-port-type-alsa))
  (set scm-sp-port-type-file (scm-from-uint8 sp-port-type-file))
  (scm-c-module-define scm-module "sp-port-type-alsa" scm-sp-port-type-alsa)
  (scm-c-module-define scm-module "sp-port-type-file" scm-sp-port-type-file)
  (scm-c-define-procedure-c t "sp-port-close" 1 0 0 scm-sp-port-close "sp-port -> boolean/error")
  (scm-c-define-procedure-c t "sp-port-input?" 1 0 0 scm-sp-port-input? "sp-port -> boolean/error")
  (scm-c-define-procedure-c t "sp-port-position?"
    1 0 0 scm-sp-port-position? "sp-port -> boolean/error")
  (scm-c-define-procedure-c t "sp-port-position"
    1 0 0 scm-sp-port-position "sp-port -> integer/boolean/error")
  (scm-c-define-procedure-c t "sp-port-channel-count"
    1 0 0 scm-sp-port-channel-count "sp-port -> integer/error")
  (scm-c-define-procedure-c t "sp-port-samples-per-second"
    1 0 0 scm-sp-port-samples-per-second "sp-port -> integer/boolean/error")
  (scm-c-define-procedure-c t "sp-port?" 1 0 0 scm-sp-port? "sp-port -> boolean")
  (scm-c-define-procedure-c t "sp-port-type" 1 0 0 scm-sp-port-type "sp-port -> integer")
  (scm-c-define-procedure-c t "sp-fft"
    1 0
    0 scm-sp-fft
    "f32vector:volumes-per-time -> f32vector:frequencies-per-time
    discrete fourier transform on the input data")
  (scm-c-define-procedure-c t "sp-fft-inverse"
    1 0
    0 scm-sp-fft-inverse
    "f32vector:frequencies-per-time -> f32vector:volume-per-time
    inverse discrete fourier transform on the input data")
  (scm-c-define-procedure-c t "sp-io-file-open-input"
    1 2 0 scm-sp-io-file-open-input
    "string -> sp-port/error
    path -> sp-port/error")
  (scm-c-define-procedure-c t "sp-io-file-open-output"
    1 2
    0 scm-sp-io-file-open-output
    "string [integer integer] -> sp-port/error
    path [channel-count samples-per-second] -> sp-port/error")
  (scm-c-define-procedure-c t "sp-io-file-write"
    2 1
    0 scm-sp-io-file-write
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean/error
    write sample data to the channels of a file port")
  (scm-c-define-procedure-c t "sp-io-file-set-position"
    2 0
    0 scm-sp-io-file-set-position
    "sp-port integer:sample-offset -> boolean/error
    sample-offset can be negative, in which case it is from the end of the file")
  (scm-c-define-procedure-c t "sp-io-file-read"
    2 0 0 scm-sp-io-file-read "sp-port integer:sample-count -> (f32vector ...):channel-data/error")
  (scm-c-define-procedure-c t "sp-io-alsa-open-input"
    0 4
    0 scm-sp-io-alsa-open-input
    "[string integer integer integer] -> sp-port/error
    [device-name channel-count samples-per-second latency] -> sp-port/error")
  (scm-c-define-procedure-c t "sp-io-alsa-open-output"
    0 4
    0 scm-sp-io-alsa-open-output
    "[string integer integer integer] -> sp-port/error
    [device-name channel-count samples-per-second latency] -> sp-port/error")
  (scm-c-define-procedure-c t "sp-io-alsa-write"
    2 1
    0 scm-sp-io-alsa-write
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean/error
    write sample data to the channels of an alsa port - to the sound card for sound output for example")
  (scm-c-define-procedure-c t "sp-io-alsa-read"
    2 0
    0 scm-sp-io-alsa-read
    "port sample-count -> channel-data/error
    sp-port integer -> (f32vector ...)/error"))