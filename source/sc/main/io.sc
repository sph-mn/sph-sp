(define (sp-file-close port) (status-t sp-port-t*)
  status-declare
  (set status.id (sf-close (convert-type port:data SNDFILE*)))
  (if status-is-success (set status.group sp-status-group-sndfile)
    (set port:flags (bit-or sp-port-bit-closed port:flags)))
  (return status))

(define (sp-file-open path channel-count sample-rate result-port)
  (status-t uint8-t* sp-channel-count-t sp-sample-rate-t sp-port-t*)
  status-declare
  (declare
    bit-position uint8-t
    info SF_INFO
    file SNDFILE*
    mode int)
  (set
    info.format sp-file-format
    info.channels channel-count
    info.samplerate sample-rate
    mode SFM-RDWR
    file (sf-open path mode &info))
  (if (not file) (status-set-both-goto sp-status-group-sndfile (sf-error file)))
  (set
    bit-position
    (if* info.seekable sp-port-bit-position
      0)
    result-port:channel-count channel-count
    result-port:sample-rate sample-rate
    result-port:type sp-port-type-file
    result-port:flags (bit-or sp-port-bit-input sp-port-bit-output bit-position)
    result-port:data file)
  (label exit
    (return status)))

(define (sp-file-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**)
  status-declare
  (declare
    channel-count sp-channel-count-t
    file SNDFILE*
    interleaved sp-sample-t*
    interleaved-size size-t
    write-count sf-count-t)
  (memreg-init 1)
  (set
    channel-count port:channel-count
    file port:data
    interleaved-size (* channel-count sample-count (sizeof sp-sample-t*)))
  (status-require (sph-helper-malloc interleaved-size &interleaved))
  (memreg-add interleaved)
  (sp-interleave channel-data interleaved sample-count channel-count)
  (set write-count (sf-writef-double file interleaved sample-count))
  (if (not (= sample-count write-count))
    (status-set-both-goto sp-status-group-sp sp-status-id-file-incomplete))
  (label exit
    memreg-free
    (return status)))

(define (sp-file-read port sample-count result-channel-data)
  (status-t sp-port-t* size-t sp-sample-t**)
  status-declare
  (declare
    channel-count sp-channel-count-t
    interleaved-size size-t
    interleaved sp-sample-t*
    read-count sf-count-t)
  (memreg-init 1)
  (set
    channel-count port:channel-count
    interleaved-size (* channel-count sample-count (sizeof sp-sample-t)))
  (status-require (sph-helper-malloc interleaved-size &interleaved))
  (memreg-add interleaved)
  (set read-count (sf-readf-double (convert-type port:data SNDFILE*) interleaved sample-count))
  (if (not (= interleaved-size read-count)) (status-set-both sp-status-group-sp sp-status-id-eof))
  (sp-deinterleave result-channel-data interleaved read-count channel-count)
  (label exit
    memreg-free
    (return status)))

(define (sp-file-set-position port a) (status-t sp-port-t* size-t)
  status-declare
  (declare
    file SNDFILE*
    count sf-count-t)
  (set
    file port:data
    count (sf-seek file a SEEK-SET))
  (if (= count -1) (status-set-both-goto sp-status-group-sndfile (sf-error file)))
  (label exit
    (return status)))

(define (sp-file-position port result-position) (status-t sp-port-t* size-t*)
  status-declare
  (declare
    file SNDFILE*
    count sf-count-t)
  (set
    file port:data
    count (sf-seek file 0 SEEK-CUR))
  (if (= count -1) (status-set-both-goto sp-status-group-sndfile (sf-error file)))
  (set *result-position count)
  (label exit
    (return status)))

