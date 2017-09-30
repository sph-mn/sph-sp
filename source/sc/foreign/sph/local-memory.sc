(pre-define (local-memory-init register-size)
  "register memory in a local variable to free all memory allocated at point"
  (define sph-local-memory-register[register-size] b0*) (define sph-local-memory-index b8 0))

(pre-define (local-memory-add address)
  "do not try to add more entries than specified by register-size or a buffer overflow occurs"
  (set (deref sph-local-memory-register sph-local-memory-index) address
    sph-local-memory-index (+ 1 sph-local-memory-index)))

(pre-define local-memory-free
  (while sph-local-memory-index (set sph-local-memory-index (- sph-local-memory-index 1))
    (free (deref (+ sph-local-memory-register sph-local-memory-index)))))
