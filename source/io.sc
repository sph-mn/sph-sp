(includep "fcntl.h")
(define-macro sp-port-type-alsa 0)
(define-macro sp-port-type-file 1)

(define-type port-data-t
  (struct (samples-per-second b32) (channel-count b32) (open-flags b32) (type b8) (data pointer)))

(define (scm-c-create-sp-port type open-flags samples-per-second channel-count data)
  (SCM b8 b32-s b32 b32 pointer)
  (define port-data port-data-t* (scm-gc-malloc (sizeof port-data-t) "sp-port-data"))
  (struct-set (deref port-data) channel-count
    channel-count samples-per-second samples-per-second type type open-flags open-flags)
  (return (scm-new-smob scm-type-sp-port data)))

(define-macro (scm-c-require-success-alsa a) (set s a)
  (if s (scm-c-local-error "alsa" (scm-from-locale-string (snd-strerror s)))))

(define (scm-sp-io-alsa-open input-port? channel-count device-name samples-per-second latency)
  (SCM SCM SCM SCM SCM SCM) scm-c-local-error-init
  (local-memory-init 1) init-status
  (define alsa-port snd-pcm-t*) (define device-name-c char*)
  (scm-if-undefined device-name (set device-name-c "default")
    (begin (set device-name-c (scm->locale-string device-name)) (local-memory-add device-name-c)))
  (scm-c-require-success-alsa
    (snd-pcm-open (address-of alsa-port) device-name-c SND_PCM_STREAM_PLAYBACK 0))
  (define latency-c b32 (scm-if-undefined-expr latency 50 (scm->uint32 latency)))
  (define channel-count-c b32 (scm-if-undefined-expr channel-count 2 (scm->uint32 channel-count)))
  (define samples-per-second-c b32
    (scm-if-undefined-expr samples-per-second 48000 (scm->uint32 samples-per-second)))
  (scm-c-require-success-alsa
    (snd-pcm-set-params alsa-port SND_PCM_FORMAT_U32
      SND_PCM_ACCESS_RW_INTERLEAVED channel-count-c samples-per-second-c 0 latency-c))
  (return
    (scm-c-create-sp-port sp-port-type-alsa (if* input-port? O_RDONLY O_WRONLY)
      samples-per-second-c channel-count-c (convert-type alsa-port pointer)))
  (label error (if alsa-port (snd-pcm-close alsa-port))
    (local-memory-free) (scm-c-local-error-return)))

(define (file-au-read-header file encoding samples-per-second channel-count)
  (b8-s b32-s b32* b32* b32*)
  ;when successful, the reader is positioned to the start of the audio data
  (define s ssize-t header[6] b32) (set s (read file (address-of header) 24))
  (if (not (and (= s 24) (= (deref header) (__builtin-bswap32 779316836)))) (return -1))
  (lseek file (__builtin-bswap32  (deref header 1)) SEEK_SET)
  (set (deref encoding) (__builtin-bswap32 (deref header 3))
    (deref samples-per-second) (__builtin-bswap32 (deref header 4)) (deref channel-count) (__builtin-bswap32 (deref header 5)))
  (return 0))

(define (file-au-write-header file encoding samples-per-second channel-count)
  (b8-s b32-s b32 b32 b32)
  ;assumes file to be positioned at the beginning
  (define s ssize-t header[7] b32) (set (deref header) (__builtin-bswap32 779316836))
  (set (deref header 1) (__builtin-bswap32 28)) (set (deref header 2) (__builtin-bswap32 4294967295))
  (set (deref header 3) (__builtin-bswap32 encoding)) (set (deref header 4) (__builtin-bswap32 samples-per-second))
  (set (deref header 5) (__builtin-bswap32 channel-count)) (set (deref header 6) 0)
  (set s (write file (address-of header) 28)) (if (not (= s 24)) (return -1)) (return 0))

