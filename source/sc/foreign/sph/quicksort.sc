(sc-comment
  "a generic quicksort implementation."
  "based on the public domain implementation from http://alienryderflex.com/quicksort/")

(define (quicksort less? swap element-size array array-len)
  (void
    (function-pointer uint8-t void* void*) (function-pointer void void* void*) uint8-t void* size-t)
  "less should return true if the first argument is < than the second.
   swap should exchange the values of the two arguments it receives"
  (declare
    pivot char*
    a char*
    i size-t
    j size-t)
  (if (< array-len 2) return)
  (set
    a array
    pivot (+ a (* element-size (/ array-len 2)))
    i 0
    j (- array-len 1))
  (while #t
    (while (less? (+ a (* element-size i)) pivot)
      (set i (+ 1 i)))
    (while (less? pivot (+ a (* element-size j)))
      (set j (- j 1)))
    (if (>= i j) break)
    (swap (+ a (* element-size i)) (+ a (* element-size j)))
    (set
      i (+ 1 i)
      j (- j 1)))
  (quicksort less? swap element-size a i)
  (quicksort less? swap element-size (+ a (* element-size i)) (- array-len i)))