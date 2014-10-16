(define-macro debug-log? #t)
(includep "stdio.h")
(includep "libguile.h")
(includep "alsa/asoundlib.h")
(include-sc "../extern/sph")
(include-sc "../extern/sph/scm")
(include-sc "../extern/sph/one")
(define-macro init-status (define s b8-s))
(define-macro (optional-samples-per-second a) (if* (= SCM-UNDEFINED a) 96000 (scm->uint32 a)))
(include-sc "io")

(define (init-sp) b0
  (init-scm) sp-port-scm-type-init
  (define t SCM)
  (scm-c-define-procedure-c t "sp-io-port-close"
    1 0 0 scm-sp-io-port-close "sp-port -> boolean/error")
  (scm-c-define-procedure-c t "sp-io-file-open-input"
    1 2
    0 scm-sp-io-file-open-input
    "string [integer integer] -> sp-port/error
    path [channel-count samples-per-second] -> sp-port/error")
  (scm-c-define-procedure-c t "sp-io-file-open-output"
    1 2
    0 scm-sp-io-file-open-output
    "string [integer integer] -> sp-port/error
    path [channel-count samples-per-second] -> sp-port/error")
  (scm-c-define-procedure-c t "sp-io-file-write"
    3 0
    0 scm-sp-io-file-write
    "sp-port integer:sample-count (f32vector ...):channel-data -> boolean/error")
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
    3 0
    0 scm-sp-io-alsa-write
    "sp-port integer (f32vector ...) -> boolean/error
    port sample-count channel-data -> boolean/error")
  (scm-c-define-procedure-c t "sp-io-alsa-read"
    2 0
    0 scm-sp-io-alsa-read
    "port sample-count -> channel-data/error
    sp-port integer -> (f32vector ...)/error"))