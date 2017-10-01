(pre-define (optional-samples-per-second a) (if* (= SCM-UNDEFINED a) 96000 (scm->uint32 a)))
(pre-define (optional-channel-count a) (if* (= SCM-UNDEFINED a) 1 (scm->uint32 a)))
(pre-define (scm-c-sp-port? a) (SCM-SMOB-PREDICATE sp-port-scm-type a))

(pre-define (sp-alsa-status-require! expression) (status-set-id expression)
  (if status-failure? (status-set-group-goto sp-status-group-alsa)))

(pre-define default-alsa-enable-soft-resample 1
  default-alsa-latency 50
  default-samples-per-second 16000 default-channel-count 1 sp-port-type-alsa 0 sp-port-type-file 1)

(define sp-port-scm-type scm-t-bits)
(define scm-sp-port-type-alsa SCM)
(define scm-sp-port-type-file SCM)

(define (sample-reverse-endian a) (sp-sample-t sp-sample-t)
  "reverse the byte order for one sample" (define result sp-sample-t)
  (define b b8* (convert-type (address-of a) b8*))
  (define c b8* (convert-type (address-of result) b8*)) (set (deref c) (deref b 3))
  (set (deref c 1) (deref b 2)) (set (deref c 2) (deref b 1))
  (set (deref c 3) (deref b)) (return result))

(pre-define (define-sp-interleave name get-source-element)
  (define (name deinterleaved interleaved channel-count deinterleaved-size)
    (b0 sp-sample-t** sp-sample-t* b32 b32)
    (define interleaved-size b32 (* deinterleaved-size channel-count)) (define current-channel b32)
    (while deinterleaved-size (decrement deinterleaved-size)
      (set current-channel channel-count)
      (while current-channel (decrement current-channel)
        (decrement interleaved-size) (set (deref interleaved interleaved-size) get-source-element)))))

(pre-define (define-sp-deinterleave name get-source-element)
  (define (name interleaved deinterleaved channel-count deinterleaved-size)
    (b0 sp-sample-t* sp-sample-t** b32 b32)
    (define interleaved-size b32 (* deinterleaved-size channel-count)) (define current-channel b32)
    (while deinterleaved-size (decrement deinterleaved-size)
      (set current-channel channel-count)
      (while current-channel (decrement current-channel)
        (decrement interleaved-size)
        (set (deref (deref deinterleaved current-channel) deinterleaved-size) get-source-element)))))

(define-sp-deinterleave sp-deinterleave (deref interleaved interleaved-size))

(define-sp-interleave sp-interleave
  (deref (deref deinterleaved current-channel) deinterleaved-size))

(define-sp-deinterleave sp-deinterleave-and-reverse-endian
  (sample-reverse-endian (deref interleaved interleaved-size)))

(define-sp-interleave sp-interleave-and-reverse-endian
  (sample-reverse-endian (deref (deref deinterleaved current-channel) deinterleaved-size)))

(define-type port-data-t
  ; type is any of the sp-port-type-* values.
  ; position? is true when the port supports random access.
  ; position-offset stores the number of octets between the start
  ; of the file and the beginning of sample data - the header length
  (struct (samples-per-second b32) (channel-count b32)
    (input? b8) (type b8)
    (closed? b8) (position? b8) (position b64) (position-offset b16) (data b0*) (data-int int)))

(pre-define (sp-port-type->name a) "integer -> string"
  (cond* ((= sp-port-type-file a) "file") ((= sp-port-type-alsa a) "alsa") (else "unknown")))

(pre-define (sp-port->port-data a) (convert-type (SCM-SMOB-DATA a) port-data-t*))

(define (sp-port-print a output-port print-state) (int SCM SCM scm-print-state*)
  (define port-data port-data-t* (convert-type (SCM-SMOB-DATA a) port-data-t*))
  ; calculated maximum length with 64 bit pointers and 999 types
  (define r char* (malloc 114))
  (sprintf r "#<sp-port %lx type:%s samples-per-second:%d channel-count:%d closed?:%s input?:%s>"
    (convert-type a b0*) (sp-port-type->name (struct-pointer-get port-data type))
    (struct-pointer-get port-data samples-per-second) (struct-pointer-get port-data channel-count)
    (if* (struct-pointer-get port-data closed?) "#t" "#f")
    (if* (struct-pointer-get port-data input?) "#t" "#f"))
  (scm-display (scm-take-locale-string r) output-port) (return 0))

