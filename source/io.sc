;uses a custom type for ports because guiles port api is underdocumented / too complicated for now
(includep "fcntl.h")
(define-macro (optional-samples-per-second a) (if* (= SCM-UNDEFINED a) 96000 (scm->uint32 a)))
(define-macro (optional-channel-count a) (if* (= SCM-UNDEFINED a) 1 (scm->uint32 a)))
(define-macro (scm-c-sp-port? a) (SCM-SMOB-PREDICATE sp-port-scm-type a))

(define-macro (define-sp-interleave name get-source-element)
  (define (name interleaved non-interleaved channel-count non-interleaved-size)
    (b0 f32-s* f32-s** b32 b32) (define interleaved-size b32 (* non-interleaved-size channel-count))
    (define current-channel b32)
    (while non-interleaved-size (decrement-one non-interleaved-size)
      (set current-channel channel-count)
      (while current-channel (decrement-one current-channel)
        (decrement-one interleaved-size)
        (set (deref interleaved interleaved-size) get-source-element)))))

(define-macro (define-sp-deinterleave name get-source-element)
  (define (name non-interleaved interleaved channel-count non-interleaved-size)
    (b0 f32-s** f32-s* b32 b32) (define interleaved-size b32 (* non-interleaved-size channel-count))
    (define current-channel b32)
    (while non-interleaved-size (decrement-one non-interleaved-size)
      (set current-channel channel-count)
      (while current-channel (decrement-one current-channel)
        (decrement-one interleaved-size)
        (set (deref (deref non-interleaved current-channel) non-interleaved-size)
          get-source-element)))))

(define-sp-deinterleave sp-deinterleave-n+swap-endian
  (__builtin-bswap32 (deref interleaved interleaved-size)))

(define-sp-deinterleave sp-deinterleave-n (deref interleaved interleaved-size))

(define-sp-interleave sp-interleave-n
  (deref (deref non-interleaved current-channel) non-interleaved-size))

(define-sp-interleave sp-interleave-n+swap-endian
  (__builtin-bswap32 (deref (deref non-interleaved current-channel) non-interleaved-size)))

(define-type port-data-t
  (struct (samples-per-second b32) (channel-count b32)
    (input? b8) (type b8)
    (closed? b8) (position? b8) (position b64) (position-offset b16) (data pointer)))

(define-macro sp-port-type-alsa 0)
(define-macro sp-port-type-file 1)
(define sp-port-scm-type scm-t-bits)

(define-macro (sp-port-type->name a)
  ;integer -> string
  (cond* ((= sp-port-type-file a) "file") ((= sp-port-type-alsa a) "alsa") (else "unknown")))

(define-macro (sp-port->port-data a) (convert-type (SCM-SMOB-DATA a) port-data-t*))

(define (sp-port-print a output-port print-state) (int SCM SCM scm-print-state*)
  (define port-data port-data-t* (convert-type (SCM-SMOB-DATA a) port-data-t*)) (define r[255] char)
  (sprintf r "#<sp-port %lx type:%s samples-per-second:%d channel-count:%d closed?:%s input?:%s>"
    (convert-type a pointer) (sp-port-type->name (struct-ref (deref port-data) type))
    (struct-ref (deref port-data) samples-per-second) (struct-ref (deref port-data) channel-count)
    (if* (struct-ref (deref port-data) closed?) "#t" "#f")
    (if* (struct-ref (deref port-data) input?) "#t" "#f"))
  (scm-display (scm-from-locale-string r) output-port) (return 0))

(define-macro sp-port-scm-type-init
  ;create and setup the type
  (set sp-port-scm-type (scm-make-smob-type "sp-port" 0))
  (scm-set-smob-print sp-port-scm-type sp-port-print))

(define
  (sp-port-create type input? samples-per-second channel-count position? position-offset data)
  (SCM b8 b8 b32 b32 b8 b16 pointer)
  (define port-data port-data-t* (scm-gc-malloc (sizeof port-data-t) "sp-port-data"))
  (struct-set (deref port-data) channel-count
    channel-count samples-per-second
    samples-per-second type
    type input? input? data data position? position? position 0 position-offset position-offset)
  (return (scm-new-smob sp-port-scm-type (convert-type port-data scm-t-bits))))

(define-macro (scm-c-require-success-alsa a) (set s a)
  (if s (scm-c-local-error "alsa" (scm-from-locale-string (snd-strerror s)))))

