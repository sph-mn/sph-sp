(pre-include "inttypes.h" "stdio.h")

(pre-define
  ; shorter fixed-length type names derived from inttypes.h
  boolean uint8-t
  pointer-t uintptr_t
  void void
  uint8-t uint8_t
  uint16-t uint16_t
  uint32-t uint32_t
  uint64-t uint64_t
  int8-t int8_t
  int16-t int16_t
  int32-t int32_t
  int64-t int64_t
  f32-s float
  f64-s double
  (debug-log format ...)
  (begin
    "writes values with current routine name and line info to standard output.
    example: (debug-log \"%d\" 1)
    otherwise like printf"
    (fprintf stdout (pre-string-concat "%s:%d " format "\n") __func__ __LINE__ __VA_ARGS__))
  ; definition of null as seen in other libraries
  null (convert-type 0 void)
  (zero? a) (= 0 a))