;a signal processing loop for guile

(includep "stdio.h")
(includep "libguile.h")
(includep "alsa/asoundlib.h")
(include-sc "../lib/sph")
(include-sc "../lib/sph/scm")
(define-macro init-status (define s b8-s))
(define-macro sw-resampling 0)

(define-type sp-state-t
  (struct (samples-per-second SCM) (time SCM) (frames-per-buffer SCM) (channel-count SCM) (io b0*)))

(define-type sp-io-alsa-t (struct (out snd-pcm-t*) (in snd-pcm-t*)))
(define sp-state sp-state-t)

(define (scm-sp-init-alsa samples-per-second device-name channel-count latency)
  (SCM SCM SCM SCM SCM) init-status
  (define io sp-io-alsa-t* (malloc (sizeof sp-io-alsa-t)))
  (if (= 0 io) (debug-log "not enough memory available"))
  (define device-name-c char*
    (if* (= SCM-UNDEFINED device-name) "default" (scm->locale-string device-name)))
  (set s
    (snd-pcm-open (address-of (struct-ref (deref io) out)) device-name-c SND_PCM_STREAM_PLAYBACK 0))
  (if (< s 0) (goto error))
  (set s
    (snd-pcm-set-params (struct-ref (deref io) out) SND_PCM_FORMAT_U32
      SND_PCM_ACCESS_RW_INTERLEAVED
      (if* (= SCM-UNDEFINED channel-count) 2 (scm->uint32 channel-count))
      (if* (= SCM-UNDEFINED samples-per-second) 48000 (scm->uint32 samples-per-second)) sw-resampling
      (if* (= SCM-UNDEFINED samples-per-second) 25 (scm->uint8 latency))))
  (if (< s 0) (goto error)) (if (not (= SCM-UNDEFINED device-name)) (free device-name-c))
  (struct-set sp-state io io) (return SCM-BOOL-T)
  (label error (debug-log "alsa init error: %s" (snd-strerror s))
    (if (not (= SCM-UNDEFINED device-name)) (free device-name-c)) (free io) (return SCM-BOOL-F)))

(define-macro (increment-time sp-state)
  (set (struct-ref sp-state time)
    (scm-sum (struct-ref sp-state time) (struct-ref sp-state frames-per-buffer))))

(define (scm-sp-loop-alsa proc) (SCM SCM)
  (define buffer SCM)
  (define frames-per-buffer-c b32 (scm->uint32 (struct-ref sp-state frames-per-buffer)))
  (define io sp-io-alsa-t (deref (convert-type (struct-ref sp-state io) sp-io-alsa-t*)))
  (define frames-written snd_pcm_sframes_t)
  (label loop
    (set buffer
      (scm-call-2 proc (scm-c-make-bytevector frames-per-buffer-c) (struct-ref sp-state time)))
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

(define (scm-sp-deinit-alsa) SCM
  init-status
  (set s
    (snd-pcm-close (struct-ref (deref (convert-type (struct-ref sp-state io) sp-io-alsa-t*)) out)))
  (if (< s 0) (begin (debug-log "write error: %s" (snd-strerror s)) (return SCM-BOOL-F))
    (return SCM-BOOL-T)))

(define (init-sp) b0
  (define scm-temp SCM) (scm-c-define-c scm-temp "sp-loop-alsa" 1 0 0 scm-sp-loop-alsa "")
  (scm-c-define-c scm-temp "sp-deinit-alsa" 0 0 0 scm-sp-deinit-alsa "")
  (scm-c-define-c scm-temp "sp-init-alsa" 0 4 0 scm-sp-init-alsa ""))