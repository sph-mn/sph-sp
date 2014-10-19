(library (sph sp)
  (export
    sp-io-alsa-open-input
    sp-io-alsa-open-output
    sp-io-alsa-read
    sp-io-alsa-write
    sp-io-file-open-input
    sp-io-file-open-output
    sp-io-file-read
    sp-io-file-set-position
    sp-io-file-write
    sp-io-port-channel-count
    sp-io-port-close
    sp-io-port-input?
    sp-io-port-position
    sp-io-port-position?
    sp-io-port-samples-per-second
    sp-io-port?
    sp-io-stream)
  (import
    (guile)
    (sph))

  (load-extension "libguile-sp" "init_sp"))