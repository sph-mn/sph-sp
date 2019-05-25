(sc-comment "depends on random-h.c")
(pre-define (rotl x k) (bit-or (bit-shift-left x k) (bit-shift-right x (- 64 k))))

(define (sph-random-state-new seed) (sph-random-state-t u64)
  "use the given u64 as a seed and set state with splitmix64 results.
   the same seed will lead to the same series of random numbers from sp-random"
  (declare i u8 z u64 result sph-random-state-t)
  (for ((set i 0) (< i 4) (set i (+ 1 i)))
    (set
      seed (+ seed (UINT64_C 11400714819323198485))
      z seed
      z (* (bit-xor z (bit-shift-right z 30)) (UINT64_C 13787848793156543929))
      z (* (bit-xor z (bit-shift-right z 27)) (UINT64_C 10723151780598845931))
      (array-get result.data i) (bit-xor z (bit-shift-right z 31))))
  (return result))

(define-sph-random sph-random u32 f64 (f64-from-u64 result-plus))