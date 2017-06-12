;uses a custom type for ports because guiles port api is underdocumented / too complicated for now
(includep "fcntl.h")
(define-macro sp-port-type-alsa 0)
(define-macro sp-port-type-file 1)
(define sp-port-scm-type scm-t-bits)
(define scm-sp-port-type-alsa SCM)
(define scm-sp-port-type-file SCM)
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

(define (word-octets-reverse-f32 a) (f32-s f32-s)
  (define r f32-s) (define b b8* (convert-type (address-of a) b8*))
  (define c b8* (convert-type (address-of r) b8*)) (set (deref c) (deref b 3))
  (set (deref c 1) (deref b 2)) (set (deref c 2) (deref b 1)) (set (deref c 3) (deref b)) (return r))

(define-sp-deinterleave sp-deinterleave-n+swap-endian
  (word-octets-reverse-f32 (deref interleaved interleaved-size)))

(define-sp-deinterleave sp-deinterleave-n (deref interleaved interleaved-size))

(define-sp-interleave sp-interleave-n
  (deref (deref non-interleaved current-channel) non-interleaved-size))

(define-sp-interleave sp-interleave-n+swap-endian
  (word-octets-reverse-f32 (deref (deref non-interleaved current-channel) non-interleaved-size)))

(define-type port-data-t
  ;type is any of the sp-port-type-* values
  ;position? is true when the port supports random access
  ;position-offset stores the number of octets between the start of the file and the beginning of sample data - the header length
  (struct (samples-per-second b32) (channel-count b32)
    (input? b8) (type b8)
    (closed? b8) (position? b8) (position b64) (position-offset b16) (data pointer)))

(define-macro (sp-port-type->name a)
  ;integer -> string
  (cond* ((= sp-port-type-file a) "file") ((= sp-port-type-alsa a) "alsa") (else "unknown")))

(define-macro (sp-port->port-data a) (convert-type (SCM-SMOB-DATA a) port-data-t*))

(define (sp-port-print a output-port print-state) (int SCM SCM scm-print-state*)
  (define port-data port-data-t* (convert-type (SCM-SMOB-DATA a) port-data-t*))
  ;calculated maximum length with 64 bit pointers and 999 types
  (define r char* (malloc 114))
  (sprintf r "#<sp-port %lx type:%s samples-per-second:%d channel-count:%d closed?:%s input?:%s>"
    (convert-type a pointer) (sp-port-type->name (struct-ref (deref port-data) type))
    (struct-ref (deref port-data) samples-per-second) (struct-ref (deref port-data) channel-count)
    (if* (struct-ref (deref port-data) closed?) "#t" "#f")
    (if* (struct-ref (deref port-data) input?) "#t" "#f"))
  (scm-display (scm-take-locale-string r) output-port) (return 0))

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

