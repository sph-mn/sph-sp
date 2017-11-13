(define (file-au-write-header file encoding sample-rate channel-count) (b8-s int b32 b32 b32)
  "-> boolean
  returns 1 if the header was successfully written, 0 otherwise.
  assumes file is positioned at offset 0"
  (define header[7] b32)
  (set
    (deref header 0) (__builtin-bswap32 779316836)
    (deref header 1) (__builtin-bswap32 28)
    (deref header 2) (__builtin-bswap32 4294967295)
    (deref header 3) (__builtin-bswap32 encoding)
    (deref header 4) (__builtin-bswap32 sample-rate)
    (deref header 5) (__builtin-bswap32 channel-count)
    (deref header 6) 0)
  (define status ssize-t (write file header 28))
  (return (= status 28)))

(define (file-au-read-header file encoding sample-rate channel-count) (b8-s int b32* b32* b32*)
  "-> boolean
  when successful, the reader is positioned at the beginning of the sample data"
  (define
    status ssize-t
    header[6] b32)
  (set status (read file header 24))
  (if (not (and (= status 24) (= (deref header) (__builtin-bswap32 779316836)))) (return #f))
  (if (< (lseek file (__builtin-bswap32 (deref header 1)) SEEK_SET) 0) (return #f))
  (set
    (deref encoding) (__builtin-bswap32 (deref header 3))
    (deref sample-rate) (__builtin-bswap32 (deref header 4))
    (deref channel-count) (__builtin-bswap32 (deref header 5)))
  (return #t))

(pre-define (define-sp-interleave name type body)
  "a: deinterleaved
   b: interleaved"
  (define (name a b a-size channel-count) (b0 type** type* size-t b32)
    (define b-size size-t (* a-size channel-count))
    (define channel b32)
    (while a-size
      (dec a-size)
      (set channel channel-count)
      (while channel
        (dec channel)
        (dec b-size)
        body))))

(define-sp-interleave
  sp-interleave-and-reverse-endian
  sp-sample-t
  (compound-statement
    (set (deref b b-size) (sample-reverse-endian (deref (deref a channel) a-size)))))

(define-sp-interleave
  sp-deinterleave-and-reverse-endian
  sp-sample-t
  (compound-statement
    (set (deref (deref a channel) a-size) (sample-reverse-endian (deref b b-size)))))

(pre-define (optional-number a default)
  "choose a default when number is negative"
  (if* (< a 0) default a))

(pre-define (set-optional-number a default) (if (< a 0) (set a default)))
(sc-comment "sp-port abstracts different output targets and formats")

(define (sp-port-init result type flags sample-rate channel-count position-offset data data-int)
  (status-i-t sp-port-t* b8 b8 b32 b32 b16 b0* int)
  "integer integer integer integer integer pointer integer -> sp-port
   flags is a combination of sp-port-bits"
  (struct-pointer-set result
    channel-count channel-count
    sample-rate sample-rate
    type type flags flags data data data-int data-int position 0 position-offset position-offset)
  (return status-id-success))

(define (sp-port-position? a) (boolean sp-port-t*)
  (return (bit-and sp-port-bit-position (struct-pointer-get a flags))))

(define (sp-port-input? a) (boolean sp-port-t*)
  (return (bit-and sp-port-bit-input (struct-pointer-get a flags))))

(define (sp-port-output? a) (boolean sp-port-t*)
  (return (bit-and sp-port-bit-output (struct-pointer-get a flags))))

(sc-comment "-- au file")

(define (sp-file-open result path channel-count sample-rate) (status-t sp-port-t* b8* b32 b32)
  sp-status-init
  (define
    file int
    channel-count-file b32
    sample-rate-file b32)
  ; todo: current limitation: can only read files with the same sample type as sp-sample-t
  (define sp-port-flags b8 (bit-or sp-port-bit-input sp-port-bit-output sp-port-bit-position))
  (define encoding b8
    (case* = (sizeof sp-sample-t)
      (8 7)
      (4 6)))
  (if (file-exists? path)
    (begin
      (set file (open path O_RDWR))
      (sp-system-status-require-id file)
      (define encoding-file b32)
      (if
        (not
          (file-au-read-header
            file
            (address-of encoding-file) (address-of sample-rate-file) (address-of channel-count-file)))
        (status-set-id-goto sp-status-id-file-header))
      (if (not (= encoding-file encoding)) (status-set-id-goto sp-status-id-file-encoding))
      (if (not (= channel-count-file channel-count))
        (status-set-id-goto sp-status-id-file-incompatible))
      (if (not (= sample-rate-file sample-rate))
        (status-set-id-goto sp-status-id-file-incompatible))
      (define offset off-t (lseek file 0 SEEK-CUR))
      (sp-system-status-require-id offset)
      (status-i-require!
        (sp-port-init
          result sp-port-type-file sp-port-flags sample-rate-file channel-count-file offset 0 file)))
    (begin
      (set file (open path (bit-or O_RDWR O_CREAT) 384))
      (sp-system-status-require-id file)
      (set sample-rate-file sample-rate)
      (set channel-count-file channel-count)
      (if (not (file-au-write-header file encoding sample-rate-file channel-count-file))
        (status-set-id-goto sp-status-id-file-header))
      (define offset off-t (lseek file 0 SEEK-CUR))
      (sp-system-status-require-id offset)
      (status-i-require!
        (sp-port-init
          result sp-port-type-file sp-port-flags sample-rate-file channel-count-file offset 0 file))))
  (label exit
    (if (and status-failure? file) (close file))
    (return status)))

(define (sp-file-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**)
  status-init
  (local-memory-init 1)
  (if (not (bit-and sp-port-bit-input (struct-pointer-get port flags)))
    (status-set-both-goto sp-status-group-sp sp-status-id-port-type))
  (define channel-count b32 (struct-pointer-get port channel-count))
  (define interleaved-size size-t (* channel-count sample-count (sizeof sp-sample-t*)))
  (sp-alloc-define interleaved sp-sample-t* interleaved-size)
  (local-memory-add interleaved)
  (sp-interleave-and-reverse-endian channel-data interleaved sample-count channel-count)
  (define count int (write (struct-pointer-get port data-int) interleaved interleaved-size))
  (if (not (= interleaved-size count))
    (if (< count 0)
      (status-set-both-goto sp-status-group-libc count)
      (status-set-both-goto sp-status-group-sp sp-status-id-file-incomplete)))
  (label exit
    local-memory-free
    (return status)))

(define (sp-file-read result port sample-count) (status-t sp-sample-t** sp-port-t* size-t)
  status-init
  (local-memory-init 1)
  (define channel-count b32 (struct-pointer-get port channel-count))
  (define interleaved-size size-t (* channel-count sample-count (sizeof sp-sample-t)))
  (sp-alloc-define interleaved sp-sample-t* interleaved-size)
  (local-memory-add interleaved)
  (define count int (read (struct-pointer-get port data-int) interleaved interleaved-size))
  (cond
    ((not count) (set result 0) (goto exit))
    ((< count 0) (status-set-both-goto sp-status-group-libc count))
    ( (not (= interleaved-size count))
      (set
        interleaved-size count
        sample-count (/ interleaved-size channel-count (sizeof sp-sample-t)))))
  ; deinterleave
  (sp-deinterleave-and-reverse-endian result interleaved sample-count channel-count)
  (label exit
    local-memory-free
    (return status)))

(pre-define (sp-file-index->position index position-offset channel-count)
  (/ (- index position-offset) channel-count (sizeof sp-sample-t)))

(define (sp-file-set-position port sample-index) (status-t sp-port-t* size-t)
  "set port to offset in sample data"
  status-init
  (define index size-t
    (* sample-index (struct-pointer-get port channel-count) (sizeof sp-sample-t)))
  (define header-size b16 (struct-pointer-get port position-offset))
  (define file int (struct-pointer-get port data-int))
  ; calculate positive index from negative index
  (if (> 0 index)
    (begin
      (define end-position off-t (lseek file 0 SEEK_END))
      (sp-system-status-require-id end-position)
      (set index (+ end-position index)))
    (set index (+ header-size index)))
  ; index must be after header-size
  (if (> header-size index) (status-set-both-goto sp-status-group-sp sp-status-id-port-position))
  (sp-system-status-require! (lseek file index SEEK_SET))
  (struct-pointer-set port
    position (sp-file-index->position index header-size (struct-pointer-get port channel-count)))
  (label exit
    (return status)))

(define (sp-file-position result port) (status-t size-t* sp-port-t*)
  status-init
  (define file int (struct-pointer-get port data-int))
  (define offset off-t (lseek file 0 SEEK_END))
  (sp-system-status-require-id offset)
  (define index size-t (lseek file 0 SEEK_CUR))
  (define header-size b16 (struct-pointer-get port position-offset))
  (if (> header-size index) (status-set-both-goto sp-status-group-sp sp-status-id-port-position))
  (set (deref result)
    (sp-file-index->position index header-size (struct-pointer-get port channel-count)))
  (label exit
    (return status)))

(sc-comment "-- alsa")

(define (sp-alsa-open result device-name input? channel-count sample-rate latency)
  (status-t sp-port-t* b8* boolean b32 b32 b32-s)
  "open alsa sound output for capture or playback"
  status-init
  ; defaults
  (if (not device-name) (set device-name "default"))
  (set-optional-number latency sp-default-alsa-latency)
  (define alsa-port snd-pcm-t* 0)
  ; open
  (sp-alsa-status-require!
    (snd-pcm-open
      (address-of alsa-port)
      device-name (if* input? SND_PCM_STREAM_CAPTURE SND_PCM_STREAM_PLAYBACK) 0))
  ; settings
  (sp-alsa-status-require!
    (snd-pcm-set-params
      alsa-port
      sp-alsa-snd-pcm-format
      SND_PCM_ACCESS_RW_NONINTERLEAVED
      channel-count sample-rate sp-default-alsa-enable-soft-resample latency))
  (define sp-port-flags b8 (if* input? sp-port-bit-input sp-port-bit-output))
  (status-i-require!
    (sp-port-init
      result
      sp-port-type-alsa sp-port-flags sample-rate channel-count 0 (convert-type alsa-port b0*) 0))
  (label exit
    (if (and status-failure? alsa-port) (snd-pcm-close alsa-port))
    (return status)))

(define (sp-alsa-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**)
  status-init
  (define alsa-port snd-pcm-t* (struct-pointer-get port data))
  (define frames-written snd_pcm_sframes_t
    (snd-pcm-writen
      alsa-port (convert-type channel-data b0**) (convert-type sample-count snd_pcm_uframes_t)))
  (if (and (< frames-written 0) (< (snd-pcm-recover alsa-port frames-written 0) 0))
    (status-set-both-goto sp-status-group-alsa frames-written))
  (label exit
    (return status)))

(define (sp-alsa-read result port sample-count) (status-t sp-sample-t** sp-port-t* b32)
  status-init
  (define alsa-port snd-pcm-t* (struct-pointer-get port data))
  (define frames-read snd_pcm_sframes_t
    (snd-pcm-readn alsa-port (convert-type result b0**) sample-count))
  (if (and (< frames-read 0) (< (snd-pcm-recover alsa-port frames-read 0) 0))
    (status-set-both-goto sp-status-group-alsa frames-read))
  (label exit
    (return status)))

(define (sp-port-read result port sample-count) (status-t sp-sample-t** sp-port-t* b32)
  (case = (struct-pointer-get port type)
    (sp-port-type-file (return (sp-file-read result port sample-count)))
    (sp-port-type-alsa (return (sp-alsa-read result port sample-count)))))

(define (sp-port-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**)
  (case = (struct-pointer-get port type)
    (sp-port-type-file (return (sp-file-write port sample-count channel-data)))
    (sp-port-type-alsa (return (sp-alsa-write port sample-count channel-data)))))

(define (sp-port-close a) (status-t sp-port-t*)
  status-init
  (if (struct-pointer-get a closed?) (goto exit))
  (case = (struct-pointer-get a type)
    (sp-port-type-alsa
      (sp-alsa-status-require!
        (snd-pcm-close (convert-type (struct-pointer-get a data) snd-pcm-t*))))
    (sp-port-type-file (sp-system-status-require! (close (struct-pointer-get a data-int)))))
  (struct-pointer-set a closed? #t)
  (label exit
    (return status)))

(pre-define sp-port-not-implemented
  (begin
    status-init
    (status-set-both sp-status-group-sp sp-status-id-not-implemented)
    (return status)))

(define (sp-port-position result port) (status-t size-t* sp-port-t*)
  (case = (struct-pointer-get port type)
    (sp-port-type-file (return (sp-file-position result port)))
    (sp-port-type-alsa sp-port-not-implemented)))

(define (sp-port-set-position port sample-index) (status-t sp-port-t* size-t)
  (case = (struct-pointer-get port type)
    (sp-port-type-file (return (sp-file-set-position port sample-index)))
    (sp-port-type-alsa sp-port-not-implemented)))
