(pre-include "stdio.h" "stdlib.h"
  "errno.h" "pthread.h" "float.h" "math.h" "../main/sph-sp.h" "sph/string.c" "sph/filesystem.c")

(pre-define
  (test-helper-test-one func) (begin (printf "%s\n" (pre-stringify func)) (status-require (func)))
  (test-helper-assert description expression)
  (if (not expression) (begin (printf "%s failed\n" description) (status-set-goto "sph-sp" 1)))
  (test-helper-display-summary)
  (if status-is-success (printf "--\ntests finished successfully.\n")
    (printf "\ntests failed. %d %s\n" status.id (sp-status-description status))))

(define (test-helper-event-generate start end out event)
  (status-t sp-time-t sp-time-t sp-block-t sp-event-t*)
  status-declare
  (declare i sp-time-t ci sp-channel-count-t value uint64-t)
  (set value (convert-type (convert-type event:data uint64-t) sp-time-t))
  (for ((set i start) (< i end) (set+ i 1))
    (for ((set ci 0) (< ci out.channel-count) (set+ ci 1))
      (set (array-get out.samples ci (- i start)) value)))
  status-return)

(define (test-helper-event start end number) (sp-event-t sp-time-t sp-time-t sp-time-t)
  (sp-declare-event e)
  (set
    e.start start
    e.end end
    e.generate test-helper-event-generate
    e.data (convert-type (convert-type number uint64-t) void*))
  (return e))