(define (scm-sp-port-close a) (SCM SCM)
  scm-c-local-error-init init-status
  (define port-data port-data-t*)
  (if (scm-c-sp-port? a)
    (begin (set port-data (sp-port->port-data a))
      (if (struct-ref (deref port-data) closed?) (scm-c-local-error "is-closed" 0)
        (let-macro (type (struct-ref (deref port-data) type))
          (cond
            ( (= sp-port-type-alsa type)
              (scm-c-require-success-alsa
                (snd-pcm-close (convert-type (struct-ref (deref port-data) data) snd-pcm-t*))))
            ( (= sp-port-type-file type)
              (scm-c-require-success-system
                (close (convert-type (struct-ref (deref port-data) data) int)))))
          (struct-set (deref port-data) closed? #t))))
    (scm-c-local-error "type-check" 0))
  (return SCM-BOOL-T) (label error scm-c-local-error-return))

(define-macro default-alsa-enable-soft-resample 1)
(define-macro default-alsa-latency 50)
(define-macro default-samples-per-second 16000)
(define-macro default-channel-count 1)

(define (sp-io-alsa-open input? device-name channel-count samples-per-second latency)
  (SCM b8 SCM SCM SCM SCM) scm-c-local-error-init
  (local-memory-init 1) init-status
  (define alsa-port snd-pcm-t* 0) (define device-name-c char*)
  (scm-if-undefined device-name (set device-name-c "default")
    (begin (set device-name-c (scm->locale-string device-name)) (local-memory-add device-name-c)))
  (scm-c-require-success-alsa
    (snd-pcm-open (address-of alsa-port) device-name-c
      (if* input? SND_PCM_STREAM_CAPTURE SND_PCM_STREAM_PLAYBACK) 0))
  (define latency-c b32 (scm-if-undefined-expr latency default-alsa-latency (scm->uint32 latency)))
  (define channel-count-c b32 (scm-if-undefined-expr channel-count 1 (scm->uint32 channel-count)))
  (define samples-per-second-c b32
    (scm-if-undefined-expr samples-per-second default-samples-per-second
      (scm->uint32 samples-per-second)))
  (scm-c-require-success-alsa
    (snd-pcm-set-params alsa-port SND_PCM_FORMAT_FLOAT_LE
      SND_PCM_ACCESS_RW_NONINTERLEAVED channel-count-c
      samples-per-second-c default-alsa-enable-soft-resample latency-c))
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
  (b8-s int b32 b32 b32)
  ;assumes file to be positioned at the beginning
  (define s ssize-t header[7] b32) (set (deref header) (__builtin-bswap32 779316836))
  (set (deref header 1) (__builtin-bswap32 28)) (set (deref header 2) (__builtin-bswap32 4294967295))
  (set (deref header 3) (__builtin-bswap32 encoding))
  (set (deref header 4) (__builtin-bswap32 samples-per-second))
  (set (deref header 5) (__builtin-bswap32 channel-count)) (set (deref header 6) 0)
  (set s (write file header 28)) (if (not (= s 28)) (return -1)) (return 0))

(define (file-au-read-header file encoding samples-per-second channel-count)
  (b8-s int b32* b32* b32*)
  ;when successful, the reader is positioned at the start of the audio data
  (define s ssize-t header[6] b32) (set s (read file header 24))
  (if (not (and (= s 24) (= (deref header) (__builtin-bswap32 779316836)))) (return -1))
  (if (< (lseek file (__builtin-bswap32 (deref header 1)) SEEK_SET) 0) (return -1))
  (set (deref encoding) (__builtin-bswap32 (deref header 3))
    (deref samples-per-second) (__builtin-bswap32 (deref header 4))
    (deref channel-count) (__builtin-bswap32 (deref header 5)))
  (return 0))

(define (sp-io-file-open path input? channel-count samples-per-second) (SCM SCM b8 SCM SCM)
  (define file int r SCM samples-per-second-file b32 channel-count-file b32)
  (define path-c char* (scm->locale-string path)) scm-c-local-error-init
  (local-memory-init 1) (local-memory-add path-c)
  (if (file-exists? path-c)
    (begin (set file (open path-c O_RDWR)) (scm-c-require-success-system file)
      (define encoding b32)
      (if
        (file-au-read-header file (address-of encoding)
          (address-of samples-per-second-file) (address-of channel-count-file))
        (scm-c-local-error "header-read" 0))
      (if (not (= encoding 6)) (scm-c-local-error "wrong-encoding" (scm-from-uint32 encoding)))
      (if
        (not
          (or (scm-is-undefined channel-count) (= channel-count-file (scm->uint32 channel-count))))
        (scm-c-local-error "incompatible"
          (scm-from-locale-string
            "file exists but channel count is different from what was requested")))
      (if
        (not
          (or (scm-is-undefined samples-per-second)
            (= samples-per-second-file (scm->uint32 samples-per-second))))
        (scm-c-local-error "incompatible"
          (scm-from-locale-string
            "file exists but samples per second are different from what was requested")))
      (set r
        (sp-port-create sp-port-type-file input?
          samples-per-second-file channel-count-file #t (lseek file 0 SEEK-CUR) file)))
    (begin (set file (open path-c (bit-or O_RDWR O_CREAT) 384)) (scm-c-require-success-system file)
      (set samples-per-second-file
        (if* (scm-is-undefined samples-per-second) default-samples-per-second
          (scm->uint32 samples-per-second)))
      (set channel-count-file
        (if* (scm-is-undefined samples-per-second) default-channel-count
          (scm->uint32 channel-count)))
      (if (< (file-au-write-header file 6 samples-per-second-file channel-count-file) 0)
        (scm-c-local-error "header-write" 0))
      (set r
        (sp-port-create sp-port-type-file input?
          samples-per-second-file channel-count-file #t (lseek file 0 SEEK-CUR) file))))
  local-memory-free (return r) (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-file-open-input path) (SCM SCM)
  scm-c-local-error-init (scm-c-local-error-assert "type-check" (scm-is-string path))
  (return (sp-io-file-open path #t SCM-UNDEFINED SCM-UNDEFINED))
  (label error scm-c-local-error-return))

(define (scm-sp-io-file-open-output path channel-count samples-per-second) (SCM SCM SCM SCM)
  scm-c-local-error-init
  (scm-c-local-error-assert "type-check"
    (and (scm-is-string path) (or (scm-is-undefined channel-count) (scm-is-integer channel-count))
      (or (scm-is-undefined samples-per-second) (scm-is-integer samples-per-second))))
  (return (sp-io-file-open path #f channel-count samples-per-second))
  (label error scm-c-local-error-return))

(define (scm-sp-io-alsa-write port sample-count channel-data) (SCM SCM SCM SCM)
  scm-c-local-error-init
  (scm-c-local-error-assert "type-check"
    (and (scm-c-sp-port? port) (scm-is-true (scm-list? channel-data))
      (not (scm-is-null channel-data)) (scm-is-integer sample-count)))
  (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (define sample-count-c b32 (scm->uint32 sample-count)) (local-memory-init 2)
  (scm-c-local-define-malloc+size channel-data-c f32-s* (* channel-count (sizeof pointer)))
  (local-memory-add channel-data-c) (define index b32 0)
  (define e SCM)
  (scm-c-list-each channel-data e
    (compound-statement
      (set (deref channel-data-c index) (convert-type (SCM-BYTEVECTOR-CONTENTS e) f32-s*))
      (increment-one index)))
  (define frames-written snd_pcm_sframes_t)
  (set frames-written
    (snd-pcm-writen (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
      (convert-type channel-data-c b0**) (convert-type sample-count-c snd_pcm_uframes_t)))
  (if
    (and (< frames-written 0)
      (<
        (snd-pcm-recover (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
          frames-written 0)
        0))
    (scm-c-local-error "alsa" (scm-from-locale-string (snd-strerror frames-written))))
  local-memory-free (return SCM-BOOL-T) (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-alsa-read port sample-count) (SCM SCM SCM)
  (local-memory-init 1) scm-c-local-error-init
  (scm-c-local-error-assert "type-check" (and (scm-c-sp-port? port) (scm-is-integer sample-count)))
  (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count-c b32 (struct-ref (deref port-data) channel-count))
  (define sample-count-c b64 (scm->uint32 sample-count))
  (scm-c-local-define-malloc+size channel-data f32-s* (* channel-count-c (sizeof pointer)))
  (local-memory-add channel-data) (define index b32 channel-count-c)
  (define data f32-s*)
  (while index (decrement-one index)
    (set data (malloc (* sample-count-c (sizeof f32-s))))
    (if (not data) (scm-c-local-error "memory" 0)) (set (deref channel-data index) data))
  (define frames-read snd_pcm_sframes_t)
  (set frames-read
    (snd-pcm-readn (convert-type (struct-ref (deref port-data) data) snd-pcm-t*)
      (convert-type channel-data b0**) sample-count-c))
  (if
    (and (< frames-read 0)
      (<
        (snd-pcm-recover (convert-type (struct-ref (deref port-data) data) snd-pcm-t*) frames-read
          0)
        0))
    (scm-c-local-error "alsa" (scm-from-locale-string (snd-strerror frames-read))))
  (set index channel-count-c) (define r SCM SCM-EOL)
  (while index (decrement-one index)
    (set r (scm-cons (scm-take-f32vector (deref channel-data index) sample-count-c) r)))
  local-memory-free (return r) (label error local-memory-free scm-c-local-error-return))

(define (scm-sp-io-file-write port sample-count channel-data) (SCM SCM SCM SCM)
  scm-c-local-error-init (local-memory-init 2)
  (scm-c-local-error-assert "type-check"
    (and (scm-c-sp-port? port) (scm-is-true (scm-list? channel-data))
      (not (scm-is-null channel-data)) (scm-is-true (scm-f32vector? (scm-first channel-data)))
      (scm-is-integer sample-count)))
  (define port-data port-data-t* (sp-port->port-data port))
  (scm-c-local-error-assert "not-an-output-port" (not (struct-ref (deref port-data) input?)))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (scm-c-local-error-assert "channel-data-length-mismatch"
    (= (scm->uint32 (scm-length channel-data)) channel-count))
  (define sample-count-c b64 (scm->uint64 sample-count))
  (scm-c-local-define-malloc+size channel-data-c f32-s* (* channel-count (sizeof pointer)))
  (local-memory-add channel-data-c) (define index b32 0)
  (define e SCM)
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
  scm-c-local-error-init (local-memory-init 2)
  (scm-c-local-error-assert "type-check" (and (scm-c-sp-port? port) (scm-is-integer sample-count)))
  (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-ref (deref port-data) channel-count))
  (define sample-count-c b64 (scm->uint64 sample-count))
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

(define (scm-sp-port-input? port) (SCM SCM)
  ;type-check has been left out explicitly
  (return (scm-from-bool (struct-ref (deref (sp-port->port-data port)) input?))))

(define (scm-sp-port-position port) (SCM SCM)
  ;returns the current port position in octets
  (return
    (if* (struct-ref (deref (sp-port->port-data port)) position?)
      (scm-from-uint64 (* (struct-ref (deref (sp-port->port-data port)) position) 0.25)) SCM-BOOL-F)))

(define (scm-sp-port-position? port) (SCM SCM)
  (return (scm-from-bool (struct-ref (deref (sp-port->port-data port)) position))))

(define (scm-sp-port-channel-count port) (SCM SCM)
  (return (scm-from-uint32 (struct-ref (deref (sp-port->port-data port)) channel-count))))

(define (scm-sp-port-samples-per-second port) (SCM SCM)
  (return (scm-from-uint32 (struct-ref (deref (sp-port->port-data port)) samples-per-second))))

(define (scm-sp-port? port) (SCM SCM) (return (scm-from-bool (scm-c-sp-port? port))))

(define (scm-sp-port-type port) (SCM SCM)
  (let-macro (type (struct-ref (deref (sp-port->port-data port)) type))
    (return
      (cond* ((= type sp-port-type-alsa) scm-sp-port-type-alsa)
        ((= type sp-port-type-file) scm-sp-port-type-file) (else SCM-BOOL-F)))))

(define (scm-sp-io-file-set-position port sample-position) (SCM SCM SCM)
  scm-c-local-error-init
  (scm-c-local-error-assert "type-check"
    (and (scm-c-sp-port? port) (scm-is-integer sample-position)))
  (define port-data port-data-t* (sp-port->port-data port))
  (define position-c b64-s (* (scm->int64 sample-position) 4))
  (let-macro
    (file (convert-type (struct-ref (deref port-data) data) int)
      position-offset (struct-ref (deref port-data) position-offset))
    (if (>= position-c 0)
      (scm-c-require-success-system (lseek file (+ position-offset position-c) SEEK_SET))
      (begin (define end-position off-t (lseek file 0 SEEK_END))
        (scm-c-require-success-system end-position) (set position-c (+ end-position position-c))
        (if (>= position-c position-offset)
          (scm-c-require-success-system (lseek file position-c SEEK_SET))
          (scm-c-local-error "invalid position" 0)))))
  (set (struct-ref (deref port-data) position) position-c) (return SCM-BOOL-T)
  (label error scm-c-local-error-return))