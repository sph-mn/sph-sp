(pre-include-once
  inttypes-h "inttypes.h"
  stdio-h "stdio.h")

(pre-define
  ; short fixed-length type names derived from inttypes.h
  boolean
  b8
  pointer-t
  uintptr_t
  b0
  void
  b8
  uint8_t
  b16
  uint16_t
  b32
  uint32_t
  b64
  uint64_t
  b8_s
  int8_t
  b16_s
  int16_t
  b32_s
  int32_t
  b64_s
  int64_t
  f32_s
  float
  f64_s
  double)

(pre-define (debug-log format ...)
  "writes values with current routine name and line info to standard output.
  example: (debug-log \"%d\" 1)
  otherwise like printf"
  (fprintf stdout (pre-string-concat "%s:%d " format "\n") __func__ __LINE__ __VA_ARGS__))

; typical definition of null as seen in other libraries
(pre-define null (convert-type 0 b0))
(pre-define (zero? a) (= 0 a))
