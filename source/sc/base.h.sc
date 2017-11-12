(pre-include-once
  alsa-asoundlib-h "alsa/asoundlib.h")

(sc-include "base/foreign/sph" "base/foreign/sph/status" "base/status" "base/config" "base/io.h")
(pre-define (sp-octets->samples a) (/ a (sizeof sp-sample-t)))
(pre-define (sp-samples->octets a) (* a (sizeof sp-sample-t)))
(pre-define (duration->sample-count seconds sample-rate) (* seconds sample-rate))
(pre-define (sample-count->duration sample-count sample-rate) (/ sample-count sample-rate))
;
;-- memory

(pre-define (sp-alloc-require a)
  "set sph/status object to group sp and id memory-error and goto \"exit\" label if \"a\" is 0"
  (if (not a) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))

(pre-define (sp-alloc-set a octet-count)
  (set a (malloc octet-count))
  (sp-alloc-require a))

(pre-define (sp-alloc-set-zero a octet-count)
  (set a (calloc octet-count 1))
  (sp-alloc-require a))

(pre-define (sp-alloc-define id type octet-count)
  (define id type)
  (sp-alloc-set id octet-count))

(pre-define (sp-alloc-define-zero id type octet-count)
  (define id type)
  (sp-alloc-set-zero id octet-count))

(pre-define (sp-alloc-define-samples id sample-count)
  (sp-alloc-define id sp-sample-t* (* sample-count (sizeof sp-sample-t))))

(pre-define (sp-alloc-set-samples a sample-count)
  (sp-alloc-set a (* sample-count (sizeof sp-sample-t))))

(pre-define (sp-alloc-define-samples-zero id sample-count)
  (sp-alloc-define-zero id sp-sample-t* (sp-samples->octets (sizeof sp-sample-t))))

(pre-define (sp-alloc-set-samples-zero a sample-count)
  (sp-alloc-set-zero a (sp-samples->octets sample-count)))

(define (sp-alloc-channel-array channel-count sample-count) (sp-sample-t** b32 b32))
