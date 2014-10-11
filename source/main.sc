(define-macro debug-log? #t)
(includep "stdio.h")
(includep "libguile.h")
(includep "alsa/asoundlib.h")
(include-sc "../lib/sph")
(include-sc "../lib/sph/scm")
(define-macro init-status (define s b8-s))
(define-macro (optional-samples-per-second a) (if (= SCM-UNDEFINED a) 96000 (scm->uint32 a)))
(define scm-type-sp-port scm-t-bits)
(include-sc "io")

(define (init-sp) b0
  (set scm-type-sp-port (scm-make-smob-type "sp-port" 0))
  (scm-set-smob-free scm-type-sp-port scm-c-type-sp-port-free) (define scm-temp SCM)
  (scm-c-define-procedure-c scm-temp "sp-loop-alsa"
    1 0 0 scm-sp-loop-alsa "procedure:{buffer time} -> boolean")
  (scm-c-define-procedure-c scm-temp "sp-deinit-alsa" 0 0 0 scm-sp-deinit-alsa "->")
  (scm-c-define-procedure-c scm-temp "sp-init-alsa"
    0 4 0 scm-sp-init-alsa "[samples-per-second device-name channel-count latency] -> boolean"))