(pre-define sp-port-scm-type-init
  ; initialise the sp port type
  (set sp-port-scm-type (scm-make-smob-type "sp-port" 0))
  (scm-set-smob-print sp-port-scm-type sp-port-print))

(define
  (sp-port-create type input? samples-per-second channel-count position? position-offset data
    data-int)
  (SCM b8 b8 b32 b32 b8 b16 b0* int)
  "integer boolean integer integer boolean integer pointer integer -> sp-port"
  (define port-data port-data-t* (scm-gc-malloc (sizeof port-data-t) "sp-port-data"))
  (struct-pointer-set port-data channel-count
    channel-count samples-per-second
    samples-per-second type
    type input?
    input? data data data-int data-int position? position? position 0 position-offset position-offset)
  (return (scm-new-smob sp-port-scm-type (convert-type port-data scm-t-bits))))

(define (scm-sp-port-close a) (SCM SCM)
  status-init (define port-data port-data-t*)
  (if (scm-c-sp-port? a)
    (begin (set port-data (sp-port->port-data a))
      (if (struct-pointer-get port-data closed?)
        (status-set-both-goto sp-status-group-sp sp-status-id-port-closed)
        (pre-let (type (struct-pointer-get port-data type))
          (cond
            ( (= sp-port-type-alsa type)
              (sp-alsa-status-require!
                (snd-pcm-close (convert-type (struct-pointer-get port-data data) snd-pcm-t*))))
            ( (= sp-port-type-file type)
              (sp-system-status-require! (close (struct-pointer-get port-data data-int)))))
          (struct-pointer-set port-data closed? #t))))
    (status-set-both-goto sp-status-group-sp sp-status-id-input-type))
  (label exit (status->scm-return SCM-BOOL-T)))

(define (sp-io-alsa-open input? device-name channel-count samples-per-second latency)
  (SCM b8 SCM SCM SCM SCM) status-init
  (local-memory-init 1) (define alsa-port snd-pcm-t* 0)
  (define device-name-c char*)
  (if (scm-is-undefined device-name) (set device-name-c "default")
    (begin (set device-name-c (scm->locale-string device-name)) (local-memory-add device-name-c)))
  (sp-alsa-status-require!
    (snd-pcm-open (address-of alsa-port) device-name-c
      (if* input? SND_PCM_STREAM_CAPTURE SND_PCM_STREAM_PLAYBACK) 0))
  (define latency-c b32 (if* (scm-is-undefined latency) default-alsa-latency (scm->uint32 latency)))
  (define channel-count-c b32 (if* (scm-is-undefined channel-count) 1 (scm->uint32 channel-count)))
  (define samples-per-second-c b32
    (if* (scm-is-undefined samples-per-second) default-samples-per-second
      (scm->uint32 samples-per-second)))
  (sp-alsa-status-require!
    (snd-pcm-set-params alsa-port SND_PCM_FORMAT_FLOAT_LE
      SND_PCM_ACCESS_RW_NONINTERLEAVED channel-count-c
      samples-per-second-c default-alsa-enable-soft-resample latency-c))
  (define result SCM
    (sp-port-create sp-port-type-alsa input?
      samples-per-second-c channel-count-c 0 0 (convert-type alsa-port b0*) 0))
  (label exit local-memory-free
    (if alsa-port (snd-pcm-close alsa-port)) (status->scm-return result)))

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
  (define file int result SCM samples-per-second-file b32 channel-count-file b32)
  (define path-c char* (scm->locale-string path)) sp-status-init
  (local-memory-init 1) (local-memory-add path-c)
  (if (file-exists? path-c)
    (begin (set file (open path-c O_RDWR)) (sp-system-status-require-id file)
      (define encoding b32)
      (if
        (file-au-read-header file (address-of encoding)
          (address-of samples-per-second-file) (address-of channel-count-file))
        (status-set-id-goto sp-status-id-file-header))
      (if (not (= encoding 6)) (status-set-id-goto sp-status-id-file-encoding))
      (if
        (not
          (or (scm-is-undefined channel-count) (= channel-count-file (scm->uint32 channel-count))))
        (status-set-id-goto sp-status-id-file-incompatible))
      (if
        (not
          (or (scm-is-undefined samples-per-second)
            (= samples-per-second-file (scm->uint32 samples-per-second))))
        (status-set-id-goto sp-status-id-file-incompatible))
      (set result
        (sp-port-create sp-port-type-file input?
          samples-per-second-file channel-count-file #t (lseek file 0 SEEK-CUR) 0 file)))
    (begin (set file (open path-c (bit-or O_RDWR O_CREAT) 384)) (sp-system-status-require-id file)
      (set samples-per-second-file
        (if* (scm-is-undefined samples-per-second) default-samples-per-second
          (scm->uint32 samples-per-second)))
      (set channel-count-file
        (if* (scm-is-undefined samples-per-second) default-channel-count
          (scm->uint32 channel-count)))
      (if (< (file-au-write-header file 6 samples-per-second-file channel-count-file) 0)
        (status-set-id-goto sp-status-id-file-header))
      (set result
        (sp-port-create sp-port-type-file input?
          samples-per-second-file channel-count-file #t (lseek file 0 SEEK-CUR) 0 file))))
  (label exit local-memory-free (status->scm-return result)))

(define (scm-sp-io-file-open-input path) (SCM SCM)
  (return (sp-io-file-open path #t SCM-UNDEFINED SCM-UNDEFINED)))

(define (scm-sp-io-file-open-output path channel-count samples-per-second) (SCM SCM SCM SCM)
  (return (sp-io-file-open path #f channel-count samples-per-second)))

(define (scm-sp-io-alsa-write port sample-count channel-data) (SCM SCM SCM SCM)
  status-init (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-pointer-get port-data channel-count))
  (define sample-count-c b32 (scm->uint32 sample-count)) (local-memory-init 2)
  (sp-define-malloc channel-data-c sp-sample-t** (* channel-count (sizeof sp-sample-t*)))
  (local-memory-add channel-data-c) (define channel b32 0)
  (define a SCM)
  (scm-c-list-each channel-data a
    (compound-statement
      (set (deref channel-data-c channel) (convert-type (SCM-BYTEVECTOR-CONTENTS a) sp-sample-t*))
      (increment channel)))
  (define frames-written snd_pcm_sframes_t)
  (set frames-written
    (snd-pcm-writen (convert-type (struct-pointer-get port-data data) snd-pcm-t*)
      (convert-type channel-data-c b0**) (convert-type sample-count-c snd_pcm_uframes_t)))
  (if
    (and (< frames-written 0)
      (<
        (snd-pcm-recover (convert-type (struct-pointer-get port-data data) snd-pcm-t*)
          frames-written 0)
        0))
    (status-set-both-goto sp-status-group-alsa frames-written))
  (label exit local-memory-free (status->scm-return SCM-UNSPECIFIED)))

(define (scm-sp-io-alsa-read port sample-count) (SCM SCM SCM)
  status-init (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-pointer-get port-data channel-count))
  (define sample-count-c b64 (scm->uint32 sample-count)) (local-memory-init (+ 1 channel-count))
  (sp-define-malloc channel-data sp-sample-t** (* channel-count (sizeof sp-sample-t*)))
  (local-memory-add channel-data) (define channel b32 channel-count)
  (while channel (decrement channel)
    (sp-define-malloc data sp-sample-t* (* sample-count-c (sizeof sp-sample-t)))
    (local-memory-add data) (set (deref channel-data channel) data))
  (define frames-read snd_pcm_sframes_t)
  (set frames-read
    (snd-pcm-readn (convert-type (struct-pointer-get port-data data) snd-pcm-t*)
      (convert-type channel-data b0**) sample-count-c))
  (if
    (and (< frames-read 0)
      (<
        (snd-pcm-recover (convert-type (struct-pointer-get port-data data) snd-pcm-t*) frames-read
          0)
        0))
    (status-set-both-goto sp-status-group-alsa frames-read))
  (set channel channel-count) (define result SCM SCM-EOL)
  (while channel (decrement channel)
    (set result (scm-cons (scm-take-f32vector (deref channel-data channel) sample-count-c) result)))
  (label exit local-memory-free (status->scm-return result)))

(define (scm-sp-io-file-write port sample-count channel-data) (SCM SCM SCM SCM)
  status-init (define port-data port-data-t* (sp-port->port-data port))
  (if (struct-pointer-get port-data input?)
    (status-set-both-goto sp-status-group-sp sp-status-id-port-type))
  (define channel-count b32 (struct-pointer-get port-data channel-count))
  (if (not (= (scm->uint32 (scm-length channel-data)) channel-count))
    (status-set-both-goto sp-status-group-sp sp-status-id-file-channel-mismatch))
  (local-memory-init 2) (define sample-count-c b64 (scm->uint64 sample-count))
  (sp-define-malloc channel-data-c sp-sample-t** (* channel-count (sizeof sp-sample-t*)))
  (local-memory-add channel-data-c) (define channel b32 0)
  (define a SCM)
  (scm-c-list-each channel-data a
    (compound-statement
      (set (deref channel-data-c channel) (convert-type (SCM-BYTEVECTOR-CONTENTS a) sp-sample-t*))
      (increment channel)))
  (sp-define-malloc data-interleaved sp-sample-t*
    (* sample-count-c channel-count 4 (sizeof sp-sample-t)))
  (local-memory-add data-interleaved)
  (sp-interleave-and-reverse-endian channel-data-c data-interleaved channel-count sample-count-c)
  (sp-system-status-require!
    (write (struct-pointer-get port-data data-int) data-interleaved
      (* channel-count sample-count-c 4)))
  (label exit local-memory-free (status->scm-return SCM-UNSPECIFIED)))

(define (scm-sp-io-file-read port sample-count) (SCM SCM SCM)
  status-init (define port-data port-data-t* (sp-port->port-data port))
  (define channel-count b32 (struct-pointer-get port-data channel-count))
  (define sample-count-c b64 (scm->uint64 sample-count)) (local-memory-init (+ 2 channel-count))
  (sp-define-malloc data-interleaved sp-sample-t*
    (* sample-count-c channel-count 4 (sizeof sp-sample-t)))
  (local-memory-add data-interleaved)
  (sp-system-status-require!
    (read (struct-pointer-get port-data data-int) data-interleaved
      (* sample-count-c channel-count 4)))
  ; prepare deinterleaved memory
  (sp-define-malloc data-deinterleaved sp-sample-t** (* channel-count (sizeof sp-sample-t*)))
  (local-memory-add data-deinterleaved) (define channel b32 channel-count)
  (while channel (decrement channel)
    (sp-define-malloc data sp-sample-t* (* sample-count-c 4)) (local-memory-add data)
    (set (deref data-deinterleaved channel) data))
  ; deinterleave
  (sp-deinterleave-and-reverse-endian data-interleaved data-deinterleaved
    channel-count sample-count-c)
  ; create f32vectors
  (define result SCM SCM-EOL) (set channel channel-count)
  (while channel (decrement channel)
    (set result
      (scm-cons (scm-take-f32vector (deref data-deinterleaved channel) sample-count-c) result)))
  (label exit local-memory-free (status->scm-return result)))

(define (scm-sp-port-input? port) (SCM SCM)
  (return (scm-from-bool (struct-get (deref (sp-port->port-data port)) input?))))

(define (scm-sp-port-position port) (SCM SCM)
  "returns the current port position in number of octets"
  (return
    (if* (struct-get (deref (sp-port->port-data port)) position?)
      (scm-from-uint64 (* (struct-pointer-get (sp-port->port-data port) position) 0.25)) SCM-BOOL-F)))

(define (scm-sp-port-position? port) (SCM SCM)
  (return (scm-from-bool (struct-pointer-get (sp-port->port-data port) position))))

(define (scm-sp-port-channel-count port) (SCM SCM)
  (return (scm-from-uint32 (struct-pointer-get (sp-port->port-data port) channel-count))))

(define (scm-sp-port-samples-per-second port) (SCM SCM)
  (return (scm-from-uint32 (struct-pointer-get (sp-port->port-data port) samples-per-second))))

(define (scm-sp-port? port) (SCM SCM) (return (scm-from-bool (scm-c-sp-port? port))))

(define (scm-sp-port-type port) (SCM SCM)
  (pre-let (type (struct-pointer-get (sp-port->port-data port) type))
    (return
      (cond* ((= type sp-port-type-alsa) scm-sp-port-type-alsa)
        ((= type sp-port-type-file) scm-sp-port-type-file) (else SCM-BOOL-F)))))

(define (scm-sp-io-file-set-position port sample-position) (SCM SCM SCM)
  status-init (define port-data port-data-t* (sp-port->port-data port))
  (define position-c b64-s (* (scm->int64 sample-position) 4))
  (pre-let
    (file (struct-pointer-get port-data data-int) position-offset
      (struct-pointer-get port-data position-offset))
    (if (>= position-c 0)
      (sp-system-status-require! (lseek file (+ position-offset position-c) SEEK_SET))
      (begin (define end-position off-t (lseek file 0 SEEK_END))
        (sp-system-status-require-id end-position) (set position-c (+ end-position position-c))
        (if (>= position-c position-offset)
          (sp-system-status-require! (lseek file position-c SEEK_SET))
          (status-set-both-goto sp-status-group-sp sp-status-id-port-position)))))
  (set (struct-pointer-get port-data position) position-c)
  (label exit (status->scm-return SCM-BOOL-T)))
