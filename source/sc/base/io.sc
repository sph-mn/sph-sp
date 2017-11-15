(pre-define (set-optional-number a default) (if (< a 0) (set a default)))

(pre-define (optional-number a default)
  "choose a default when number is negative"
  (if* (< a 0) default a))

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
  sp-interleave
  sp-sample-t (compound-statement (set (deref b b-size) (deref (deref a channel) a-size))))

(define-sp-interleave
  sp-deinterleave
  sp-sample-t (compound-statement (set (deref (deref a channel) a-size) (deref b b-size))))

(sc-comment "-- file")
(define sp-file-sf-format b32 (bit-or SF-FORMAT-AU SF-FORMAT-FLOAT))
;(define sp-file-sf-format b32 (bit-or SF-FORMAT-WAV SF-FORMAT-DOUBLE))

(define (sp-file-close port) (status-t sp-port-t*)
  status-init
  (struct-set status id (sf-close (convert-type (struct-pointer-get port data) SNDFILE*)))
  (if (not status.id)
    (struct-pointer-set port flags (bit-or sp-port-bit-closed (struct-pointer-get port flags))))
  (return status))

(define (sp-file-open result path channel-count sample-rate) (status-t sp-port-t* b8* b32 b32)
  status-init
  (define
    info SF_INFO
    file SNDFILE*
    mode int)
  (struct-set info format sp-file-sf-format channels channel-count samplerate sample-rate)
  (set mode SFM-RDWR)
  (set file (sf-open path mode (address-of info)))
  (if (not file) (status-set-both-goto sp-status-group-sndfile (sf-error file)))
  (define bit-position b8 (if* (struct-get info seekable) sp-port-bit-position 0))
  (struct-pointer-set result
    channel-count channel-count
    sample-rate sample-rate
    type sp-port-type-file flags (bit-or sp-port-bit-input sp-port-bit-output bit-position) data file)
  (label exit
    (return status)))

(define (sp-file-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**)
  status-init
  (local-memory-init 1)
  (define channel-count b32 (struct-pointer-get port channel-count))
  (define file SNDFILE* (struct-pointer-get port data))
  (define interleaved-size size-t (* channel-count sample-count (sizeof sp-sample-t*)))
  (sp-alloc-define interleaved sp-sample-t* interleaved-size)
  (local-memory-add interleaved)
  (sp-interleave channel-data interleaved sample-count channel-count)
  (define result-count sf-count-t (sf-writef-double file interleaved sample-count))
  (if (not (= sample-count result-count))
    (status-set-both-goto sp-status-group-sp sp-status-id-file-incomplete))
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
  (define result-count sf-count-t
    (sf-readf-double
      (convert-type (struct-pointer-get port data) SNDFILE*) interleaved sample-count))
  (if (not (= interleaved-size result-count)) (status-set-both sp-status-group-sp sp-status-id-eof))
  (sp-deinterleave result interleaved result-count channel-count)
  (label exit
    local-memory-free
    (return status)))

(define (sp-file-set-position port a) (status-t sp-port-t* size-t)
  status-init
  (define file SNDFILE* (struct-pointer-get port data))
  (define count sf-count-t (sf-seek file a SEEK-SET))
  (if (= count -1) (status-set-both-goto sp-status-group-sndfile (sf-error file)))
  (label exit
    (return status)))

(define (sp-file-position result port) (status-t size-t* sp-port-t*)
  status-init
  (define file SNDFILE* (struct-pointer-get port data))
  (define count sf-count-t (sf-seek file 0 SEEK-CUR))
  (if (= count -1) (status-set-both-goto sp-status-group-sndfile (sf-error file)))
  (set (deref result) count)
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
  (struct-pointer-set result
    type sp-port-type-alsa
    flags (if* input? sp-port-bit-input sp-port-bit-output) data (convert-type alsa-port b0*))
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
    (status-set-both sp-status-group-alsa frames-written))
  (return status))

(define (sp-alsa-read result port sample-count) (status-t sp-sample-t** sp-port-t* b32)
  status-init
  (define alsa-port snd-pcm-t* (struct-pointer-get port data))
  (define frames-read snd_pcm_sframes_t
    (snd-pcm-readn alsa-port (convert-type result b0**) sample-count))
  (if (and (< frames-read 0) (< (snd-pcm-recover alsa-port frames-read 0) 0))
    (status-set-both sp-status-group-alsa frames-read))
  (return status))

(sc-comment "-- sc-port")

(define (sp-port-position? a) (boolean sp-port-t*)
  (return (bit-and sp-port-bit-position (struct-pointer-get a flags))))

(define (sp-port-input? a) (boolean sp-port-t*)
  (return (bit-and sp-port-bit-input (struct-pointer-get a flags))))

(define (sp-port-output? a) (boolean sp-port-t*)
  (return (bit-and sp-port-bit-output (struct-pointer-get a flags))))

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
  (case = (struct-pointer-get a type)
    (sp-port-type-alsa
      (sp-alsa-status-require!
        (snd-pcm-close (convert-type (struct-pointer-get a data) snd-pcm-t*))))
    (sp-port-type-file (set status (sp-file-close a))))
  (if status-success?
    (struct-pointer-set a flags (bit-or sp-port-bit-closed (struct-pointer-get a flags))))
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
