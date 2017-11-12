(pre-define
  sp-sample-t f64-s
  sp-float-t f64-s
  sp-default-sample-rate 16000
  sp-default-channel-count 1
  sp-default-alsa-enable-soft-resample #t
  sp-float-sum f64-sum
  sp-default-alsa-latency 128)

(define (sample-reverse-endian a) (sp-sample-t sp-sample-t)
  "reverse the byte order of one sample"
  (define result sp-sample-t)
  (define b b8* (convert-type (address-of a) b8*))
  (define c b8* (convert-type (address-of result) b8*))
  (set (deref c) (deref b 3))
  (set (deref c 1) (deref b 2))
  (set (deref c 2) (deref b 1))
  (set (deref c 3) (deref b))
  (return result))