(define (scm-sp-io-port-close a) (SCM SCM)
  scm-c-local-error-init (define port-data port-data-t*)
  init-status
  (if (scm-c-sp-port? a)
    (begin (set port-data (convert-type (SCM-SMOB-DATA a) port-data-t*))
      (if (struct-ref (deref port-data) closed?) (scm-c-local-error "already-closed" 0)
        (begin
          (cond
            ( (= sp-port-type-alsa (struct-ref (deref port-data) type))
              (scm-c-require-success-alsa
                (snd-pcm-close (convert-type (struct-ref (deref port-data) data) snd-pcm-t*))))
            ( (= sp-port-type-file (struct-ref (deref port-data) type))
              (scm-c-require-success-system
                (close (convert-type (struct-ref (deref port-data) data) b32)))))
          (struct-set (deref port-data) closed? 1))))
    (scm-c-local-error "input" 0))
  (return SCM-BOOL-T) (label error scm-c-local-error-return))

(define (sp-io-alsa-open input? device-name channel-count samples-per-second latency)
  (SCM b8 SCM SCM SCM SCM) scm-c-local-error-init
  (local-memory-init 1) init-status
  (define alsa-port snd-pcm-t*) (define device-name-c char*)
  (scm-if-undefined device-name (set device-name-c "default")
    (begin (set device-name-c (scm->locale-string device-name)) (local-memory-add device-name-c)))
  (scm-c-require-success-alsa
    (snd-pcm-open (address-of alsa-port) device-name-c
      (if* input? SND_PCM_STREAM_CAPTURE SND_PCM_STREAM_PLAYBACK) 0))
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
      samples-per-second-c channel-count-c 0 0 (convert-type alsa-port pointer)))
  (label error (if alsa-port (snd-pcm-close alsa-port)) local-memory-free scm-c-local-error-return))

(define (scm-sp-io-alsa-open-input device-name channel-count samples-per-second latency)
  (SCM SCM SCM SCM SCM)
  (return (sp-io-alsa-open 1 device-name channel-count samples-per-second latency)))

(define (scm-sp-io-alsa-open-output device-name channel-count samples-per-second latency)
  (SCM SCM SCM SCM SCM)
  (return (sp-io-alsa-open 0 device-name channel-count samples-per-second latency)))

(define (file-au-write-header file encoding samples-per-second channel-count)
  (b8-s b32-s b32 b32 b32)
  ;assumes file to be positioned at the beginning
  (define s ssize-t header[7] b32) (set (deref header) (__builtin-bswap32 779316836))
  (set (deref header 1) (__builtin-bswap32 28)) (set (deref header 2) (__builtin-bswap32 4294967295))
  (set (deref header 3) (__builtin-bswap32 encoding))
  (set (deref header 4) (__builtin-bswap32 samples-per-second))
  (set (deref header 5) (__builtin-bswap32 channel-count)) (set (deref header 6) 0)
  (set s (write file header 28)) (if (not (= s 28)) (return -1)) (return 0))

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

(define (sp-io-file-open path open-flags channel-count samples-per-second) (SCM SCM b32 SCM SCM)
  scm-c-local-error-init (local-memory-init 1)
  init-status (define file b32-s r SCM)
  (define samples-per-second-c b32 channel-count-c b32)
  (define path-c char* (scm->locale-string path)) (local-memory-add path-c)
  (if (file-exists? path-c)
    (begin (set file (open path-c open-flags)) (scm-c-require-success-system file)
      (define encoding b32)
      (set s
        (file-au-read-header file (address-of encoding)
          (address-of samples-per-second-c) (address-of channel-count-c)))
      (if s (scm-c-local-error "read-header" 0))
      (if (not (= encoding 6)) (scm-c-local-error "wrong-encoding" (scm-from-uint32 encoding)))
      (if
        (not (or (scm-is-undefined channel-count) (= channel-count-c (scm->uint32 channel-count))))
        (scm-c-local-error "incompatible"
          (scm-from-locale-string
            "file exists but channel count is different from what was requested")))
      (if
        (not
          (or (scm-is-undefined samples-per-second)
            (= samples-per-second-c (scm->uint32 samples-per-second))))
        (scm-c-local-error "incompatible"
          (scm-from-locale-string
            "file exists but samples per second are different from what was requested")))
      (set r
        (sp-port-create sp-port-type-file (bit-and open-flags O_RDONLY)
          samples-per-second-c channel-count-c 1 (lseek file 0 SEEK-CUR) file)))
    (begin (set file (open path-c (bit-or open-flags O_CREAT) 384))
      (scm-c-require-success-system file)
      (set samples-per-second-c (optional-samples-per-second samples-per-second))
      (set channel-count-c (optional-channel-count channel-count))
      (set s (file-au-write-header file 6 samples-per-second-c channel-count-c))
      (if (< s 0) (scm-c-local-error "write-header" 0))
      (set r
        (sp-port-create sp-port-type-file (bit-and open-flags O_RDONLY)
          samples-per-second-c channel-count-c 1 (lseek file 0 SEEK-CUR) file))))
  local-memory-free (return r) (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-file-open-input path) (SCM SCM)
  scm-c-local-error-init (scm-c-local-error-assert "type-check" (scm-is-string path))
  (return (sp-io-file-open path O_RDONLY SCM-UNDEFINED SCM-UNDEFINED))
  (label error scm-c-local-error-return))

