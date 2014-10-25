(define-macro debug-log? #t)
(includep "stdio.h")
(includep "libguile.h")
(includep "alsa/asoundlib.h")
(include-sc "../extern/sph")
(include-sc "../extern/sph/scm")
(include-sc "../extern/sph/one")
(define-macro init-status (define s b8-s))
(include-sc "io")

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
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean/error")
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
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean/error")
  (scm-c-define-procedure-c t "sp-io-alsa-read"
    2 0
    0 scm-sp-io-alsa-read
    "port sample-count -> channel-data/error
    sp-port integer -> (f32vector ...)/error"))