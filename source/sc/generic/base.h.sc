(pre-include
  "byteswap.h" "math.h")

(pre-define
  sp-sample-type-f64 1
  sp-sample-type-f32 2)

(pre-include
  "generic/base/foreign/sph"
  "generic/base/foreign/sph/status"
  "generic/base/foreign/sph/float" "generic/base/status" "generic/base/config")

(pre-if
  (= sp-sample-type sp-sample-type-f64)
  (pre-define
    sp-sample-t f64-s
    sample-reverse-endian __bswap_64
    sp-sample-sum f64-sum
    sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT64_LE)
  (pre-if
    (= sp-sample-type-f32 sp-sample-type)
    (pre-define
      sp-sample-t f32-s
      sample-reverse-endian __bswap_32
      sp-sample-sum f32-sum
      sp-alsa-snd-pcm-format SND_PCM_FORMAT_FLOAT_LE)))

(pre-define
  (sp-octets->samples a) (/ a (sizeof sp-sample-t))
  (sp-samples->octets a) (* a (sizeof sp-sample-t))
  (duration->sample-count seconds sample-rate) (* seconds sample-rate)
  (sample-count->duration sample-count sample-rate) (/ sample-count sample-rate))

(pre-include "generic/base/io.h")
;
;-- memory

(pre-define
  (sp-alloc-require a)
  (begin
    "set sph/status object to group sp and id memory-error and goto \"exit\" label if \"a\" is 0"
    (if (not a)
      (status-set-both-goto sp-status-group-sp sp-status-id-memory)))
  (sp-alloc-set a octet-count)
  (begin
    (set a (malloc octet-count))
    (sp-alloc-require a))
  (sp-alloc-set-zero a octet-count)
  (begin
    (set a (calloc octet-count 1))
    (sp-alloc-require a))
  (sp-alloc-define id type octet-count)
  (begin
    (declare id type)
    (sp-alloc-set id octet-count))
  (sp-alloc-define-zero id type octet-count)
  (begin
    (declare id type)
    (sp-alloc-set-zero id octet-count))
  (sp-alloc-define-samples id sample-count)
  (sp-alloc-define id sp-sample-t* (* sample-count (sizeof sp-sample-t)))
  (sp-alloc-set-samples a sample-count) (sp-alloc-set a (* sample-count (sizeof sp-sample-t)))
  (sp-alloc-define-samples-zero id sample-count)
  (sp-alloc-define-zero id sp-sample-t* (sp-samples->octets (sizeof sp-sample-t)))
  (sp-alloc-set-samples-zero a sample-count) (sp-alloc-set-zero a (sp-samples->octets sample-count)))

(define (sp-alloc-channel-array channel-count sample-count) (sp-sample-t** b32 b32))
(define (sp-status-description a) (b8* status-t))
(define (sp-status-name a) (b8* status-t))