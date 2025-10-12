(pre-include-guard-begin sph-sp-h)
(pre-include "byteswap.h" "inttypes.h" "sys/types.h" "sph/float.h" "sph/random.h")
(sc-comment "configuration. changes need recompilation of sph-sp shared library")

(pre-define-if-not-defined
  sp-channel-count-t uint8-t
  sp-channel-count-limit 2
  sp-filter-passes-limit 8
  sp-filter-passes-t uint8-t
  sp-frq-t uint16-t
  sp-random-seed 1557083953
  sp-sample-nearly-equal sph-f64-nearly-equal
  sp-sample-random-primitive sph-random-f64-1to1
  sp-samples-nearly-equal sph-f64-array-nearly-equal
  sp-samples-random-bounded-primitive sph-random-f64-bounded-array
  sp-samples-random-primitive sph-random-f64-array-1to1
  sp-samples-sum sph-f64-sum
  sp-sample-t double
  sp-scale-t uint64-t
  sp-render-block-seconds 4
  sp-size-t sp-time-t
  sp-ssize-t int32-t
  sp-stime-t int32-t
  sp-time-random-bounded-primitive sph-random-u32-bounded
  sp-time-random-primitive sph-random-u32
  sp-times-random-bounded-primitive sph-random-u32-bounded-array
  sp-times-random-primitive sph-random-u32-array
  sp-time-t uint32-t
  sp-unit-random-primitive sph-random-f64-0to1
  sp-units-random-primitive sph-random-f64-array-0to1
  sp-unit-t double
  sp-pow pow
  sp-exp exp)

(pre-include "string.h" "stdio.h"
  "stdlib.h" "sph/status.h" "sph/array3.h"
  "sph/helper.h" "sph/memreg.h" "sph-sp/main.h"
  "sph-sp/arrays.h" "sph-sp/distributions.h" "sph/spline-path.h"
  "sph-sp/path.h" "sph-sp/filter.h" "sph/queue.h"
  "sph/thread-pool.h" "sph/futures.h" "sph-sp/sequencer.h"
  "sph-sp/parallel.h" "sph-sp/statistics.h" "sph-sp/plot.h" "sph-sp/main2.h")

(pre-include-guard-end)
