(library (sph sp)
  (export
    sp-io-alsa-open
    sp-io-file-open
    sp-io-port-close
    sp-io-stream)
  (import
    (guile)
    (sph))

  (load-extension "libguile-sp" "init_sp")
  (let (out (sp-io-file-open "/tmp/test.au" O_RDWR 1 8000)) (debug-log out)
    ()
    (sp-io-port-close out)))