(pre-include-guard-begin sph-sp-h)
(pre-include "byteswap.h" "inttypes.h" "sys/types.h" "sph-sp/float.h" "sph-sp/random.h")
(sc-comment "configuration. changes need recompilation of sph-sp shared library")

(pre-define-if-not-defined
  sp-channel-count-t uint8-t
  sp-channel-limit 2
  sp-cheap-filter-passes-limit 8
  spline-path-value-t sp-sample-t
  sp-random-seed 1557083953
  sp-sample-nearly-equal sph-f64-nearly-equal
  sp-sample-random-primitive sph-random-f64-1to1
  sp-samples-nearly-equal sph-f64-array-nearly-equal
  sp-samples-random-bounded-primitive sph-random-f64-bounded-array
  sp-samples-random-primitive sph-random-f64-array-1to1
  sp-samples-sum sph-f64-sum
  sp-sample-t double
  sp-size-t sp-time-t
  sp-ssize-t int32-t
  sp-time-half-t uint16-t
  sp-time-random-bounded-primitive sph-random-u32-bounded
  sp-time-random-primitive sph-random-u32
  sp-times-random-bounded-primitive sph-random-u32-bounded-array
  sp-times-random-primitive sph-random-u32-array
  sp-time-t uint32-t
  sp-unit-random-primitive sph-random-f64-0to1
  sp-units-random-primitive sph-random-f64-array-0to1
  sp-unit-t double)

(pre-include "string.h" "stdio.h"
  "sph-sp/status.h" "sph-sp/array3.c" "sph-sp/hashtable.c"
  "sph-sp/helper.h" "sph-sp/memreg.c" "sph-sp/set.c" "sph-sp/spline-path.h")

(pre-include "sph-sp/main.h" "sph-sp/arrays.h"
  "sph-sp/path.h" "sph-sp/filter.h" "sph-sp/sequencer.h"
  "sph-sp/statistics.h" "sph-sp/plot.h" "sph-sp/main2.h")

(pre-include-guard-end)