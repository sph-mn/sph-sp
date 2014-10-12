(library (sph sp)
  (export
    sp-io-alsa-open
    sp-io-file-open
    sp-io-port-close
    sp-io-ports-write
    sp-io-stream)
  (import
    (guile)
    (sph))

  (load-extension "libguile-sp" "init_sp")

  (let
    ( (out-file (sp-io-file-open "/tmp/test.au" O_RDWR 3 8000))
      (out-segment (make-f32vector 8000 0)))
    (debug-log (sp-io-ports-write (list out-file) (list out-segment out-segment out-segment)))
    (sp-io-port-close out-file)))