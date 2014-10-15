;uses a custom type for ports because guiles port api is underdocumented / too complicated for now
(includep "fcntl.h")

(define (sp-interleave-n target source n size) (b0 f32-s* f32-s** b32 b32)
  (define temp-n b32)
  (while size (decrement-one size)
    (set temp-n n)
    (while temp-n (decrement-one temp-n)
      (set (deref target (+ size temp-n)) (deref (deref source size) temp-n)))))

(define (sp-deinterleave-n target source n size) (b0 f32-s** f32-s* b32 b32)
  (define temp-n b32)
  (while size (decrement-one size)
    (set temp-n n)
    (while temp-n (decrement-one temp-n)
      (set (deref (deref target size) temp-n) (deref source (+ size temp-n))))))

(define-type port-data-t
  (struct (samples-per-second b32) (channel-count b32)
    (input b8) (type b8) (closed? b8) (data pointer)))

(define-macro sp-port-type-alsa 0)
(define-macro sp-port-type-file 1)
(define sp-port-scm-type scm-t-bits)

(define-macro sp-port-scm-type-init
  ;create and setup the type
  (set sp-port-scm-type (scm-make-smob-type "sp-port" 0))
  (scm-set-smob-print sp-port-scm-type sp-port-print))

(define-macro (sp-port-type->name a)
  (cond* ((= sp-port-type-file a) "file") ((= sp-port-type-alsa a) "alsa") (else "unknown")))

(define-macro (sp-port->port-data a) (convert-type (SCM-SMOB-DATA a) port-data-t*))

(define (sp-port-print a output-port print-state) (int SCM SCM scm-print-state*)
  (define port-data port-data-t* (convert-type (SCM-SMOB-DATA a) port-data-t*)) (define r[255] char)
  (sprintf r
    "#<sp-port %lx type:%s samples-per-second:%d channel-count:%d closed?:%s input?:%s>"
    (convert-type a pointer) (sp-port-type->name (struct-ref (deref port-data) type))
    (struct-ref (deref port-data) samples-per-second) (struct-ref (deref port-data) channel-count)
 (if* (struct-ref (deref port-data) closed?) "#t" "#f")
    (if* (struct-ref (deref port-data) input?) "#t" "#f"))
  (scm-display (scm-from-locale-string r) output-port) (return 0))

(define (sp-port-create type input? samples-per-second channel-count data)
  (SCM b8 b32-s b32 b32 pointer)
  (define port-data port-data-t* (scm-gc-malloc (sizeof port-data-t) "sp-port-data"))
  (struct-set (deref port-data) channel-count
    channel-count samples-per-second samples-per-second type type input? input? data data)
  (return (scm-new-smob sp-port-scm-type (convert-type port-data scm-t-bits))))

(define-macro (scm-c-require-success-alsa a) (set s a)
  (if s (scm-c-local-error "alsa" (scm-from-locale-string (snd-strerror s)))))

(define (scm-sp-io-port-close a) (SCM SCM)
  scm-c-local-error-init (define port-data port-data-t*)
  init-status
  (if (SCM-SMOB-PREDICATE sp-port-scm-type a)
    (begin (set port-data (convert-type (SCM-SMOB-DATA a) port-data-t*))
      (if (struct-ref (deref port-data) closed?) (scm-c-local-error "already-closed" 0)
        (begin
          (cond
            ( (= sp-port-type-alsa (struct-ref (deref port-data) type))
              (scm-c-require-success-alsa
                (snd-pcm-close (convert-type (struct-ref (deref port-data) data) snd-pcm-t*))))
            ( (= sp-port-type-file (struct-ref (deref port-data) type))
              (scm-c-require-success-glibc
                (close (convert-type (struct-ref (deref port-data) data) b32)))))
          (struct-set (deref port-data) closed? 1))))
    (scm-c-local-error "input" 0))
  (return SCM-BOOL-T) (label error scm-c-local-error-return))

