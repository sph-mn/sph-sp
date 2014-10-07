(define-macro debug-log? #t)
(includep "stdio.h")
(includep "libguile.h")
(includep "alsa/asoundlib.h")
(include-sc "../lib/sph")
(include-sc "../lib/sph/scm")
(define-macro init-status (define s b8-s))

(define-macro (optional-samples-per-second a)
  (if (= SCM-UNDEFINED a) 96000 (scm->uint32 a)))

(include-sc "error")
(include-sc "io")
(define-macro sw-resampling 0)
(define-macro default-bits-per-sample 32)

(define-macro (increment-time sp-state)
  (set (struct-ref sp-state time)
    (scm-sum (struct-ref sp-state time) (struct-ref sp-state frames-per-buffer))))

(define (scm-sp-io-stream input-ports output-ports prepared-segment-count proc)
  (SCM SCM SCM SCM SCM)
  (define frames-per-buffer-c snd_pcm_uframes_t
    (scm->uint32 (struct-ref sp-state frames-per-buffer)))
  (define buffer SCM
    (scm-c-make-bytevector
      (/ (* bits-per-sample frames-per-buffer-c (scm->uint32 (struct-ref sp-state channel-count)))
        8)))
  (define io sp-io-alsa-t (deref (convert-type (struct-ref sp-state io) sp-io-alsa-t*)))
  (define frames-written snd_pcm_sframes_t)
  (label loop (set buffer (scm-call-2 proc buffer (struct-ref sp-state time)))
    (if (scm-is-true buffer)
      (begin
        (set frames-written
          (snd-pcm-writei (struct-ref io out) (SCM-BYTEVECTOR-CONTENTS buffer) frames-per-buffer-c))
        (if (= frames-written -EAGAIN) (goto loop)
          (if (< frames-written 0)
            (if (< (snd-pcm-recover (struct-ref io out) frames-written 0) 0)
              (begin (debug-log "write error: %s" (snd-strerror frames-written))
                (return SCM-BOOL-F))
              (begin (increment-time sp-state) (goto loop)))
            (begin (increment-time sp-state) (goto loop)))))
      (return SCM-BOOL-T))))

(define (init-sp) b0
  (set scm-type-sp-port (scm-make-smob-type "sp-port" 0))
  (scm-set-smob-free scm-type-sp-port scm-type-sp-port-free) (define scm-temp SCM)
  (scm-c-define-c scm-temp "sp-loop-alsa"
    1 0 0 scm-sp-loop-alsa "procedure:{buffer time} -> boolean")
  (scm-c-define-c scm-temp "sp-deinit-alsa" 0 0 0 scm-sp-deinit-alsa "->")
  (scm-c-define-c scm-temp "sp-init-alsa"
    0 4 0 scm-sp-init-alsa "[samples-per-second device-name channel-count latency] -> boolean"))