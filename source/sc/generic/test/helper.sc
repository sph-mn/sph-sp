(pre-include
  "stdio.h"
  "stdlib.h"
  "errno.h"
  "pthread.h"
  "float.h" "math.h" "generic/base/foreign/sph/one.c" "generic/base/foreign/sph/local-memory.c")

(pre-define (inc a) (set a (+ 1 a)))
(pre-define (dec a) (set a (- a 1)))

(pre-define
  (test-helper-test-one func) (printf "%s\n" (pre-stringify func)) (status-require (func)))

(pre-define (test-helper-assert description expression)
  (if (not expression)
    (begin
      (printf "%s failed\n" description)
      (status-set-id-goto 1))))

(pre-define (test-helper-display-summary)
  (if status-is-success (printf "--\ntests finished successfully.\n")
    (printf "\ntests failed. %d %s\n" status.id (sp-status-description status))))

(define (debug-log-samples a len) (b0 sp-sample-t* size-t)
  (define
    column-width size-t
    column-end size-t
    index size-t)
  (set
    column-width 8
    index 0)
  (while (< index len)
    (set column-end (+ index column-width))
    (while (and (< index len) (< index column-end))
      (printf "%f " (array-get a index))
      (set index (+ 1 index)))
    (printf "\n")))