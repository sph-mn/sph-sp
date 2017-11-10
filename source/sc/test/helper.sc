(pre-include-once stdio-h "stdio.h"
  stdlib-h "stdlib.h" errno-h "errno.h" pthread-h "pthread.h" float-h "float.h" math-h "math.h")

(pre-define (inc a) (set a (+ 1 a)))
(pre-define (dec a) (set a (- a 1)))

(pre-define (test-helper-test-one func) (printf "%s\n" (pre-stringify func))
  (status-require! (func)))

(pre-define (test-helper-assert description expression)
  (if (not expression) (begin (printf "%s failed\n" description) (status-set-id-goto 1))))

(pre-define (test-helper-display-summary)
  (if status-success? (printf "--\ntests finished successfully.\n")
    (printf "\ntests failed. %d %s\n" (struct-get status id) (sp-status-description status))))

(sc-include "base/foreign/sph/one" "base/foreign/sph/local-memory")