(define (sp-io-alsa-open input? device-name channel-count samples-per-second latency)
  (b8 SCM SCM SCM SCM SCM) scm-c-local-error-init
  (local-memory-init 1) init-status
  (define alsa-port snd-pcm-t*) (define device-name-c char*)
  (scm-if-undefined device-name (set device-name-c "default")
    (begin (set device-name-c (scm->locale-string device-name)) (local-memory-add device-name-c)))
  (scm-c-require-success-alsa
    (snd-pcm-open (address-of alsa-port) device-name-c (if* input? SND_PCM_STREAM_CAPTURE SND_PCM_STREAM_PLAYBACK) 0))
  (define latency-c b32 (scm-if-undefined-expr latency 50 (scm->uint32 latency)))
  (define channel-count-c b32 (scm-if-undefined-expr channel-count 1 (scm->uint32 channel-count)))
  (define samples-per-second-c b32
    (scm-if-undefined-expr samples-per-second 48000 (scm->uint32 samples-per-second)))
  (scm-c-require-success-alsa
    (snd-pcm-set-params alsa-port SND_PCM_FORMAT_U32
      SND_PCM_ACCESS_RW_NONINTERLEAVED channel-count-c samples-per-second-c 0 latency-c))
  local-memory-free
  (return
    (sp-port-create sp-port-type-alsa input?
      samples-per-second-c channel-count-c (convert-type alsa-port pointer)))
  (label error (if alsa-port (snd-pcm-close alsa-port)) local-memory-free scm-c-local-error-return))

(define (scm-sp-io-alsa-open-input channel-count device-name samples-per-second latency)
  (return (sp-io-alsa-open 1 device-name channel-count samples-per-second latency))
  )

(define (scm-sp-io-alsa-open-output channel-count device-name samples-per-second latency)
  (return (sp-io-alsa-open 0 device-name channel-count samples-per-second latency))
  )

(define (file-au-write-header file encoding samples-per-second channel-count)
  (b8-s b32-s b32 b32 b32)
  ;assumes file to be positioned at the beginning
  (define s ssize-t header[7] b32) (set (deref header) (__builtin-bswap32 779316836))
  (set (deref header 1) (__builtin-bswap32 28)) (set (deref header 2) (__builtin-bswap32 4294967295))
  (set (deref header 3) (__builtin-bswap32 encoding))
  (set (deref header 4) (__builtin-bswap32 samples-per-second))
  (set (deref header 5) (__builtin-bswap32 channel-count)) (set (deref header 6) 0)
  (set s (write file header 28)) (if (not (= s 24)) (return -1)) (return 0))

(define (file-au-read-header file encoding samples-per-second channel-count)
  (b8-s b32-s b32* b32* b32*)
  ;when successful, the reader is positioned to the start of the audio data
  (define s ssize-t header[6] b32) (set s (read file header 24))
  (if (not (and (= s 24) (= (deref header) (__builtin-bswap32 779316836)))) (return -1))
  (set s (lseek file (__builtin-bswap32 (deref header 1)) SEEK_SET)) (if (< s 0) (return -1))
  (set (deref encoding) (__builtin-bswap32 (deref header 3))
    (deref samples-per-second) (__builtin-bswap32 (deref header 4))
    (deref channel-count) (__builtin-bswap32 (deref header 5)))
  (return 0))

(define (sp-io-file-open path open-flags channel-count samples-per-second)
  (SCM b32 SCM SCM SCM) scm-c-local-error-init
  (scm-c-local-error-assert "input"
    (and (scm-is-string path) (scm-is-integer open-flags)
      (scm-is-integer channel-count) (scm-is-integer samples-per-second)))
  (local-memory-init 1) init-status
  (define file b32-s r SCM)
  (define samples-per-second-c b32 channel-count-c b32)
  (define path-c char* (scm->locale-string path)) (local-memory-add path-c)
  (if (file-exists? path-c)
    (begin
      (set file
        (open path-c open-flags))
      (scm-c-require-success-glibc file) (define encoding b32)
      (set s
        (file-au-read-header file (address-of encoding)
          (address-of samples-per-second-c) (address-of channel-count-c)))
      (if s (scm-c-local-error "read-header" 0))
      (if (not (= encoding 6)) (scm-c-local-error "wrong-encoding" (scm-from-uint32 encoding)))
      (set r
        (sp-port-create sp-port-type-file open-flags samples-per-second-c channel-count-c file)))
    (begin (set file (open path-c (bit-or open-flags O_CREAT) 384))
      (scm-c-require-success-glibc file)
      (set samples-per-second-c (optional-samples-per-second samples-per-second))
      (set channel-count-c (scm->uint32 channel-count))
      (set s (file-au-write-header file 6 samples-per-second-c channel-count-c))
      (if (< s 0) (scm-c-local-error "write-header" 0))
      (set r
        (sp-port-create sp-port-type-file open-flags samples-per-second-c channel-count-c file))))
  local-memory-free (return r) (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-file-open-input path) (SCM SCM)
  (return (sp-io-file-open path O_RDONLY SCM-UNDEFINED SCM-UNDEFINED))
  )

(define (scm-sp-io-file-open-output path channel-count samples-per-second) (SCM SCM SCM SCM)
  (return (sp-io-file-open path O_WRONLY channel-count samples-per-second))
  )