(define (scm-sp-io-file-open path open-flags channel-count samples-per-second)
  (SCM SCM SCM SCM SCM) scm-c-local-error-init
  (scm-c-local-error-assert "input"
    (and (scm-is-string path) (scm-is-integer open-flags)
      (scm-is-integer channel-count) (scm-is-integer samples-per-second)))
  (local-memory-init 1) init-status
  (define file b32-s r SCM) (define open-flags-c b32-s (scm->int32 open-flags))
  (define samples-per-second-c b32 channel-count-c b32)
  (define path-c char* (scm->locale-string path)) (local-memory-add path-c)
  (if (file-exists? path-c)
    (begin (set file (open path-c open-flags-c)) (scm-c-require-success-glibc file)
      (define encoding b32)
      (set s
        (file-au-read-header file (address-of encoding)
          (address-of samples-per-second-c) (address-of channel-count-c)))
      (if (or s (not (= encoding 6))) (scm-c-local-error "encoding" (scm-from-uint32 encoding)))
      (set r
        (scm-c-create-sp-port sp-port-type-file open-flags-c
          samples-per-second-c channel-count-c file)))
    (begin (set file (open path-c (bit-or O_CREAT open-flags-c) #o600))
      (scm-c-require-success-glibc file)
      (set samples-per-second-c (optional-samples-per-second samples-per-second))
      (set channel-count-c (scm->uint32 channel-count))
      (set s (file-au-write-header file 6 samples-per-second-c channel-count-c))
      (if (< s 0) (scm-c-local-error "write-header" 0))
      (set r
        (scm-c-create-sp-port sp-port-type-file open-flags-c
          samples-per-second-c channel-count-c file))))
  (local-memory-free) (return r) (label error (local-memory-free) (scm-c-local-error-return)))

(define (scm-sp-io-ports-close a) (SCM SCM)
  scm-c-local-error-init (define e SCM port-data port-data-t*)
  init-status
  (scm-c-list-each a e
    (compound-statement
      (if (SCM-SMOB-PREDICATE scm-type-sp-port e)
        (begin (set port-data (convert-type (SCM-SMOB-DATA e) port-data-t*))
          (cond
            ( (= sp-port-type-alsa (struct-ref (deref port-data) type))
              (scm-c-require-success-alsa
                (snd-pcm-close (convert-type (struct-ref (deref port-data) data) snd-pcm-t*))))
            ( (= sp-port-type-file (struct-ref (deref port-data) type))
              (scm-c-require-success-glibc
                (close (convert-type (struct-ref (deref port-data) data) b32)))))))))
  (label error (scm-c-local-error-return)))

(define (scm-c-type-sp-port-free a) (size-t SCM) (scm-sp-io-ports-close (scm-list-1 a)) (return 0))

(define
  (scm-sp-io-stream input-ports output-ports segment-size prepared-segment-count proc user-state)
  (SCM SCM SCM SCM SCM SCM SCM)
  #;(define frames-per-buffer-c snd_pcm_uframes_t
    (scm->uint32 (struct-ref sp-state frames-per-buffer)))
  (define i b32 (scm->uint32 prepared-segment-count)) (define prepared-segments SCM SCM-EOL)
  (define time-scm SCM (scm-from-uint8 0))
  (define segment-size-octets SCM (scm-product segment-size (scm-from-uint8 8)))
  (define zero SCM (scm-from-uint8 0))
  (while i (scm-cons (scm-make-f32vector segment-size-octets zero) prepared-segments)
    (decrement-one i))
  (define output SCM)
  (label loop (set output (scm-apply-2 proc time-scm prepared-segments user-state))
    (if (not (scm-is-true output)) (return SCM-UNSPECIFIED))
    (set time-scm (scm-sum segment-size time-scm)) (goto loop)
    ;for each output-port
    ;interleave output-channel-count bytevectors from output if necessary
    ;write to port
    #;(if (scm-is-true buffer)
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
      (return SCM-BOOL-T)))
  ;(define frames-written snd_pcm_sframes_t)
  (return SCM-UNSPECIFIED))

;sp-io-stream input-ports output-ports new-segments-count user-state ... {time segments ... -> (out-n-channel-n  ...)/false} ->

#;(define (scm-ports-read a) (SCM SCM)
  ;(sp-port ...) -> (bytevector ...)
  ;reads one segment for each channel of each port
  )

#;(define (scm-ports-write a data) (SCM SCM SCM)
  ;(sp-port ...) (bytevector ...) ->
  ;writes one segment for each channel of each port
  )