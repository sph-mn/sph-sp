(sc-comment "depends on types.c. have to use sph- prefix because standard lib uses random")

(pre-define
  (f64-from-u64 a)
  (begin
    "guarantees that all dyadic rationals of the form (k / 2**−53) will be equally likely. this conversion prefers the high bits of x.
     from http://xoshiro.di.unimi.it/"
    (* (bit-shift-right a 11) (/ 1.0 (bit-shift-left (UINT64_C 1) 53))))
  (define-sph-random name size-type data-type transfer)
  (define (name state size out) (void sph-random-state-t* size-type data-type*)
    "return uniformly distributed random real numbers in the range -1 to 1.
     implements xoshiro256plus from http://xoshiro.di.unimi.it/
     referenced by https://nullprogram.com/blog/2017/09/21/"
    (declare result-plus u64 i size-type t u64 s sph-random-state-t)
    (set s *state)
    (for ((set i 0) (< i size) (set i (+ 1 i)))
      (set
        result-plus (+ (array-get s.data 0) (array-get s.data 3))
        t (bit-shift-left (array-get s.data 1) 17)
        (array-get s.data 2) (bit-xor (array-get s.data 2) (array-get s.data 0))
        (array-get s.data 3) (bit-xor (array-get s.data 3) (array-get s.data 1))
        (array-get s.data 1) (bit-xor (array-get s.data 1) (array-get s.data 2))
        (array-get s.data 0) (bit-xor (array-get s.data 0) (array-get s.data 3))
        (array-get s.data 2) (bit-xor (array-get s.data 2) t)
        (array-get s.data 3) (rotl (array-get s.data 3) 45)
        (array-get out i) transfer))
    (set *state s)))

(declare
  sph-random-state-t (type (struct (data (array u64 4))))
  (sph-random-state-new seed) (sph-random-state-t u64)
  (sph-random state size out) (void sph-random-state-t* u32 f64*))