(define (scm-sp-io-alsa-write port sample-count channel-data) (SCM SCM SCM)
  init-status scm-c-local-error-init
  (local-memory-init 2)
  (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (define sample-count b32 (scm->uint32 sample-count))
  (scm-c-local-define-malloc+size channel-data-c f32-s* (* channel-count (sizeof pointer)))
  (local-memory-add channel-data-c) (define index 0)
  (define e SCM)
  (scm-c-list-each channel-data e
    (compound-statement
      (set (deref channel-data-c index) (convert-type (SCM-BYTEVECTOR-CONTENTS e) f32-s*))
      (increment-one index)))
  (define frames-written snd_pcm_sframes_t)
  (define retry-count b8 20)
  (label loop
    (if retry-count
      (begin
        (set frames-written
          (snd-pcm-writen (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
            channel-data-c sample-count))
        (if (= frames-written -EAGAIN) (begin (decrement-one retry-count) (goto loop))
          (if (< frames-written 0)
            (if
              (<
                (snd-pcm-recover (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
                  frames-written 0)
                0)
              (scm-c-local-error "alsa" (scm-from-locale-string (snd-strerror frames-written)))))))
      (scm-c-local-error "max-retries-made" 0)))
  (return SCM-BOOL-T) (label error local-memory-free scm-c-local-error-return))
)

(define (scm-sp-io-alsa-read port sample-count) (SCM SCM SCM)
  init-status scm-c-local-error-init
  (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (define sample-count b32 (scm->uint32 sample-count))
  (scm-c-local-define-malloc+size channel-data f32-s* (* channel-count (sizeof pointer)))
  (local-memory-add channel-data)
  (define index b32 channel-count)
  (define data f32-s*)
  (while index
    (decrement-one index)
    (set data (malloc (* sample-count (sizeof f32-s))))
    (if (not data) (scm-c-local-error "memory" 0))
    (set (deref channel-data index) data)
    )
  (define frames-read snd_pcm_sframes_t)
  (define retry-count b8 20)
  (label loop
    (if retry-count
      (begin
        (set frames-read
          (snd-pcm-readn (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
            channel-data-c sample-count))
        (if (= frames-read -EAGAIN) (begin (decrement-one retry-count) (goto loop))
          (if (< frames-read 0)
            (if
              (<
                (snd-pcm-recover (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
                  frames-read 0)
                0)
              (scm-c-local-error "alsa" (scm-from-locale-string (snd-strerror frames-read)))))))
      (scm-c-local-error "max-retries-made" 0)))
  (define index b32 channel-count)
  (while index (decrement-one index)
    (scm-cons (scm-take-f32vector (deref channel-data index) sample-count)))
  (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-file-write port sample-count channel-data) (SCM SCM SCM)
  init-status scm-c-local-error-init
  (local-memory-init 2) (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (define sample-count b32 (scm->uint32 sample-count))
  (scm-c-local-define-malloc+size channel-data-c f32-s* (* channel-count (sizeof pointer)))
  (local-memory-add channel-data-c) (define index 0)
  (define e SCM)
  (scm-c-list-each channel-data e
    (compound-statement
      (set (deref channel-data-c index) (convert-type (SCM-BYTEVECTOR-CONTENTS e) f32-s*))
      (increment-one index)))
  (scm-c-local-define-malloc+size data-interleaved f32-s
    (* segment-size channel-count (sizeof f32-s)))
  (local-memory-add data-interleaved)
  (sp-interleave-n data-interleaved channel-data-c channel-count sample-count)
  (scm-c-require-success-glibc
    (write (convert-type (struct-ref (deref port-data) data) int) data-interleaved
      (* channel-count sample-count)))
  local-memory-free (return SCM-BOOL-T) (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-file-read port sample-count) (SCM SCM)
  init-status scm-c-local-error-init
  (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (define sample-count b32 (scm->uint32 sample-count))
  (scm-c-local-define-malloc+size channel-data f32-s* (* channel-count (sizeof pointer)))
  (local-memory-add channel-data)
  (scm-c-local-define-malloc+size data-interleaved f32-s (* sample-count channel-count))
  (local-memory-add data-interleaved)
  (scm-c-require-success-glibc
    (read (convert-type (struct-ref (deref port-data) data) int) data-interleaved
      (* sample-count channel-count)))
  (sp-deinterleave-n channel-data data-interleaved channel-count sample-count) (define r SCM-EOL)
  (define index b32 channel-count)
  (while index (decrement-one index)
    (scm-cons (scm-take-f32vector (deref channel-data index) sample-count)))
  (return r))
