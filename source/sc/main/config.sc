(pre-define
  sp-file-format (bit-or SF-FORMAT-AU SF-FORMAT-FLOAT)
  sp-sample-format sp-sample-format-f64
  sp-float-t double
  sp-default-sample-rate 16000
  sp-default-channel-count 1
  sp-default-alsa-enable-soft-resample #t
  sp-default-alsa-latency 128
  sp-channel-count-t uint32-t
  sp-sample-count-t uint32-t
  sp-sample-rate-t uint32-t)