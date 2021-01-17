(pre-include "stdio.h" "stdlib.h"
  "errno.h" "pthread.h" "float.h"
  "math.h" "../main/sph-sp.h" "sph/helper.c" "sph/memreg.c" "sph/string.c" "sph/filesystem.c")

(pre-define
  (test-helper-test-one func) (begin (printf "%s\n" (pre-stringify func)) (status-require (func)))
  (test-helper-assert description expression)
  (if (not expression) (begin (printf "%s failed\n" description) (status-set-goto "sph-sp" 1)))
  (test-helper-display-summary)
  (if status-is-success (printf "--\ntests finished successfully.\n")
    (printf "\ntests failed. %d %s\n" status.id (sp-status-description status))))

(define (test-helper-event-f start end out state) (void sp-time-t sp-time-t sp-block-t void*)
  (declare i sp-time-t channel-i sp-channel-count-t)
  (for ((set i 0) (< i (- end start)) (set+ i 1))
    (for ((set channel-i 0) (< channel-i out.channels) (set+ channel-i 1))
      (set (array-get out.samples channel-i i)
        (convert-type (convert-type state uint64-t) sp-time-t)))))

(define (test-helper-event start end number) (sp-event-t sp-time-t sp-time-t sp-time-t)
  (declare e sp-event-t)
  (set
    e.start start
    e.end end
    e.generate test-helper-event-f
    e.free 0
    e.state (convert-type (convert-type number uint64-t) void*))
  (return e))