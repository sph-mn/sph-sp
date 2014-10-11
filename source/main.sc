(define-macro debug-log? #t)
(includep "stdio.h")
(includep "libguile.h")
(includep "alsa/asoundlib.h")
(include-sc "../extern/sph")
(include-sc "../extern/sph/scm")
(include-sc "../extern/sph/one")
(define-macro init-status (define s b8-s))
(define-macro (optional-samples-per-second a) (if* (= SCM-UNDEFINED a) 96000 (scm->uint32 a)))
(define scm-type-sp-port scm-t-bits)
(include-sc "io")

(define (init-sp) b0
  (init-scm)
  (set scm-type-sp-port (scm-make-smob-type "sp-port" 0))
  (scm-set-smob-free scm-type-sp-port scm-c-type-sp-port-free) (define t SCM)
  (scm-c-define-procedure-c t "sp-io-stream"
    0 1 1
    scm-sp-io-stream
    "list list integer integer procedure ->
    (sp-port ...) (sp-port ...) samples-per-segment prepared-segment-count {integer:time list:prepared-segments user-state ...} ->")
  (scm-c-define-procedure-c t "sp-io-ports-close" 1 0 0 scm-sp-io-ports-close "(sp-port ...) ->")
  (scm-c-define-procedure-c t "sp-io-file-open"
    2 2
    0 scm-sp-io-file-open
    "path mode [channel-count samples-per-second] -> sp-port
    string integer integer integer -> sp-port")
  (scm-c-define-procedure-c t "sp-io-alsa-open"
    2 3
    0 scm-sp-io-alsa-open
    "input-port? channel-count device-name samples-per-second latency -> sp-port
    boolean integer string integer integer"))