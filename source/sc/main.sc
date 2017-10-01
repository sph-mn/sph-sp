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
  (scm-c-define-procedure-c "sp-io-file-open-input" 1
    2 0 scm-sp-io-file-open-input
    "string -> sp-port
    path -> sp-port")
  (scm-c-define-procedure-c "sp-io-file-open-output" 1
    2 0
    scm-sp-io-file-open-output
    "string [integer integer] -> sp-port
    path [channel-count sample-rate] -> sp-port")
  (scm-c-define-procedure-c "sp-io-file-write" 2
    1 0
    scm-sp-io-file-write
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean
    write sample data to the channels of a file port")
  (scm-c-define-procedure-c "sp-io-file-set-position" 2
    0 0
    scm-sp-io-file-set-position
    "sp-port integer:sample-offset -> boolean
    sample-offset can be negative, in which case it is from the end of the file")
  (scm-c-define-procedure-c "sp-io-file-read" 2
    0 0 scm-sp-io-file-read "sp-port integer:sample-count -> (f32vector ...):channel-data")
  (scm-c-define-procedure-c "sp-io-alsa-open-input" 0
    4 0
    scm-sp-io-alsa-open-input
    "[string integer integer integer] -> sp-port
    [device-name channel-count sample-rate latency] -> sp-port")
  (scm-c-define-procedure-c "sp-io-alsa-open-output" 0
    4 0
    scm-sp-io-alsa-open-output
    "[string integer integer integer] -> sp-port
    [device-name channel-count sample-rate latency] -> sp-port")
  (scm-c-define-procedure-c "sp-io-alsa-write" 2
    1 0
    scm-sp-io-alsa-write
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean
    write sample data to the channels of an alsa port - to the sound card for sound output for example")
  (scm-c-define-procedure-c "sp-io-alsa-read" 2
    0 0
    scm-sp-io-alsa-read
    "port sample-count -> channel-data
    sp-port integer -> (f32vector ...)"))
