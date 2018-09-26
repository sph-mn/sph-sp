(sc-comment "shorter type names derived from inttypes.h")
(pre-include "inttypes.h")

(pre-define
  boolean uint8-t
  i8 int8-t
  i16 int16-t
  i32 int32-t
  i64 int64-t
  i8-least int-least8-t
  i16-least int-least16-t
  i32-least int-least32-t
  i64-least int-least64-t
  i8-fast int-fast8-t
  i16-fast int-fast16-t
  i32-fast int-fast32-t
  i64-fast int-fast64-t
  ui8 uint8-t
  ui16 uint16-t
  ui32 uint32-t
  ui64 uint64-t
  ui8-least uint-least8-t
  ui16-least uint-least16-t
  ui32-least uint-least32-t
  ui64-least uint-least64-t
  ui8-fast uint-fast8-t
  ui16-fast uint-fast16-t
  ui32-fast uint-fast32-t
  ui64-fast uint-fast64-t
  f32 float
  f64 double)