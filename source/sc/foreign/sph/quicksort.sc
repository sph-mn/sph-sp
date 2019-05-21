(sc-comment "a generic quicksort implementation."
  "based on the public domain implementation from http://alienryderflex.com/quicksort/")

(define (quicksort less? swap array size offset)
  (void (function-pointer uint8-t void* size-t size-t) (function-pointer void void* size-t size-t)
    void* size-t size-t)
  "less should return true if the first argument is < than the second.
   swap should exchange the values of the two arguments it receives"
  (declare pivot size-t a size-t i size-t j size-t)
  (if (< size 2) return)
  (set pivot (+ offset (/ size 2)) i 0 j (- size 1))
  (while #t
    (while (less? array (+ offset i) pivot) (set i (+ 1 i)))
    (while (less? array pivot (+ offset j)) (set j (- j 1)))
    (if (>= i j) break)
    (swap array (+ offset i) (+ offset j))
    (set i (+ 1 i) j (- j 1)))
  (quicksort less? swap array i offset)
  (quicksort less? swap array (- size i) (+ offset i)))