(includep "alsa/asoundlib.h")
(includep "sph.c")
(includep "libguile.h")
(includep "stdio.h")

(define-type sp-state-t
  (struct (samples-per-second b32) (frames-per-buffer b32) (channels b32) (io b0*)))

(define-type sp-io-alsa-t
  (struct (out snd-pcm-t*) (in snd-input-t*) ))

(define sp-state sp-state-t)

(define (sp-alsa-underrun-recovery handle s) (snd-pcm-t* b8-s)
  (if (= -EPIPE s)
    (begin (set s (snd-pcm-prepare handle))
      (if (< s 0) (debug-log "could not recover from underrun: %s" (snd-strerror s)) (return 0)))
    (if (= -ESTRIPIPE s)
      (begin
        (label loop (set s (snd-pcm-resume handle))
          (if (= -EAGAIN s) (begin (sleep 1) (goto loop))))
        (if (< s 0) (set s (snd-pcm-prepare handle))
          (if (< s 0) (debug-log "could not recover from suspend: %s" (snd-strerror s))))
        (return 0))))
  (return s))

(define (scm-sp-init-alsa channels sample-per-second
) SCM
  (define latency uint32-t)
  (define s int32-t out snd-pcm-t*)
  (set s (snd-pcm-open (address-of out) device-name SND_PCM_STREAM_PLAYBACK 0))
  (if (< s 0)
    (begin (debug-log "alsa playback init error: %s" (snd-strerror s))
      (return SCM-BOOL-F)
      ))

  (set s
    (snd-pcm-set-params out SND_PCM_FORMAT_U32
      SND_PCM_ACCESS_RW_INTERLEAVED channels sample-rate 0 latency))
  (if (< s 0) (begin (printf "Playback open error: %s\n" (snd-strerror s)) (exit EXIT_FAILURE))))

(define (scm-sp-start-alsa proc) (SCM SCM)
  (define s b8-s buffer SCM)
  (label loop
    (set buffer
      (scm-call-2 (struct-ref sp-state samples-per-second)
        (scm-c-make-bytevector (struct-ref sp-state frames-per-buffer-c))))
    (set s
      (snd-pcm-writei (convert-type (struct-ref sp-state io) snd-pcm-t*)
        (SCM-BYTEVECTOR-CONTENTS buffer) (struct-ref sp-state frames-per-buffer-c))
      (if (= s -EAGAIN) (goto loop)
        (if (< s 0)
          (if (< (sp-alsa-underrun-recovery (struct-ref sp-state io) s) 0)
            (begin (debug-log "write error: %s" (snd-strerror s)) (return s)))
          (goto loop))))
    (return SCM-UNSPECIFIED)))

(define (scm-sp-exit-alsa) SCM (snd-pcm-close (convert-type (struct-ref sp-state io) snd-pcm-t)))

(define (init-sp) b0
  (define scm-temp SCM) (scm-c-define-c scm-temp "sp-start-alsa" 1 0 0 scm-sp-start-alsa "")
  (scm-c-define-c scm-temp "sp-exit-alsa" 0 0 0 scm-sp-exit-alsa "")
  (scm-c-define-c scm-temp "sp-init-alsa" 0 0 0 scm-sp-init-alsa ""))