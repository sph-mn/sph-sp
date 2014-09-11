(library (sph sp)
  (export)
  (import
    (sph base)
    (only (guile) load-extension))

  (load-extension "libguile-sp" "init_sp"))