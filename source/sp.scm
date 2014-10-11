(library (sph sp)
  (export
    sp-io-alsa-open
    sp-io-file-open
    sp-io-ports-close
    sp-io-stream)
  (import
    (sph)
    (only (guile) load-extension O_WRONLY))

  (load-extension "libguile-sp" "init_sp")


  (debug-log (sp-io-file-open "/tmp/test.au" O_WRONLY 1 8000))
  )