(define (scm-sp-io-file-open-output path channel-count samples-per-second) (SCM SCM SCM SCM)
  scm-c-local-error-init
  (scm-c-local-error-assert "type-check"
    (and (scm-is-string path) (or (= SCM-UNDEFINED channel-count) (scm-is-integer channel-count))
      (or (= SCM-UNDEFINED samples-per-second) (scm-is-integer samples-per-second))))
  (return (sp-io-file-open path O_WRONLY channel-count samples-per-second))
  (label error scm-c-local-error-return))

(define (scm-sp-io-alsa-write port channel-data sample-count) (SCM SCM SCM SCM)
  scm-c-local-error-init (local-memory-init 1)
  (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (define sample-count-c b32
    (scm->uint32
      (if* (= SCM-UNDEFINED sample-count) (scm-f32vector-length channel-data) sample-count)))
  (scm-c-local-define-malloc+size channel-data-c f32-s* (* channel-count (sizeof pointer)))
  (local-memory-add channel-data-c) (define index b32 0)
  (define e SCM)
  (scm-c-list-each channel-data e
    (compound-statement
      (set (deref channel-data-c index) (convert-type (SCM-BYTEVECTOR-CONTENTS e) f32-s*))
      (increment-one index)))
  (define frames-written snd_pcm_sframes_t) (define retry-count b8 20)
  (label loop
    (if retry-count
      (begin
        (set frames-written
          (snd-pcm-writen (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
            (convert-type channel-data-c b0**) (convert-type sample-count-c snd_pcm_uframes_t)))
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

(define (scm-sp-io-alsa-read port sample-count) (SCM SCM SCM)
  scm-c-local-error-init (local-memory-init 1)
  (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count-c b32 (struct-ref (deref port-data) channel-count))
  (define sample-count-c b32 (scm->uint32 sample-count))
  (scm-c-local-define-malloc+size channel-data f32-s* (* channel-count-c (sizeof pointer)))
  (local-memory-add channel-data) (define index b32 channel-count-c)
  (define data f32-s*)
  (while index (decrement-one index)
    (set data (malloc (* sample-count-c (sizeof f32-s))))
    (if (not data) (scm-c-local-error "memory" 0)) (set (deref channel-data index) data))
  (define frames-read snd_pcm_sframes_t) (define retry-count b8 20)
  (label loop
    (if retry-count
      (begin
        (set frames-read
          (snd-pcm-readn (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
            (convert-type channel-data b0**) sample-count-c))
        (if (= frames-read -EAGAIN) (begin (decrement-one retry-count) (goto loop))
          (if (< frames-read 0)
            (if
              (<
                (snd-pcm-recover (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
                  frames-read 0)
                0)
              (scm-c-local-error "alsa" (scm-from-locale-string (snd-strerror frames-read)))))))
      (scm-c-local-error "max-retries-made" 0)))
  (set index channel-count-c) (define r SCM SCM-EOL)
  (while index (decrement-one index)
    (set r (scm-cons (scm-take-f32vector (deref channel-data index) sample-count-c) r)))
  local-memory-free (return r) (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-file-write port channel-data sample-count) (SCM SCM SCM SCM)
  scm-c-local-error-init (define port-data port-data-t* (sp-port->port-data port))
  (scm-c-local-error-assert "argument-types"
    (and (scm-c-sp-port? port) (scm-is-true (scm-list? channel-data))
      (or (scm-is-undefined sample-count) (scm-is-integer sample-count))))
  (scm-c-local-error-assert "not-an-output-port" (not (struct-ref (deref port-data) input?)))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (scm-c-local-error-assert "channel-data-length"
    (= (scm->uint32 (scm-length channel-data)) channel-count))
  (define sample-count-c b64
    (scm->uint64
      (if* (= SCM-UNDEFINED sample-count) (scm-f32vector-length (scm-first channel-data))
        sample-count)))
  (scm-c-local-define-malloc+size channel-data-c f32-s* (* channel-count (sizeof pointer)))
  (local-memory-init 2) (local-memory-add channel-data-c)
  (define index b32 0) (define e SCM)
  (scm-c-list-each channel-data e
    (compound-statement
      (set (deref channel-data-c index) (convert-type (SCM-BYTEVECTOR-CONTENTS e) f32-s*))
      (increment-one index)))
  (scm-c-local-define-malloc+size data-interleaved f32-s (* sample-count-c channel-count 4))
  (local-memory-add data-interleaved)
  (sp-interleave-n+swap-endian data-interleaved channel-data-c channel-count sample-count-c)
  (scm-c-require-success-system
    (write (convert-type (struct-ref (deref port-data) data) int) data-interleaved
      (* channel-count sample-count-c 4)))
  local-memory-free (return SCM-BOOL-T) (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-file-read port sample-count) (SCM SCM SCM)
  (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (define sample-count-c b64 (scm->uint64 sample-count)) scm-c-local-error-init
  (local-memory-init 2)
  (scm-c-local-define-malloc+size data-interleaved f32-s (* sample-count-c channel-count 4))
  (local-memory-add data-interleaved)
  (scm-c-require-success-system
    (read (convert-type (struct-ref (deref port-data) data) int) data-interleaved
      (* sample-count-c channel-count 4)))
  ;prepare non-interleaved memory
  (scm-c-local-define-malloc+size data-non-interleaved f32-s* (* channel-count (sizeof pointer)))
  (local-memory-add data-non-interleaved) (define index b32 channel-count)
  (while index (decrement-one index)
    (set (deref data-non-interleaved index) (malloc (* sample-count-c 4)))
    (if (not (deref data-non-interleaved index)) (scm-c-local-error "memory" 0)))
  ;deinterleave
  (sp-deinterleave-n+swap-endian data-non-interleaved data-interleaved channel-count sample-count-c)
  ;create f32vectors
  (define r SCM SCM-EOL) (set index channel-count)
  (while index (decrement-one index)
    (set r (scm-cons (scm-take-f32vector (deref data-non-interleaved index) sample-count-c) r)))
  local-memory-free (return r) (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-port-input? port) (SCM SCM)
  (return (scm-from-bool (struct-ref (deref (sp-port->port-data port)) input?))))

(define (scm-sp-io-port-position port) (SCM SCM)
  (return
    (if* (struct-ref (deref (sp-port->port-data port)) position?)
      (scm-from-uint64 (* (struct-ref (deref (sp-port->port-data port)) position) 0.25)) SCM-BOOL-F)))

(define (scm-sp-io-port-position? port) (SCM SCM)
  (return (scm-from-bool (struct-ref (deref (sp-port->port-data port)) position))))

(define (scm-sp-io-port-channel-count port) (SCM SCM)
  (return (scm-from-uint32 (struct-ref (deref (sp-port->port-data port)) channel-count))))

(define (scm-sp-io-port-samples-per-second port) (SCM SCM)
  (return (scm-from-uint32 (struct-ref (deref (sp-port->port-data port)) samples-per-second))))

(define (scm-sp-io-port? port) (SCM SCM) (return (if* (scm-c-sp-port? port) SCM-BOOL-T SCM-BOOL-F)))

(define (scm-sp-io-file-set-position port sample-position) (SCM SCM SCM)
  scm-c-local-error-init (define port-data port-data-t* (sp-port->port-data port))
  (define position-c b64-s (* (scm->int64 sample-position) 4)) (define s off-t)
  (let-macro
    (file (convert-type (struct-ref (deref port-data) data) int)
      position-offset (struct-ref (deref port-data) position-offset))
    (if (>= position-c 0) (set s (lseek file (+ position-offset position-c) SEEK_SET))
      (begin (define end-position off-t (lseek file 0 SEEK_END))
        (if (< end-position 0)
          (scm-c-local-error "lseek" (scm-from-locale-string (strerror errno))))
        (set position-c (+ end-position position-c))
        (if (>= position-c position-offset) (set s (lseek file position-c SEEK_SET))
          (scm-c-local-error "invalid position" 0)))))
  (if (< s 0) (scm-c-local-error "lseek" (scm-from-locale-string (strerror errno)))
    (begin (set (struct-ref (deref port-data) position) position-c) (return SCM-BOOL-T)))
  (label error scm-c-local-error-return))