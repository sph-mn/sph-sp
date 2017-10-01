(pre-define (local-memory-init register-size)
  "register memory in a local variable to free all memory allocated at point"
  (define-array sph-local-memory-register b0* (register-size)) (define sph-local-memory-index b8 0))

(pre-define (local-memory-add address)
  "do not try to add more entries than specified by register-size or a buffer overflow occurs"
  (set (deref sph-local-memory-register sph-local-memory-index) address
    sph-local-memory-index (+ 1 sph-local-memory-index)))

(pre-define local-memory-free
  (while sph-local-memory-index
    (set sph-local-memory-index (- sph-local-memory-index 1))
    (debug-log "index %lu freeing %p" sph-local-memory-index
      (deref (+ sph-local-memory-register sph-local-memory-index)))
    (free (deref (+ sph-local-memory-register sph-local-memory-index)))))
