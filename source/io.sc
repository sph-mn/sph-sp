;input/output ports
(define scm-type-cursor scm-t-bits)
(define-macro sp-port-type-alsa 0)
(define-macro sp-port-type-file 10)
(define-macro sp-port-type-output 0)
(define-macro sp-port-type-input 1)

(define-macro (scm-create-sp-port input-port? type data)
  (scm-new-double-smob scm-type-sp-port (convert-type (bit-or input-port? type) scm-t-bits)
    (convert-type data scm-t-bits)))

(define (scm-type-sp-port-free a) (size-t SCM)
  (define type b32 (convert-type (SCM-SMOB-DATA a) b32))
  (define data b0* (convert-type (SCM-SMOB-DATA-2 a) b0*)) init-status
  (if (bit-and sp-port-type-alsa type)
    (begin (set s (snd-pcm-close (convert-type data snd-pcm-t*)))
      (if s (debug-log "%s" "error closing alsa port: " (snd-strerror s)))))
  (return 0))

(define-macro (asound-require-success a) (set s a) (if s (goto error)))

(define (sp-io-alsa-open input-port? channel-count device-name samples-per-second latency)
  (SCM SCM SCM SCM SCM) init-status
  (define alsa-port snd-pcm-t*)
  (define device-name-c char*
    (if* (= SCM-UNDEFINED device-name) "default" (scm->locale-string device-name)))
  (asound-require-success
    (snd-pcm-open (address-of alsa-port) device-name-c SND_PCM_STREAM_PLAYBACK 0))
  (set s
    (snd-pcm-set-params alsa-port SND_PCM_FORMAT_U32
      SND_PCM_ACCESS_RW_INTERLEAVED
      (if* (= SCM-UNDEFINED channel-count) 2 (scm->uint32 channel-count))
      (if* (= SCM-UNDEFINED samples-per-second) 48000 (scm->uint32 samples-per-second)) sw-resampling
      (if* (= SCM-UNDEFINED samples-per-second) 25 (scm->uint8 latency))))
  (if (< s 0) (goto error)) (if (not (= SCM-UNDEFINED device-name)) (free device-name-c))
  (return SCM-BOOL-T)
  (label error (debug-log "alsa init error: %s" (snd-strerror s))
    (if (not (= SCM-UNDEFINED device-name)) (free device-name-c)) (free io) (return SCM-BOOL-F)))

(define (scm-sp-io-close-alsa) SCM
  init-status
  (set s
    (snd-pcm-close (struct-ref (deref (convert-type (struct-ref sp-state io) sp-io-alsa-t*)) out)))
  (if (< s 0) (begin (debug-log "write error: %s" (snd-strerror s)) (return SCM-BOOL-F))
    (return SCM-BOOL-T)))

(define-type sp-io-alsa-t (struct (data snd-pcm-t*) ()))
(define-macro sp-io-file-au-type-f32 6)
(define-type port-data-t (struct samples-per-second b32 channel-count b32))

(define (file-au-read-header file encoding samples-per-second channel-count)
  (b8-s b32-s b32* b32* b32*)
  ;when successful, the reader is positioned to the start of the audio data
  (define ssize-t s) (define header b32[6])
  (set s (read file (address-of header) 24)) (if (not (= s 24)) (return -1))
  (if (not (= (deref header) 779316836)) (return -1)) (seek file (deref header 1) SEEK_SET)
  (set (deref encoding) (deref header 3)
    (deref samples-per-second) (deref header 4) (deref channel-count) (deref header 5))
  (return 0))

(define (file-au-write-header file encoding samples-per-second channel-count)
  (b8-s b32-s b32 b32 b32)
  ;assumes file to be positioned at the beginning
  (define ssize-t s) (define header b32[7])
  (set (deref header) 779316836) (set (deref header 1) 28)
  (set (deref header 2) 4294967295) (set (deref header 3) encoding)
  (set (deref header 4) samples-per-second) (set (deref header 5) channel-count)
  (set (deref header 6) 0) (set s (write file (address-of header) 28))
  (if (not (= s 24)) (return -1)) (return 0))

(define-macro (sp-malloc r size) (b0* size-t)
  (set r b0* (malloc size)) (if (not r) (sp-scm-return-error-from-number error-memory)))

(define-macro (sp-define-malloc name type) (define name type* (malloc (sizeof type)))
  (if (not name) (sp-scm-return-error-from-number error-memory)))

(define (scm-sp-io-file-open path mode channel-count samples-per-second) (SCM SCM SCM)
  local-error-init
  (local-error-assert error-input
    (and (scm-is-string path) (scm-is-integer mode)
      (scm-is-integer channel-count) (scm-is-integer samples-per-second)))
  (local-memory-init 3)
  init-status
  (define file b32-s r SCM)
  (define mode b32-s (scm->int32 mode))
  (define path-c char* (scm->locale-string path))
  (local-memory-add path-c)
  (if (file-exists? path-c)
    (begin (set file (open path-c mode))
      (if (< file 0) (local-error "access" ""))
      (define encoding b32 port-data port-data-t*) (sp-define-malloc port-data port-data-t)
      (set s
        (file-au-read-header (address-of encoding)
          (address-of (struct-ref port-data samples-per-second))
          (address-of (struct-ref port-data channels))))
      (if (or s (not (= encoding 6)))
        (local-error "read-header" ""))
      (set r
        (scm-create-sp-port (or (bit-and O_REAhD mode) (bit-and O_RDRW mode)) sp-port-type-file
          port-data)))
    (begin (set file (open path-c (bit-or O_CREAT mode)))
      (if (< file 0) (local-error "create" ""))
      (local-define-malloc port-data port-data-t)
      (struct-set port-data samples-per-second
        (optional-samples-per-second samples-per-second) channels (scm->uint32 channel-count))
      (set s (file-au-write-header 6 samples-per-second-c channel-count-c)) (define encoding b32)
      (if (or s (not (= encoding 6))) (local-error "encoding" ""))
      (set r
        (scm-create-sp-port (or (bit-and O_READ mode) (bit-and O_RDRW mode)) sp-port-type-file
          port-data))))
  (free path-c) (return)
  (label error
    (if (< 0 local-error-number)
      (error-description local-error-number)
      (case
        ((= 0) )
        )
      )
    (sp-scm-return-error local-error-string local-error-description)
    (free path-c) (return SCM-BOOL-F)))

(define (scm-ports-close a) (SCM SCM))

(define (scm-ports-read a) (SCM SCM)
  ;(sp-port ...) -> (bytevector ...)
  ;reads one segment for each channel of each port
  )

(define (scm-ports-write a data) (SCM SCM SCM)
  ;(sp-port ...) (bytevector ...) ->
  ;writes one segment for each channel of each port
  )