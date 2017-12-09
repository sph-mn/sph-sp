(pre-include-once
  stdio-h "stdio.h"
  fcntl-h "fcntl.h"
  alsa-asoundlib-h "alsa/asoundlib.h"
  sndfile-h "sndfile.h")

(sc-include "generic/base.h" "generic/base/foreign/sph/one" "generic/base/foreign/sph/local-memory")
;
;-- other
;
(pre-define (inc a) (set a (+ 1 a)))
(pre-define (dec a) (set a (- a 1)))

(define (sp-status-description a) (b8* status-t)
  (return
    (case* = a.group
      (sp-status-group-sp
        (convert-type
          (case* = a.id
            (sp-status-id-eof "end of file")
            (sp-status-id-input-type "input argument is of wrong type")
            (sp-status-id-not-implemented "not implemented")
            (sp-status-id-memory "memory allocation error")
            (sp-status-id-file-incompatible
              "file channel count or sample rate is different from what was requested")
            (sp-status-id-file-incomplete "incomplete write")
            (sp-status-id-port-type "incompatible port type")
            (else ""))
          b8*))
      (sp-status-group-alsa (convert-type (sf-error-number a.id) b8*))
      (sp-status-group-sndfile (convert-type (sf-error-number a.id) b8*))
      (else (convert-type "" b8*)))))

(define (sp-status-name a) (b8* status-t)
  (return
    (case* = a.group
      (sp-status-group-sp
        (convert-type
          (case* = a.id
            (sp-status-id-input-type "input-type")
            (sp-status-id-not-implemented "not-implemented")
            (sp-status-id-memory "memory")
            (else "unknown"))
          b8*))
      (sp-status-group-alsa (convert-type "alsa" b8*))
      (sp-status-group-sndfile (convert-type "sndfile" b8*))
      (else (convert-type "unknown" b8*)))))

(define (sp-alloc-channel-array channel-count sample-count) (sp-sample-t** b32 b32)
  "return an array for channels with data arrays for each channel.
  returns zero if memory could not be allocated"
  (local-memory-init (+ channel-count 1))
  (define result sp-sample-t** (malloc (* channel-count (sizeof sp-sample-t*))))
  (if (not result) (return 0))
  (local-memory-add result)
  (define channel sp-sample-t*)
  (while channel-count
    (dec channel-count)
    (set channel (calloc (* sample-count (sizeof sp-sample-t)) 1))
    (if (not channel)
      (begin
        local-memory-free
        (return 0)))
    (local-memory-add channel)
    (set (deref result channel-count) channel))
  (return result))

(sc-include "generic/base/io")
