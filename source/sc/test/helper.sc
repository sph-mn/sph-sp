(pre-include "stdio.h" "stdlib.h"
  "errno.h" "pthread.h" "float.h"
  "math.h" "../main/sph-sp.h" "../foreign/sph/float.c"
  "../foreign/sph/helper.c" "../foreign/sph/memreg.c" "../foreign/sph/string.c"
  "../foreign/sph/filesystem.c")

(pre-define
  (test-helper-test-one func) (begin (printf "%s\n" (pre-stringify func)) (s (func)))
  (test-helper-assert description expression)
  (if (not expression) (begin (printf "%s failed\n" description) (s-set-goto "sph-sp" 1)))
  (test-helper-display-summary)
  (if s-is-success (printf "--\ntests finished successfully.\n")
    (printf "\ntests failed. %d %s\n" s-current.id (sp-status-description s-current))))

(define (debug-display-sample-array a len) (void sp-sample-t* sp-time-t)
  "display a sample array in one line"
  (declare i sp-time-t)
  (printf "%.17g" (array-get a 0))
  (for ((set i 1) (< i len) (set i (+ 1 i))) (printf " %.17g" (array-get a i)))
  (printf "\n"))