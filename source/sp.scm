(library (sph sp)
  (export
    sp-io-alsa-open-input
    sp-io-alsa-open-output
    sp-io-alsa-read
    sp-io-alsa-write
    sp-io-file-open-input
    sp-io-file-open-output
    sp-io-file-read
    sp-io-file-write
    sp-io-port-close
    sp-io-stream)
  (import
    (guile)
    (sph))

  (load-extension "libguile-sp" "init_sp")

  (let
    ((out-file (sp-io-file-open-output "/tmp/test.au" 3 8000)) (out-segment (make-f32vector 8 1)))
    (debug-log (sp-io-file-write out-file 8 (list out-segment out-segment out-segment)))
    (sp-io-port-close out-file)))