(define (sp-alsa-open device-name is-input channel-count sample-rate latency result-port)
  (status-t uint8-t* boolean sp-channel-count-t sp-sample-rate-t int32-t sp-port-t*)
  "open alsa sound output for capture or playback"
  status-declare
  (declare alsa-port snd-pcm-t*)
  (if (not device-name) (set device-name "default"))
  (optional-set-number latency sp-default-alsa-latency)
  (set alsa-port 0)
  (sp-alsa-status-require
    (snd-pcm-open
      &alsa-port device-name
      (if* is-input SND_PCM_STREAM_CAPTURE
        SND_PCM_STREAM_PLAYBACK)
      0))
  (sp-alsa-status-require
    (snd-pcm-set-params
      alsa-port
      sp-alsa-snd-pcm-format
      SND_PCM_ACCESS_RW_NONINTERLEAVED
      channel-count sample-rate sp-default-alsa-enable-soft-resample latency))
  (set
    result-port:type sp-port-type-alsa
    result-port:flags
    (if* is-input sp-port-bit-input
      sp-port-bit-output)
    result-port:data (convert-type alsa-port void*))
  (label exit
    (if (and status-is-failure alsa-port) (snd-pcm-close alsa-port))
    (return status)))

(define (sp-alsa-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**)
  status-declare
  (declare
    alsa-port snd-pcm-t*
    frames-count snd_pcm_sframes_t)
  (set
    alsa-port port:data
    frames-count
    (snd-pcm-writen
      alsa-port (convert-type channel-data void**) (convert-type sample-count snd_pcm_uframes_t)))
  (if (and (< frames-count 0) (< (snd-pcm-recover alsa-port frames-count 0) 0))
    (status-set-both sp-status-group-alsa frames-count))
  (return status))

(define (sp-alsa-read port sample-count result-channel-data)
  (status-t sp-port-t* sp-sample-count-t sp-sample-t**)
  status-declare
  (declare
    alsa-port snd-pcm-t*
    frames-count snd_pcm_sframes_t)
  (set
    alsa-port port:data
    frames-count (snd-pcm-readn alsa-port (convert-type result-channel-data void**) sample-count))
  (if (and (< frames-count 0) (< (snd-pcm-recover alsa-port frames-count 0) 0))
    (status-set-both sp-status-group-alsa frames-count))
  (return status))

(define (sp-port-position? a) (boolean sp-port-t*) (return (bit-and sp-port-bit-position a:flags)))
(define (sp-port-input? a) (boolean sp-port-t*) (return (bit-and sp-port-bit-input a:flags)))
(define (sp-port-output? a) (boolean sp-port-t*) (return (bit-and sp-port-bit-output a:flags)))

(define (sp-port-read port sample-count result-channel-data)
  (status-t sp-port-t* sp-sample-count-t sp-sample-t**)
  (case = port:type
    (sp-port-type-file (return (sp-file-read port sample-count result-channel-data)))
    (sp-port-type-alsa (return (sp-alsa-read port sample-count result-channel-data)))))

(define (sp-port-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**)
  (case = port:type
    (sp-port-type-file (return (sp-file-write port sample-count channel-data)))
    (sp-port-type-alsa (return (sp-alsa-write port sample-count channel-data)))))

(define (sp-port-close a) (status-t sp-port-t*)
  status-declare
  (case = a:type
    (sp-port-type-alsa (sp-alsa-status-require (snd-pcm-close (convert-type a:data snd-pcm-t*))))
    (sp-port-type-file (set status (sp-file-close a))))
  (if status-is-success (set a:flags (bit-or sp-port-bit-closed a:flags)))
  (label exit
    (return status)))

(define (sp-port-position port result-position) (status-t sp-port-t* size-t*)
  status-declare
  (case = port:type
    (sp-port-type-file (return (sp-file-position port result-position)))
    (sp-port-type-alsa
      (status-set-both sp-status-group-sp sp-status-id-not-implemented) (return status))))

(define (sp-port-set-position port sample-index) (status-t sp-port-t* size-t)
  status-declare
  (case = port:type
    (sp-port-type-file (return (sp-file-set-position port sample-index)))
    (sp-port-type-alsa
      (status-set-both sp-status-group-sp sp-status-id-not-implemented) (return status))))