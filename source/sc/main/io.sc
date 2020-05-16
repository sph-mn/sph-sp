(define (sp-file-close file) (status-t sp-file-t*)
  status-declare
  (set status.id (sf-close (convert-type file:data SNDFILE*)))
  (if status-is-success (set status.group sp-s-group-sndfile)
    (set file:flags (bit-or sp-file-bit-closed file:flags)))
  status-return)

(define (sp-file-open path mode channel-count sample-rate result-file)
  (status-t uint8-t* int sp-channels-t sp-sample-rate-t sp-file-t*)
  status-declare
  (declare info SF_INFO file SNDFILE* sf-mode int flags uint8-t)
  (case = mode
    (sp-file-mode-write
      (set
        sf-mode SFM-WRITE
        flags sp-file-bit-output
        info.format sp-file-format
        info.channels channel-count
        info.samplerate sample-rate))
    (sp-file-mode-read
      (set sf-mode SFM-READ flags sp-file-bit-input info.format 0 info.channels 0 info.samplerate 0))
    (sp-file-mode-read-write
      (set
        sf-mode SFM-RDWR
        flags (bit-or sp-file-bit-input sp-file-bit-output)
        info.format sp-file-format
        info.channels channel-count
        info.samplerate sample-rate))
    (else (status-set-goto sp-s-group-sp sp-s-id-file-type)))
  (set file (sf-open path sf-mode &info))
  (if (not file) (status-set-goto sp-s-group-sndfile (sf-error file)))
  (set
    result-file:flags (if* info.seekable (bit-or sp-file-bit-position flags) flags)
    result-file:channel-count info.channels
    result-file:sample-rate info.samplerate
    result-file:data file)
  (label exit status-return))

(define (sp-file-write file channel-data sample-count result-sample-count)
  (status-t sp-file-t* sp-sample-t** sp-time-t sp-time-t*)
  "failure status if not all samples could be written (sp-s-id-file-incomplete)"
  status-declare
  (declare
    channel-count sp-channels-t
    snd_file SNDFILE*
    interleaved sp-sample-t*
    interleaved-size sp-time-t
    frames-count sf-count-t)
  (memreg-init 1)
  (set
    channel-count file:channel-count
    snd_file file:data
    interleaved-size (* channel-count sample-count (sizeof sp-sample-t)))
  (status-require (sph-helper-malloc interleaved-size &interleaved))
  (memreg-add interleaved)
  (sp-interleave channel-data interleaved sample-count channel-count)
  (set frames-count (sp-sf-write snd_file interleaved sample-count))
  (if (not (= sample-count frames-count)) (status-set sp-s-group-sp sp-s-id-file-incomplete))
  (set *result-sample-count frames-count)
  (label exit memreg-free status-return))

(define (sp-file-read file sample-count result-channel-data result-sample-count)
  (status-t sp-file-t* sp-time-t sp-sample-t** sp-time-t*)
  "failure status only if no results read (sp-s-id-eof)"
  status-declare
  (declare
    channel-count sp-channels-t
    interleaved-size sp-time-t
    interleaved sp-sample-t*
    frames-count sf-count-t)
  (memreg-init 1)
  (set
    channel-count file:channel-count
    interleaved-size (* channel-count sample-count (sizeof sp-sample-t)))
  (status-require (sph-helper-malloc interleaved-size &interleaved))
  (memreg-add interleaved)
  (set frames-count (sp-sf-read (convert-type file:data SNDFILE*) interleaved sample-count))
  (if frames-count (sp-deinterleave result-channel-data interleaved frames-count channel-count)
    (status-set sp-s-group-sp sp-s-id-eof))
  (set *result-sample-count frames-count)
  (label exit memreg-free status-return))

(define (sp-file-position-set file a) (status-t sp-file-t* sp-time-t)
  "seeks are defined in number of (multichannel) frames.
   therefore, a seek in a stereo file from the current position forward with an offset of 1 would skip forward by one sample of both channels"
  status-declare
  (declare snd_file SNDFILE* count sf-count-t)
  (set snd_file file:data count (sf-seek snd_file a SEEK-SET))
  (if (= count -1) (status-set-goto sp-s-group-sndfile (sf-error snd_file)))
  (label exit status-return))

(define (sp-file-position file result-position) (status-t sp-file-t* sp-time-t*)
  status-declare
  (declare snd_file SNDFILE* count sf-count-t)
  (set snd_file file:data count (sf-seek snd_file 0 SEEK-CUR))
  (if (= count -1) (status-set-goto sp-s-group-sndfile (sf-error snd_file)))
  (set *result-position count)
  (label exit status-return))

(define (sp-file-input? a) (boolean sp-file-t*) (return (bit-and sp-file-bit-input a:flags)))
(define (sp-file-output? a) (boolean sp-file-t*) (return (bit-and sp-file-bit-output a:flags)))

(define (sp-file-input-output? a) (boolean sp-file-t*)
  (return (and (bit-and sp-file-bit-input a:flags) (bit-and sp-file-bit-output a:flags))))

(define (sp-block->file block path rate) (status-t sp-block-t uint8-t* sp-time-t)
  status-declare
  (declare file sp-file-t written sp-time-t)
  (status-require (sp-file-open path sp-file-mode-write block.channels rate &file))
  (status-require (sp-file-write &file block.samples block.size &written))
  (sp-file-close &file)
  (label exit status-return))