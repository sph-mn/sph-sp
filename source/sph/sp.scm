(library (sph sp)
  (export
    sp-deinit-alsa
    sp-init-alsa
    sp-loop-alsa
    sp-use-alsa)
  (import
    (sph base)
    (only (guile) load-extension))

  (load-extension "libguile-sp" "init_sp")
  (define-syntax-rule (sp-use-alsa body ...) (if (sp-init-alsa) (begin body ... (sp-deinit-alsa)))))