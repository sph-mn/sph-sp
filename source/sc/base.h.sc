(pre-include-once
  alsa-asoundlib-h "alsa/asoundlib.h")

(sc-include "base/foreign/sph" "base/foreign/sph/status" "base/status" "base/config" "base/io.h")
(pre-define (sp-octets->samples a) (/ a (sizeof sp-sample-t)))
(pre-define (sp-samples->octets a) (* a (sizeof sp-sample-t)))
(define (sp-alloc-channel-array channel-count sample-count) (sp-sample-t** b32 b32))
