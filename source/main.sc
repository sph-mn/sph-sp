;a signal processing loop for guile

(includep "alsa/asoundlib.h")
(includep "sph.c")
(includep "libguile.h")
(includep "stdio.h")
(define-macro init-status (define s b8-s))
(define-macro latency-ms 25)
(define-macro sw-resampling 0)

(define-type sp-state-t
  (struct (samples-per-second SCM) (frames-per-buffer SCM) (channel-count SCM) (io b0*)))

(define-type sp-io-alsa-t (struct (out snd-pcm-t*) (in snd-pcm-t*)))
(define sp-state sp-state-t)

(define (scm-sp-init-alsa samples-per-second device-name channel-count latency)
  (SCM SCM SCM SCM SCM) init-status
  (define io sph-io-alsa-t* (malloc (sizeof sph-io-alsa-t)))
  (if (= 0 io) (debug-log "not enough memory available"))
  (define device-name-c b8* (scm->locale-string device-name))
  (set s (snd-pcm-open (struct-ref (deref io) out) device-name-c SND_PCM_STREAM_PLAYBACK 0))
  (if (< s 0) (goto error))
  (set s
    (snd-pcm-set-params (struct-ref (deref io) out) SND_PCM_FORMAT_U32
      SND_PCM_ACCESS_RW_INTERLEAVED
      (if* (= SCM-UNDEFINED channel-count) 2 (scm->uint32 channel-count))
      (if* (= SCM-UNDEFINED samples-per-second) 96000 (scm->uint32 samples-per-second)) sw-resampling
      (if* (= SCM-UNDEFINED samples-per-second) 25 (scm->uint8 latency))))
  (if (< s 0) (goto error)) (free device-name-c)
  (struct-set sp-state io io) (return SCM-BOOL-T)
  (label error (debug-log "alsa init error: %s" (snd-strerror s))
    (free device-name-c) (free io) (return SCM-BOOL-F)))

(define (scm-sp-loop-alsa proc) (SCM SCM)
  init-status (define buffer SCM)
  (define frames-per-buffer-c b32 (scm->uint32 (struct-ref sp-state frames-per-buffer)))
  (define io sp-io-alsa-t (deref (struct-ref sp-state io)))
  (label loop
    (set buffer
      (scm-call-2 proc (struct-ref sp-state samples-per-second)
        (scm-c-make-bytevector frames-per-buffer-c)))
    (if (scm-is-true buffer)
      (begin
        (set s
          (snd-pcm-writei (struct-ref io out) (SCM-BYTEVECTOR-CONTENTS buffer) frames-per-buffer-c))
        (if (= s -EAGAIN) (goto loop)
          (if (< s 0)
            (if (< (snd-pcm-recover (struct-ref io out) s 0) 0)
              (begin (debug-log "write error: %s" (snd-strerror s)) (return SCM-BOOL-F)))
            (goto loop))))
      (return SCM-BOOL-T))))

(define (scm-sp-deinit-alsa) SCM
  init-status
  (set s
    (snd-pcm-close
      (convert-type (struct-ref (convert-type (struct-ref sp-state io) sp-io-alsa-t) out))))
  (if (< s 0) (begin (debug-log "write error: %s" (snd-strerror s)) (return SCM-BOOL-F))
    (return SCM-BOOL-T)))

(define (init-sp) b0
  (define scm-temp SCM) (scm-c-define-c scm-temp "sp-loop-alsa" 1 0 0 scm-sp-loop-alsa "")
  (scm-c-define-c scm-temp "sp-deinit-alsa" 0 0 0 scm-sp-deinit-alsa "")
  (scm-c-define-c scm-temp "sp-init-alsa" 0 4 0 scm-sp-init-alsa ""))