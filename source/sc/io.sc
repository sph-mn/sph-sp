(pre-define sp-port-type-alsa 0 sp-port-type-file 1 sp-port-bit-input 1 sp-port-bit-position 2)

(define (file-au-write-header file encoding sample-rate channel-count) (b8-s int b32 b32 b32)
  "-> boolean
  returns 1 if the header was successfully written, 0 otherwise.
  assumes file is positioned at offset 0"
  (define header[7] b32)
  (set (deref header 0) (__builtin-bswap32 779316836)
    (deref header 1) (__builtin-bswap32 28)
    (deref header 2) (__builtin-bswap32 4294967295)
    (deref header 3) (__builtin-bswap32 encoding)
    (deref header 4) (__builtin-bswap32 sample-rate)
    (deref header 5) (__builtin-bswap32 channel-count) (deref header 6) 0)
  (define status ssize-t (write file header 28)) (return (= status 28)))

(define (file-au-read-header file encoding sample-rate channel-count) (b8-s int b32* b32* b32*)
  "-> boolean
  when successful, the reader is positioned at the beginning of the sample data"
  (define status ssize-t header[6] b32) (set status (read file header 24))
  (if (not (and (= status 24) (= (deref header) (__builtin-bswap32 779316836)))) (return #f))
  (if (< (lseek file (__builtin-bswap32 (deref header 1)) SEEK_SET) 0) (return #f))
  (set (deref encoding) (__builtin-bswap32 (deref header 3))
    (deref sample-rate) (__builtin-bswap32 (deref header 4))
    (deref channel-count) (__builtin-bswap32 (deref header 5)))
  (return #t))

(pre-define (optional-sample-rate a)
  (if* (= SCM-UNDEFINED a) sp-default-sample-rate (scm->uint32 a)))

(pre-define (optional-channel-count a)
  (if* (scm-is-undefined a) sp-default-channel-count (scm->uint32 a)))

(pre-define (sp-alsa-status-require! expression) (status-set-id expression)
  (if status-failure? (status-set-group-goto sp-status-group-alsa)))

(pre-define (define-sp-interleave name type body)
  "a: deinterleaved
   b: interleaved"
  (define (name a b a-size channel-count) (b0 type** type* size-t b32)
    (define b-size size-t (* a-size channel-count)) (define channel b32)
    (while a-size (decrement a-size)
      (set channel channel-count) (while channel (decrement channel) (decrement b-size) body))))

(define-sp-interleave sp-interleave-and-reverse-endian sp-sample-t
  (compound-statement
    (set (deref b b-size) (sample-reverse-endian (deref (deref a channel) a-size)))))

(define-sp-interleave sp-deinterleave-and-reverse-endian sp-sample-t
  (compound-statement
    (set (deref (deref a channel) a-size) (sample-reverse-endian (deref b b-size)))))

(define-type port-data-t
  ; type: any of the sp-port-type-* values
  ; position?: true if the port supports random access
  ; position-offset: header length
  (struct (sample-rate b32) (channel-count b32)
    (flags b8) (type b8) (closed? b8) (position b64) (position-offset b16) (data b0*) (data-int int)))

(define sp-port-scm-type scm-t-bits scm-sp-port-type-alsa SCM scm-sp-port-type-file SCM)
(pre-define (scm-c-sp-port? a) (SCM-SMOB-PREDICATE sp-port-scm-type a))
(pre-define (scm->port-data a) (convert-type (SCM-SMOB-DATA a) port-data-t*))

(pre-define (sp-port-type->name a) "integer -> string"
  (cond* ((= sp-port-type-file a) "file") ((= sp-port-type-alsa a) "alsa") (else "unknown")))

(define (sp-port-print a output-port print-state) (int SCM SCM scm-print-state*)
  (define port-data port-data-t* (scm->port-data a))
  (define result char* (malloc (+ 70 10 7 10 10 2 2))) (if (not result) (return 0))
  (sprintf result "#<sp-port %lx type:%s sample-rate:%d channel-count:%d closed?:%s input?:%s>"
    (convert-type a b0*) (sp-port-type->name (struct-pointer-get port-data type))
    (struct-pointer-get port-data sample-rate) (struct-pointer-get port-data channel-count)
    (if* (struct-pointer-get port-data closed?) "#t" "#f")
    (if* (bit-and sp-port-bit-input (struct-pointer-get port-data flags)) "#t" "#f"))
  ; scm-take- eventually frees
  (scm-display (scm-take-locale-string result) output-port) (return 0))

(define (sp-port-create type flags sample-rate channel-count position-offset data data-int)
  (SCM b8 b8 b32 b32 b16 b0* int)
  "integer integer integer integer integer pointer integer -> sp-port
   flags is a combination of sp-port-bits.
  memory is allocated with scm-gc-malloc"
  (define port-data port-data-t* (scm-gc-malloc (sizeof port-data-t) "sp-port-data"))
  (struct-pointer-set port-data channel-count
    channel-count sample-rate
    sample-rate type
    type flags flags data data data-int data-int position 0 position-offset position-offset)
  (return (scm-new-smob sp-port-scm-type (convert-type port-data scm-t-bits))))

(pre-define sp-port-scm-type-init
  ; initialise the sp port type
  (set sp-port-scm-type (scm-make-smob-type "sp-port" 0))
  (scm-set-smob-print sp-port-scm-type sp-port-print))

(define (sp-io-alsa-open input? scm-device-name scm-channel-count scm-sample-rate scm-latency)
  (SCM b8 SCM SCM SCM SCM) status-init
  (define alsa-port snd-pcm-t* 0) (local-memory-init 1)
  (define device-name char*)
  (if (scm-is-undefined scm-device-name) (set device-name "default")
    (begin (set device-name (scm->locale-string scm-device-name)) (local-memory-add device-name)))
  (sp-alsa-status-require!
    (snd-pcm-open (address-of alsa-port) device-name
      (if* input? SND_PCM_STREAM_CAPTURE SND_PCM_STREAM_PLAYBACK) 0))
  (define latency b32
    (if* (scm-is-undefined scm-latency) sp-default-alsa-latency (scm->uint32 scm-latency)))
  (define channel-count b32 (optional-channel-count scm-channel-count))
  (define sample-rate b32 (optional-sample-rate scm-sample-rate))
  (sp-alsa-status-require!
    (snd-pcm-set-params alsa-port SND_PCM_FORMAT_FLOAT_LE
      SND_PCM_ACCESS_RW_NONINTERLEAVED channel-count
      sample-rate sp-default-alsa-enable-soft-resample latency))
  (define result SCM
    (sp-port-create sp-port-type-alsa (if* input? sp-port-bit-input 0)
      sample-rate channel-count 0 (convert-type alsa-port b0*) 0))
  (label exit (if (and status-failure? alsa-port) (snd-pcm-close alsa-port))
    local-memory-free (status->scm-return result)))

(define (sp-io-file-open path input? channel-count sample-rate) (SCM SCM b8 SCM SCM)
  (define file int result SCM sample-rate-file b32 channel-count-file b32)
  (define path-c char* (scm->locale-string path)) sp-status-init
  (local-memory-init 1) (local-memory-add path-c)
  (define flags b8
    (if* input? (bit-or sp-port-bit-input sp-port-bit-position) sp-port-bit-position))
  (if (file-exists? path-c)
    (begin (set file (open path-c O_RDWR)) (sp-system-status-require-id file)
      (define encoding b32)
      (if
        (not
          (file-au-read-header file (address-of encoding)
            (address-of sample-rate-file) (address-of channel-count-file)))
        (status-set-id-goto sp-status-id-file-header))
      (if (not (= encoding 6)) (status-set-id-goto sp-status-id-file-encoding))
      (if
        (not
          (or (scm-is-undefined channel-count) (= channel-count-file (scm->uint32 channel-count))))
        (status-set-id-goto sp-status-id-file-incompatible))
      (if (not (or (scm-is-undefined sample-rate) (= sample-rate-file (scm->uint32 sample-rate))))
        (status-set-id-goto sp-status-id-file-incompatible))
      (define offset off-t (lseek file 0 SEEK-CUR)) (sp-system-status-require-id offset)
      (set result
        (sp-port-create sp-port-type-file flags sample-rate-file channel-count-file offset 0 file)))
    (begin (set file (open path-c (bit-or O_RDWR O_CREAT) 384)) (sp-system-status-require-id file)
      (set sample-rate-file (optional-sample-rate sample-rate))
      (set channel-count-file (optional-channel-count channel-count))
      (if (not (file-au-write-header file 6 sample-rate-file channel-count-file))
        (status-set-id-goto sp-status-id-file-header))
      (define offset off-t (lseek file 0 SEEK-CUR)) (sp-system-status-require-id offset)
      (set result
        (sp-port-create sp-port-type-file flags sample-rate-file channel-count-file offset 0 file))))
  (label exit local-memory-free (status->scm-return result)))

(define (scm-sp-port-close a) (SCM SCM)
  status-init (define port-data port-data-t* (scm->port-data a))
  (if (struct-pointer-get port-data closed?) (goto exit))
  (case = (struct-pointer-get port-data type)
    (sp-port-type-alsa
      (sp-alsa-status-require!
        (snd-pcm-close (convert-type (struct-pointer-get port-data data) snd-pcm-t*))))
    (sp-port-type-file (sp-system-status-require! (close (struct-pointer-get port-data data-int)))))
  (struct-pointer-set port-data closed? #t) (label exit (status->scm-return SCM-BOOL-T)))

(define (scm-sp-port-position port) (SCM SCM)
  "returns the current port position in number of octets"
  (return (scm-from-uint64 (struct-pointer-get (scm->port-data port) position))))

(define (scm-sp-port-position? port) (SCM SCM)
  (return
    (scm-from-bool (bit-and sp-port-bit-position (struct-pointer-get (scm->port-data port) flags)))))

(define (scm-sp-port-input? port) (SCM SCM)
  (return
    (scm-from-bool (bit-and sp-port-bit-input (struct-pointer-get (scm->port-data port) flags)))))

(define (scm-sp-port-channel-count port) (SCM SCM)
  (return (scm-from-uint32 (struct-pointer-get (scm->port-data port) channel-count))))

(define (scm-sp-port-sample-rate port) (SCM SCM)
  (return (scm-from-uint32 (struct-pointer-get (scm->port-data port) sample-rate))))

(define (scm-sp-port? port) (SCM SCM) (return (scm-from-bool (scm-c-sp-port? port))))

(define (scm-sp-port-type port) (SCM SCM)
  (return
    (case* = (struct-pointer-get (scm->port-data port) type)
      (sp-port-type-alsa scm-sp-port-type-alsa) (sp-port-type-file scm-sp-port-type-file)
      (else SCM-BOOL-F))))

(define (scm-sp-io-alsa-write port scm-sample-count scm-channel-data) (SCM SCM SCM SCM)
  status-init (define port-data port-data-t* (scm->port-data port))
  (define channel-count b32 (struct-pointer-get port-data channel-count))
  (define deinterleaved-count size-t (scm->size-t scm-sample-count)) (local-memory-init 1)
  (sp-define-malloc deinterleaved sp-sample-t** (* channel-count (sizeof sp-sample-t*)))
  (local-memory-add deinterleaved) (define channel b32 0)
  (define a SCM)
  (scm-c-list-each scm-channel-data a
    (compound-statement
      (set (deref deinterleaved channel) (convert-type (SCM-BYTEVECTOR-CONTENTS a) sp-sample-t*))
      (increment channel)))
  (define frames-written snd_pcm_sframes_t
    (snd-pcm-writen (convert-type (struct-pointer-get port-data data) snd-pcm-t*)
      (convert-type deinterleaved b0**) (convert-type deinterleaved-count snd_pcm_uframes_t)))
  (if
    (and (< frames-written 0)
      (<
        (snd-pcm-recover (convert-type (struct-pointer-get port-data data) snd-pcm-t*)
          frames-written 0)
        0))
    (status-set-both-goto sp-status-group-alsa frames-written))
  (label exit local-memory-free (status->scm-return SCM-UNSPECIFIED)))

(define (scm-sp-io-alsa-read port scm-sample-count) (SCM SCM SCM)
  status-init (define port-data port-data-t* (scm->port-data port))
  (define channel-count b32 (struct-pointer-get port-data channel-count))
  (define deinterleaved-count size-t (scm->size-t scm-sample-count)) (local-memory-init 1)
  (sp-define-malloc deinterleaved sp-sample-t** (* channel-count (sizeof sp-sample-t*)))
  (local-memory-add deinterleaved) (define channel b32 channel-count)
  (define deinterleaved-size size-t (* deinterleaved-count (sizeof sp-sample-t)))
  (while channel (decrement channel)
    (sp-define-malloc data sp-sample-t* deinterleaved-size) (set (deref deinterleaved channel) data))
  (define frames-read snd_pcm_sframes_t
    (snd-pcm-readn (convert-type (struct-pointer-get port-data data) snd-pcm-t*)
      (convert-type deinterleaved b0**) deinterleaved-count))
  (if
    (and (< frames-read 0)
      (<
        (snd-pcm-recover (convert-type (struct-pointer-get port-data data) snd-pcm-t*) frames-read
          0)
        0))
    (status-set-both-goto sp-status-group-alsa frames-read))
  (define result SCM SCM-EOL) (set channel channel-count)
  (while channel (decrement channel)
    (set result
      (scm-cons (scm-take-f32vector (deref deinterleaved channel) deinterleaved-count) result)))
  (label exit local-memory-free (status->scm-return result)))

(define (scm-sp-io-file-write port scm-sample-count scm-channel-data) (SCM SCM SCM SCM)
  status-init (define port-data port-data-t* (scm->port-data port))
  (if (bit-and sp-port-bit-input (struct-pointer-get port-data flags))
    (status-set-both-goto sp-status-group-sp sp-status-id-port-type))
  (define channel-count b32 (struct-pointer-get port-data channel-count))
  (if (not (= (scm->uint32 (scm-length scm-channel-data)) channel-count))
    (status-set-both-goto sp-status-group-sp sp-status-id-file-channel-mismatch))
  (local-memory-init 2)
  (sp-define-malloc deinterleaved sp-sample-t** (* channel-count (sizeof sp-sample-t*)))
  (local-memory-add deinterleaved) (define channel b32 0)
  (define a SCM)
  (scm-c-list-each scm-channel-data a
    (compound-statement
      (set (deref deinterleaved channel) (convert-type (SCM-BYTEVECTOR-CONTENTS a) sp-sample-t*))
      (increment channel)))
  (define deinterleaved-count size-t (scm->size-t scm-sample-count))
  (define interleaved-size size-t (* channel-count deinterleaved-count (sizeof sp-sample-t*)))
  (sp-define-malloc interleaved sp-sample-t* interleaved-size) (local-memory-add interleaved)
  (sp-interleave-and-reverse-endian deinterleaved interleaved deinterleaved-count channel-count)
  (define count int (write (struct-pointer-get port-data data-int) interleaved interleaved-size))
  (if (not (= interleaved-size count))
    (if (< count 0) (status-set-both-goto sp-status-group-libc count)
      (status-set-both-goto sp-status-group-sp sp-status-id-file-incomplete)))
  (label exit local-memory-free (status->scm-return SCM-UNSPECIFIED)))

(define (scm-sp-io-file-read port scm-sample-count) (SCM SCM SCM)
  status-init (define result SCM)
  (define port-data port-data-t* (scm->port-data port))
  (define channel-count b32 (struct-pointer-get port-data channel-count)) (local-memory-init 2)
  (define deinterleaved-count size-t (scm->size-t scm-sample-count))
  (define interleaved-size size-t (* channel-count deinterleaved-count (sizeof sp-sample-t)))
  (sp-define-malloc interleaved sp-sample-t* interleaved-size) (local-memory-add interleaved)
  (define count int (read (struct-pointer-get port-data data-int) interleaved interleaved-size))
  (cond ((not count) (set result SCM_EOF_VAL) (goto exit))
    ((< count 0) (status-set-both-goto sp-status-group-libc count))
    ( (not (= interleaved-size count))
      (set interleaved-size count
        deinterleaved-count (/ interleaved-size channel-count (sizeof sp-sample-t)))))
  ; deinterleave
  (sp-define-malloc deinterleaved sp-sample-t** (* channel-count (sizeof sp-sample-t*)))
  (local-memory-add deinterleaved)
  (define deinterleaved-size size-t (/ interleaved-size channel-count))
  (define channel b32 channel-count)
  (while channel (decrement channel)
    ; deallocation by guile after scm-take-f32vector
    (sp-define-malloc data sp-sample-t* deinterleaved-size) (set (deref deinterleaved channel) data))
  (sp-deinterleave-and-reverse-endian deinterleaved interleaved deinterleaved-count channel-count)
  ; result vectors
  (set result SCM_EOL) (set channel channel-count)
  (while channel (decrement channel)
    (set result
      (scm-cons (scm-take-f32vector (deref deinterleaved channel) deinterleaved-count) result)))
  (label exit local-memory-free (status->scm-return result)))

(define (scm-sp-io-file-set-position port scm-sample-index) (SCM SCM SCM)
  "sp-port integer -> unspecified
   set port to offset in sample data"
  status-init (define port-data port-data-t* (scm->port-data port))
  (define sample-index b32 (scm->int64 scm-sample-index))
  (define index b64-s (* (sizeof sp-sample-t) sample-index))
  (pre-let
    (file (struct-pointer-get port-data data-int) header-size
      (struct-pointer-get port-data position-offset))
    (if (>= index 0) (sp-system-status-require! (lseek file (+ header-size index) SEEK_SET))
      (begin (define end-position off-t (lseek file 0 SEEK_END))
        (sp-system-status-require-id end-position) (set index (+ end-position index))
        (if (>= index header-size) (sp-system-status-require! (lseek file index SEEK_SET))
          (status-set-both-goto sp-status-group-sp sp-status-id-port-position)))))
  (set (struct-pointer-get port-data position) sample-index)
  (label exit (status->scm-return SCM-BOOL-T)))

(define (scm-sp-io-alsa-open-input device-name channel-count sample-rate latency)
  (SCM SCM SCM SCM SCM) (return (sp-io-alsa-open #t device-name channel-count sample-rate latency)))

(define (scm-sp-io-alsa-open-output device-name channel-count sample-rate latency)
  (SCM SCM SCM SCM SCM) (return (sp-io-alsa-open #f device-name channel-count sample-rate latency)))

(define (scm-sp-io-file-open-input path) (SCM SCM)
  (return (sp-io-file-open path #t SCM-UNDEFINED SCM-UNDEFINED)))

(define (scm-sp-io-file-open-output path channel-count sample-rate) (SCM SCM SCM SCM)
  (return (sp-io-file-open path #f channel-count sample-rate)))
