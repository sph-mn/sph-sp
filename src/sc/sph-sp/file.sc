(sc-comment "32 bit float wav file writing and reading. http://soundfile.sapp.org/doc/WaveFormat")

(pre-define
  wav-string-riff (htonl 0x52494646)
  wav-string-fmt (htonl 0x666d7420)
  wav-string-wav (htonl 0x57415645)
  wav-string-data (htonl 0x64617461))

(define (sp-file-open-write path channel-count sample-rate file)
  (status-t uint8-t* sp-channel-count-t sp-time-t sp-file-t*)
  status-declare
  (declare header (array uint8-t 44))
  (set
    (pointer-get (convert-type header uint32-t*)) wav-string-riff
    (pointer-get (convert-type (+ 8 header) uint32-t*)) wav-string-wav
    (pointer-get (convert-type (+ 12 header) uint32-t*)) wav-string-fmt
    (pointer-get (convert-type (+ 16 header) uint32-t*)) 16
    (pointer-get (convert-type (+ 20 header) uint16-t*)) 3
    (pointer-get (convert-type (+ 22 header) uint16-t*)) channel-count
    (pointer-get (convert-type (+ 24 header) uint32-t*)) sample-rate
    (pointer-get (convert-type (+ 28 header) uint32-t*)) (* sample-rate channel-count 4)
    (pointer-get (convert-type (+ 32 header) uint16-t*)) (* channel-count 4)
    (pointer-get (convert-type (+ 34 header) uint16-t*)) 32
    (pointer-get (convert-type (+ 36 header) uint32-t*)) wav-string-data
    (pointer-get (convert-type (+ 40 header) uint32-t*)) 0
    file:data-size 0
    file:channel-count channel-count
    file:file (fopen path "w"))
  (if (not file:file) (status-set-goto sp-s-group-libc errno))
  (if (not (fwrite header 40 1 file:file))
    (begin (fclose file:file) (sp-status-set-goto sp-s-id-file-write)))
  (fseek file:file 4 SEEK_CUR)
  (label exit status-return))

(define (sp-file-close-write file) (void sp-file-t*)
  (declare chunk-size uint32-t)
  (set chunk-size (+ 36 file:data-size))
  (if (bit-and file:data-size 1)
    (begin (define pad uint8-t 0) (fwrite &pad 1 1 file:file) (set+ file:data-size 1)))
  (fseek file:file 4 SEEK_SET)
  (fwrite &chunk-size 4 1 file:file)
  (fseek file:file 40 SEEK_SET)
  (fwrite &file:data-size 4 1 file:file)
  (fclose file:file))

(define (sp-file-write file samples sample-count) (status-t sp-file-t* sp-sample-t** sp-time-t)
  status-declare
  (declare file-data float* interleaved-size size-t channel-count sp-channel-count-t)
  (set
    file-data 0
    channel-count file:channel-count
    interleaved-size (* channel-count sample-count 4))
  (srq (sph-malloc interleaved-size &file-data))
  (for-each-index i sp-time-t
    sample-count
    (for-each-index j sp-channel-count-t
      channel-count
      (set (array-get file-data (+ (* i channel-count) j)) (array-get (array-get samples j) i))))
  (if (not (fwrite file-data interleaved-size 1 file:file)) (sp-status-set-goto sp-s-id-file-write))
  (set+ file:data-size interleaved-size)
  (label exit (free file-data) status-return))

(define (sp-file-open-read path file) (status-t uint8-t* sp-file-t*)
  status-declare
  (declare header (array uint8-t 44) subchunk-id uint32-t subchunk-size uint32-t)
  (set file:file (fopen path "r"))
  (if
    (not
      (and (fread header 44 1 file:file)
        (= (pointer-get (convert-type header uint32-t*)) wav-string-riff)
        (= (pointer-get (convert-type (+ 8 header) uint32-t*)) wav-string-wav)
        (= (pointer-get (convert-type (+ 12 header) uint32-t*)) wav-string-fmt)))
    (sp-status-set-goto sp-s-id-file-read))
  (if
    (not
      (and (= 3 (pointer-get (convert-type (+ 20 header) uint16-t*)))
        (= 32 (pointer-get (convert-type (+ 34 header) uint16-t*)))))
    (sp-status-set-goto sp-s-id-file-not-implemented))
  (define channel-count uint16-t (pointer-get (convert-type (+ 22 header) uint16-t*)))
  (if
    (not
      (and (= (pointer-get (convert-type (+ 32 header) uint16-t*)) (* channel-count 4))
        (= (pointer-get (convert-type (+ 28 header) uint32-t*))
          (* (pointer-get (convert-type (+ 24 header) uint16-t*)) channel-count 4))))
    (sp-status-set-goto sp-s-id-file-not-implemented))
  (set
    file:channel-count channel-count
    subchunk-id (pointer-get (convert-type (+ 36 header) uint32-t*))
    subchunk-size (pointer-get (convert-type (+ 40 header) uint32-t*)))
  (while (not (= wav-string-data subchunk-id))
    (fseek file:file subchunk-size SEEK_CUR)
    (if (not (fread &subchunk-id 4 1 file:file)) (sp-status-set-goto sp-s-id-file-read))
    (if (not (fread &subchunk-size 4 1 file:file)) (sp-status-set-goto sp-s-id-file-read)))
  (label exit status-return))

(define (sp-file-read file sample-count samples) (status-t sp-file-t sp-time-t sp-sample-t**)
  status-declare
  (declare interleaved-count sp-time-t file-data float* read sp-time-t)
  (set file-data 0 interleaved-count (* file.channel-count sample-count))
  (srq (sph-malloc (* 4 interleaved-count) &file-data))
  (set read (fread file-data 4 interleaved-count file.file))
  (if (not (= interleaved-count read))
    (if (feof file.file) (if (not read) (sp-status-set-goto sp-s-id-file-eof))
      (sp-status-set-goto sp-s-id-file-read)))
  (for-each-index i sp-time-t
    (/ read file.channel-count)
    (for-each-index j sp-channel-count-t
      file.channel-count
      (set (array-get (array-get samples j) i) (array-get file-data (+ (* i file.channel-count) j)))))
  (label exit (free file-data) status-return))

(define (sp-file-close-read file) (void sp-file-t) (fclose file.file))