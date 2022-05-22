(sc-comment
  "utilities for arrays of sp-time-t or sp-sample-t."
  "sp-time-t subtraction is limited to zero."
  "sp-time-t addition is not limited.")

(sc-include-once "./sc-macros")

(define (sp-shuffle state swap a size)
  (void sp-random-state-t* (function-pointer void void* size-t size-t) void* size-t)
  "generic shuffle that works on any array type. fisher-yates algorithm"
  (declare i size-t j size-t)
  (for ((set i 0) (< i size) (set+ i 1))
    (set j (+ i (sp-time-random-bounded state (- size i))))
    (swap a i j)))

(define (sp-samples-new size out) (status-t sp-time-t sp-sample-t**)
  (return (sph-helper-calloc (* size (sizeof sp-sample-t)) out)))

(define (sp-times-new size out) (status-t sp-time-t sp-time-t**)
  (return (sph-helper-calloc (* size (sizeof sp-time-t)) out)))

(define (sp-samples->times in size out) (void sp-sample-t* sp-time-t sp-time-t*)
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set i (+ 1 i)))
    (set (array-get out i) (sp-cheap-round-positive (array-get in i)))))

(define (sp-samples-absolute-max in in-size) (sp-sample-t sp-sample-t* sp-time-t)
  "get the maximum value in samples array, disregarding sign"
  (declare result sp-sample-t a sp-sample-t i sp-time-t)
  (for ((set i 0 result 0) (< i in-size) (set i (+ 1 i)))
    (set a (fabs (array-get in i)))
    (if (> a result) (set result a)))
  (return result))

(define (sp-times-absolute-max in size) (sp-time-t sp-time-t* sp-time-t)
  "get the maximum value in samples array, disregarding sign"
  (declare a sp-time-t)
  (define max sp-time-t 0)
  (for-each-index i size (set a (sp-abs (array-get in i))) (if (> a max) (set max a)))
  (return max))

(define (sp-samples-display a size) (void sp-sample-t* sp-time-t)
  "display a sample array in one line"
  (declare i sp-time-t)
  (printf "%.5f" (array-get a 0))
  (for ((set i 1) (< i size) (set i (+ 1 i))) (printf " %.5f" (array-get a i)))
  (printf "\n"))

(define (sp-times-display a size) (void sp-time-t* sp-time-t)
  "display a time array in one line"
  (declare i sp-time-t)
  (printf "%lu" (array-get a 0))
  (for ((set i 1) (< i size) (set i (+ 1 i))) (printf " %lu" (array-get a i)))
  (printf "\n"))

(define (sp-samples-set-gain a a-len amp) (void sp-sample-t* sp-time-t sp-sample-t)
  (declare i sp-time-t a-max sp-sample-t difference sp-sample-t correction sp-sample-t)
  (set a-max (sp-samples-absolute-max a a-len))
  (if (or (= 0 amp) (= 0 a-max)) return)
  (set difference (/ a-max amp) correction (+ 1 (/ (- 1 difference) difference)))
  (for ((set i 0) (< i a-len) (set+ i 1)) (set* (array-get a i) correction)))

(define (sp-samples-set-unity-gain in out size) (void sp-sample-t* sp-sample-t* sp-time-t)
  "scale amplitude of out to match the one of in.
   no change if any array is zero"
  (sp-samples-set-gain out size (sp-samples-absolute-max in size)))

(pre-define
  (define-value-functions prefix value-t)
  (begin
    "functions that work on single sp-sample-t and sp-time-t"
    (define ((pre-concat prefix _sort-swap) a b c) (void void* ssize-t ssize-t)
      (declare d value-t)
      (set
        d (array-get (convert-type a value-t*) b)
        (array-get (convert-type a value-t*) b) (array-get (convert-type a value-t*) c)
        (array-get (convert-type a value-t*) c) d))
    (define ((pre-concat prefix _sort-less) a b c) (uint8-t void* ssize-t ssize-t)
      (return (< (array-get (convert-type a value-t*) b) (array-get (convert-type a value-t*) c)))))
  (define-array-functions prefix value-t)
  (begin
    "functions that work the same on sp-sample-t* and sp-time-t*"
    (define ((pre-concat prefix _reverse) a size out) (void value-t* sp-time-t value-t*)
      "a/out can not be the same pointer"
      (declare i sp-time-t)
      (for ((set i 0) (< i size) (set+ i 1)) (set (array-get out (- size i)) (array-get a i))))
    (define ((pre-concat prefix _equal-1) a size n) (uint8-t value-t* sp-time-t value-t)
      (declare i sp-time-t)
      (for ((set i 0) (< i size) (set+ i 1)) (if (not (= n (array-get a i))) (return 0)))
      (return 1))
    (define ((pre-concat prefix _and) a b size limit out)
      (void value-t* value-t* sp-time-t value-t value-t*)
      "if a[i] and b[i] greater than limit, take b[i] else 0"
      (declare i sp-time-t)
      (for ((set i 0) (< i size) (set+ i 1))
        (if (and (>= (array-get a i) limit) (>= (array-get a i) limit))
          (set (array-get out i) (array-get b i))
          (set (array-get out i) 0))))
    (define ((pre-concat prefix _or) a b size limit out)
      (void value-t* value-t* sp-time-t value-t value-t*)
      "if a[i] < limit and b[i] > limit, take b[i], else a[i]"
      (declare i sp-time-t)
      (for ((set i 0) (< i size) (set+ i 1))
        (if (< (array-get a i) limit)
          (if (< (array-get b i) limit) (set (array-get out i) (array-get a i))
            (set (array-get out i) (array-get b i)))
          (set (array-get out i) (array-get a i)))))
    (define ((pre-concat prefix _xor) a b size limit out)
      (void value-t* value-t* sp-time-t value-t value-t*)
      "if a[i] > limit and b[i] > limit then 0 else take the one greater than limit"
      (declare i sp-time-t)
      (for ((set i 0) (< i size) (set+ i 1))
        (if (and (>= (array-get a i) limit) (>= (array-get b i) limit)) (set (array-get out i) 0)
          (if (< (array-get a i) limit)
            (if (< (array-get b i) limit) (set (array-get out i) (array-get a i))
              (set (array-get out i) (array-get b i)))
            (set (array-get out i) (array-get a i)))))))
  (define-array-mapper name value-t transfer)
  (define (name a size out) (void value-t* sp-time-t value-t*)
    (declare i sp-time-t)
    (for ((set i 0) (< i size) (set+ i 1)) (set (array-get out i) transfer)))
  (define-array-combinator name value-t transfer)
  (define (name a size b out) (void value-t* sp-time-t value-t* value-t*)
    "a/out can be the same"
    (declare i sp-time-t)
    (for ((set i 0) (< i size) (set+ i 1)) (set (array-get out i) transfer)))
  (define-array-combinator-1 name value-t transfer)
  (define (name a size n out) (void value-t* sp-time-t value-t value-t*)
    "a/out can be the same"
    (declare i sp-time-t)
    (for ((set i 0) (< i size) (set+ i 1)) (set (array-get out i) transfer))))

(define (sp-samples-every-equal a size n) (uint8-t sp-sample-t* sp-time-t sp-sample-t)
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1)) (if (not (= n (array-get a i))) (return 0)))
  (return 1))

(define (sp-times-copy a size out) (void sp-time-t* sp-time-t sp-time-t*)
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1)) (set (array-get out i) (array-get a i))))

(define (sp-samples-copy a size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1)) (set (array-get out i) (array-get a i))))

(define-value-functions sp-times sp-time-t)
(define-value-functions sp-samples sp-sample-t)
(define-array-functions sp-times sp-time-t)
(define-array-functions sp-samples sp-sample-t)
(define-array-mapper sp-times-square sp-time-t (* (array-get a i) (array-get a i)))
(define-array-mapper sp-samples-square sp-sample-t (* (array-get a i) (array-get a i)))
(define-array-combinator sp-times-add sp-time-t (+ (array-get a i) (array-get b i)))
(define-array-combinator sp-samples-add sp-sample-t (+ (array-get a i) (array-get b i)))

(define-array-combinator sp-times-subtract sp-time-t
  (sp-no-underflow-subtract (array-get a i) (array-get b i)))

(define-array-combinator sp-samples-subtract sp-sample-t (- (array-get a i) (array-get b i)))
(define-array-combinator sp-times-multiply sp-time-t (* (array-get a i) (array-get b i)))
(define-array-combinator sp-samples-multiply sp-sample-t (* (array-get a i) (array-get b i)))
(define-array-combinator sp-times-divide sp-time-t (/ (array-get a i) (array-get b i)))
(define-array-combinator sp-samples-divide sp-sample-t (/ (array-get a i) (array-get b i)))
(define-array-combinator-1 sp-times-add-1 sp-time-t (+ (array-get a i) n))
(define-array-combinator-1 sp-samples-add-1 sp-sample-t (+ (array-get a i) n))

(define-array-combinator-1 sp-times-subtract-1 sp-time-t
  (sp-no-underflow-subtract (array-get a i) n))

(define-array-combinator-1 sp-samples-subtract-1 sp-sample-t (- (array-get a i) n))
(define-array-combinator-1 sp-times-multiply-1 sp-time-t (* (array-get a i) n))
(define-array-combinator-1 sp-times-divide-1 sp-time-t (/ (array-get a i) n))
(define-array-combinator-1 sp-times-set-1 sp-time-t n)
(define-array-combinator-1 sp-samples-set-1 sp-sample-t n)
(define-array-combinator-1 sp-samples-multiply-1 sp-sample-t (* (array-get a i) n))
(define-array-combinator-1 sp-samples-divide-1 sp-sample-t (/ (array-get a i) n))

(define (sp-u64-from-array a size) (uint64-t uint8-t* sp-time-t)
  "lower value parts of large types are preferred if
   the system byte order is as expected little-endian"
  (case = size
    (1 (return *a))
    (2 (return (pointer-get (convert-type a uint16-t*))))
    (3
      (return
        (+ (pointer-get (convert-type a uint16-t*))
          (bit-shift-left (convert-type (array-get a 2) uint64-t) 16))))
    (4 (return (pointer-get (convert-type a uint32-t*))))
    (5
      (return
        (+ (pointer-get (convert-type a uint32-t*))
          (bit-shift-left (convert-type (array-get a 4) uint64-t) 32))))
    (6
      (return
        (+ (pointer-get (convert-type a uint32-t*))
          (bit-shift-left (convert-type (pointer-get (convert-type (+ 4 a) uint16-t*)) uint64-t) 32))))
    (7
      (return
        (+ (pointer-get (convert-type a uint32-t*))
          (bit-shift-left (convert-type (pointer-get (convert-type (+ 4 a) uint16-t*)) uint64-t) 32)
          (bit-shift-left (convert-type (array-get a 6) uint64-t) 48))))
    (8 (return (pointer-get (convert-type a uint64-t*))))))

(sc-comment "samples")

(define (sp-samples-duplicate a size out) (status-t sp-sample-t* sp-time-t sp-sample-t**)
  status-declare
  (declare temp sp-sample-t*)
  (status-require (sp-samples-new size &temp))
  (memcpy temp a (* size (sizeof sp-sample-t)))
  (set *out temp)
  (label exit status-return))

(define (sp-samples-differences a size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  "write to out the differences between subsequent values of a.
   size must be > 1.
   out-size will be size - 1"
  (declare i sp-time-t)
  (for ((set i 1) (< i size) (set+ i 1))
    (set (array-get out (- i 1)) (- (array-get a i) (array-get a (- i 1))))))

(sc-comment "times")

(define (sp-times-duplicate a size out) (status-t sp-time-t sp-time-t sp-time-t**)
  status-declare
  (declare temp sp-time-t*)
  (status-require (sp-times-new size &temp))
  (set *out temp)
  (label exit status-return))

(define (sp-times-differences a size out) (void sp-time-t* sp-time-t sp-time-t*)
  "a.size must be > 1. a.size minus 1 elements will be written to out"
  (declare i sp-time-t)
  (for ((set i 1) (< i size) (set+ i 1))
    (set (array-get out (- i 1)) (sp-absolute-difference (array-get a i) (array-get a (- i 1))))))

(define (sp-times-cusum a size out) (void sp-time-t* sp-time-t sp-time-t*)
  "calculate cumulative sums from the given numbers.
   (a b c ...) -> (a (+ a b) (+ a b c) ...)"
  (declare i sp-time-t sum sp-time-t)
  (set sum (array-get a 0) (array-get out 0) sum)
  (for ((set i 1) (< i size) (set+ i 1)) (set sum (+ sum (array-get a i)) (array-get out i) sum)))

(define (sp-times-random-discrete state cudist cudist-size count out)
  (void sp-random-state-t* sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "generate random integers in the range 0..(cudist-size - 1)
   with probability distribution given via cudist, the cumulative sums of the distribution"
  (declare deviate sp-time-t sum sp-time-t i sp-time-t i1 sp-time-t)
  (set sum (array-get cudist (- cudist-size 1)))
  (for ((set i 0) (< i count) (set+ i 1))
    (set deviate (sp-time-random-bounded state sum))
    (for ((set i1 0) (< i1 cudist-size) (set+ i1 1))
      (if (< deviate (array-get cudist i1)) (begin (set (array-get out i) i1) break)))))

(define (sp-time-random-discrete state cudist cudist-size)
  (sp-time-t sp-random-state-t* sp-time-t* sp-time-t)
  (declare deviate sp-time-t i sp-time-t)
  (set deviate (sp-time-random-bounded state (array-get cudist (- cudist-size 1))))
  (for ((set i 0) (< i cudist-size) (set+ i 1)) (if (< deviate (array-get cudist i)) (return i)))
  (return cudist-size))

(define (sp-time-random-custom state cudist cudist-size range)
  (sp-time-t sp-random-state-t* sp-time-t* sp-time-t sp-time-t)
  "get a random number in range with a custom probability distribution given by cudist,
   the cumulative sums of the distribution. the resulting number resolution is proportional to cudist-size"
  (sc-comment "cudist-size minus one because range end is exclusive")
  (return
    (sp-cheap-round-positive
      (* range
        (/ (sp-time-random-discrete state cudist cudist-size)
          (convert-type (- cudist-size 1) sp-sample-t))))))

(define (sp-sample-random-custom state cudist cudist-size range)
  (sp-sample-t sp-random-state-t* sp-time-t* sp-time-t sp-sample-t)
  (return
    (* range
      (/ (sp-time-random-discrete state cudist cudist-size) (convert-type cudist-size sp-sample-t)))))

(define (sp-times-swap a i1 i2) (void sp-time-t* ssize-t ssize-t)
  (declare temp sp-time-t)
  (set temp (array-get a i1) (array-get a i1) (array-get a i2) (array-get a i2) temp))

(define (sp-times-sequence-increment a size set-size) (void sp-time-t* sp-time-t sp-time-t)
  "increment array as if its elements were digits of a written number of base set-size, lower endian"
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1))
    (if (< (array-get a i) (- set-size 1)) (begin (set+ (array-get a i) 1) break)
      (set (array-get a i) 0))))

(define (sp-times-compositions sum out out-size out-sizes)
  (status-t sp-time-t sp-time-t*** sp-time-t* sp-time-t**)
  "return all permutations of integers that sum to \"sum\".
   Kelleher 2006, 'Encoding partitions as ascending compositions'"
  status-declare
  (declare
    a sp-time-t*
    b sp-time-t*
    i sp-time-t
    k sp-time-t
    o-size sp-time-t
    o-sizes sp-time-t*
    o sp-time-t**
    o-used sp-time-t
    sigma sp-time-t
    b-size sp-time-t
    x sp-time-t
    y sp-time-t)
  (set sigma 1 o 0 o-used 0 a 0 k 1 o-size (sp-time-expt 2 (- sum 1)))
  (status-require (sph-helper-calloc (* (+ 1 sum) (sizeof sp-time-t)) &a))
  (status-require (sph-helper-calloc (* o-size (sizeof sp-time-t*)) &o))
  (status-require (sph-helper-calloc (* o-size (sizeof sp-time-t)) &o-sizes))
  (set (array-get a 1) sum)
  (while (not (= 0 k))
    (set x (+ (array-get a (- k 1)) 1) y (- (array-get a k) 1) k (- k 1))
    (while (<= sigma y) (set (array-get a k) x x sigma y (- y x) k (+ k 1)))
    (set (array-get a k) (+ x y) b-size (+ k 1))
    (status-require (sph-helper-malloc (* b-size (sizeof sp-time-t)) &b))
    (memcpy b a (* b-size (sizeof sp-time-t)))
    (set (array-get o o-used) b (array-get o-sizes o-used) b-size o-used (+ o-used 1)))
  (set *out o *out-size o-used *out-sizes o-sizes)
  (label exit
    (if a (free a))
    (if status-is-failure
      (if o
        (begin
          (for ((set i 0) (< i o-used) (set+ i 1)) (free (array-get o i)))
          (free o)
          (free o-sizes))))
    status-return))

(define (sp-times-permutations size set set-size out out-size)
  (status-t sp-time-t sp-time-t* sp-time-t sp-time-t*** sp-time-t*)
  "return all permutations of length \"size\" for \"set\".
   allocates all needed memory in \"out\" and passes ownership to caller.
   https://en.wikipedia.org/wiki/Heap's_algorithm"
  (declare
    a sp-time-t*
    b sp-time-t*
    i sp-time-t
    o-size sp-time-t
    o sp-time-t**
    o-used sp-time-t
    s sp-time-t*)
  status-declare
  (set a 0 b 0 s 0 o 0 i 0 o-used 0 o-size (sp-time-factorial size))
  (status-require (sph-helper-malloc (* size (sizeof sp-time-t)) &a))
  (status-require (sph-helper-calloc (* size (sizeof sp-time-t)) &s))
  (status-require (sph-helper-calloc (* o-size (sizeof sp-time-t*)) &o))
  (sc-comment "ensure that new b are always added to o")
  (status-require (sph-helper-malloc (* size (sizeof sp-time-t)) &b))
  (set (array-get o o-used) b o-used (+ o-used 1))
  (memcpy a set (* size (sizeof sp-time-t)))
  (memcpy b a (* size (sizeof sp-time-t)))
  (while (< i size)
    (if (< (array-get s i) i)
      (begin
        (if (bit-and i 1) (sp-times-swap a (array-get s i) i) (sp-times-swap a 0 i))
        (set+ (array-get s i) 1)
        (set i 0)
        (status-require (sph-helper-malloc (* size (sizeof sp-time-t)) &b))
        (memcpy b a (* size (sizeof sp-time-t)))
        (set (array-get o o-used) b o-used (+ o-used 1)))
      (set (array-get s i) 0 i (+ i 1))))
  (set *out o *out-size o-used)
  (label exit
    (if s (free s))
    (if a (free a))
    (if status-is-failure
      (if o (begin (for ((set i 0) (< i o-used) (set+ i 1)) (free (array-get o i))) (free o))))
    status-return))

(define (sp-samples-divisions start n count out)
  (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (declare i sp-time-t)
  (for ((set i 0) (< i count) (set+ i 1)) (set (array-get out i) start) (set/ start n)))

(define (sp-samples-scale-y a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  "adjust all values, keeping relative sizes, so that the maximum value is n.
   a/out can be the same pointer"
  (define max sp-sample-t (sp-samples-absolute-max a size))
  (sp-samples-multiply-1 a size (/ n max) a))

(define (sp-times-scale-y a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "adjust all values, keeping relative sizes, so that the maximum value is n.
   a/out can be the same pointer"
  (define max sp-time-t (sp-times-absolute-max a size))
  (sp-times-multiply-1 a size (/ n max) a))

(define (sp-samples-scale-sum a size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  "adjust all values, keeping relative sizes, so that the sum is n.
   a/out can be the same pointer"
  (define sum sp-sample-t (sp-samples-sum a size))
  (sp-samples-multiply-1 a size (/ n sum) a))

(define (sp-times-sum a size) (sp-time-t sp-time-t* sp-time-t)
  (define sum sp-time-t 0)
  (for-each-index i size (set+ sum (array-get a i)))
  (return sum))

(define (sp-times-scale-sum a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "adjust all values, keeping relative sizes, so that the sum is n.
   a/out can be the same pointer"
  (declare factor sp-sample-t)
  (set factor (/ (convert-type n sp-sample-t) (sp-times-sum a size)))
  (for-each-index i size
    (set (array-get out i) (sp-cheap-round-positive (* (array-get out i) factor)))))

(define (sp-times-multiplications start factor count out)
  (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  "write count cumulative multiplications with factor from start to out"
  (declare i sp-time-t)
  (for ((set i 0) (< i count) (set+ i 1)) (set (array-get out i) start) (set* start factor)))

(define (sp-times-additions start summand count out)
  (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  "write count cumulative additions with summand from start to out.
   with summand, only the nth additions are written.
   use case: generating harmonic frequency values"
  (declare i sp-time-t)
  (for ((set i 0) (< i count) (set+ i 1)) (set (array-get out i) start) (set+ start summand)))

(define (sp-samples-additions start summand count out)
  (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  "write count cumulative additions with summand from start to out"
  (declare i sp-time-t)
  (for ((set i 0) (< i count) (set+ i 1)) (set (array-get out i) start) (set+ start summand)))

(define (sp-times-select a indices size out) (void sp-time-t* sp-time-t* sp-time-t sp-time-t*)
  "take values at indices and write to out.
   a/out can be the same pointer"
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1))
    (set (array-get out i) (array-get a (array-get indices i)))))

(define (sp-times-bits->times a size out) (void sp-time-t* sp-time-t sp-time-t*)
  "store the bits of a as uint8 in b. size is the number of bits to take.
   a/out can be the same pointer"
  (declare i sp-time-t a-i sp-time-t bits-i sp-time-t mask sp-time-t)
  (set i 0 a-i 0)
  (while (< i size)
    (set mask 1 bits-i 0)
    (while (and (< bits-i (* (sizeof sp-time-t) 8)) (< i size))
      (set
        (array-get out i) (if* (bit-and (array-get a a-i) mask) 1 0)
        mask (bit-shift-left mask 1))
      (set+ i 1 bits-i 1))
    (set+ a-i 1)))

(define (sp-times-shuffle state a size) (void sp-random-state-t* sp-time-t* sp-time-t)
  "modern yates shuffle.
   https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm"
  (declare i sp-time-t j sp-time-t t sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1))
    (set
      j (+ i (sp-time-random-bounded state (- size i)))
      t (array-get a i)
      (array-get a i) (array-get a j)
      (array-get a j) t)))

(define (sp-times-random-binary state size out) (status-t sp-random-state-t* sp-time-t sp-time-t*)
  "write to out values that are randomly either 1 or 0"
  (declare random-size sp-time-t temp sp-time-t*)
  status-declare
  (set random-size
    (if* (< size (* (sizeof sp-time-t) 8)) 1 (+ (/ size (* (sizeof sp-time-t) 8)) 1)))
  (status-require (sp-times-new random-size &temp))
  (sp-times-random state random-size temp)
  (sp-times-bits->times temp size out)
  (free temp)
  (label exit status-return))

(define (sp-times-gt-indices a size n out out-size)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t* sp-time-t*)
  "write to out indices of a that are greater than n
   a/out can be the same pointer"
  (declare i sp-time-t i2 sp-time-t)
  (for ((set i 0 i2 0) (< i size) (set+ i 1))
    (if (< n (array-get a i)) (set (array-get out i2) i i2 (+ i2 1))))
  (set *out-size i2))

(define (sp-times-select-random state a size out out-size)
  (void sp-random-state-t* sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  "a/out can not be the same pointer.
   out-size will be less or equal than size.
   memory is allocated and owned by caller"
  status-declare
  (sp-times-random-binary state size out)
  (sp-times-gt-indices out size 0 out out-size)
  (sp-times-select a out *out-size out))

(define (sp-times-constant a size value out) (status-t sp-time-t sp-time-t sp-time-t sp-time-t**)
  "allocate memory for out and set all elements to value"
  (declare temp sp-time-t*)
  status-declare
  (status-require (sp-times-new size &temp))
  (sp-times-set-1 temp size value temp)
  (set *out temp)
  (label exit status-return))

(define (sp-times-scale a a-size factor out) (status-t sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "y is scaled by (y * factor), x is scaled by linear interpolation between elements of a.
   out size will be (a-size - 1) * factor"
  status-declare
  (declare i sp-time-t i2 sp-time-t aa sp-time-t*)
  (status-require (sp-times-new a-size &aa))
  (sp-times-multiply-1 a a-size factor aa)
  (for ((set i 1) (< i a-size) (set+ i 1))
    (for ((set i2 0) (< i2 factor) (set+ i2 1))
      (set (array-get out (+ (* factor (- i 1)) i2))
        (sp-time-interpolate-linear (array-get aa (- i 1)) (array-get aa i)
          (/ i2 (convert-type factor sp-sample-t))))))
  (free aa)
  (label exit status-return))

(define (sp-times-shuffle-swap a i1 i2) (void void* size-t size-t)
  "a array value swap function that can be used with sp-shuffle"
  (declare b sp-time-t*)
  (set
    b (array-get (convert-type a sp-time-t**) i1)
    (array-get (convert-type a sp-time-t**) i1) (array-get (convert-type a sp-time-t**) i2)
    (array-get (convert-type a sp-time-t**) i2) b))

(define (sp-times-array-free a size) (void sp-time-t** sp-time-t)
  "free every element array and the container array"
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1)) (free (array-get a i)))
  (free a))

(define (sp-samples-array-free a size) (void sp-sample-t** sp-time-t)
  "free every element array and the container array"
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1)) (free (array-get a i)))
  (free a))

(define (sp-times-contains a size b) (uint8-t sp-time-t* sp-time-t sp-time-t)
  "true if array a contains element b"
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1)) (if (= b (array-get a i)) (return #t)))
  (return #f))

(define (sp-times-random-discrete-unique state cudist cudist-size size out)
  (void sp-random-state-t* sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "create size number of discrete random numbers corresponding to the distribution given by cudist
   without duplicates. cudist-size must be equal to a-size.
   a/out should not be the same pointer. out is managed by the caller.
   size is the requested size of generated output values and should be smaller than a-size.
   size must not be greater than the maximum possible count of unique discrete random values
   (non-null values in the probability distribution)"
  status-declare
  (declare a sp-time-t remaining sp-time-t)
  (set remaining (sp-min size cudist-size))
  (while remaining
    (set a (sp-time-random-discrete state cudist cudist-size))
    (if (sp-times-contains out (- size remaining) a) continue)
    (set (array-get out (- size remaining)) a)
    (set- remaining 1)))

(define (sp-times-sequences base digits size out) (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  "out-size must be at least digits * size or base ** digits.
   starts from out + 0. first generated element will be in out + digits.
   out size will contain the sequences appended"
  (declare i sp-time-t)
  (for ((set i digits) (< i (* digits size)) (set+ i digits))
    (memcpy (+ out i) (+ out (- i digits)) (* digits (sizeof sp-time-t)))
    (sp-times-sequence-increment (+ out i) digits base)))

(define (sp-times-range start end out) (void sp-time-t sp-time-t sp-time-t*)
  "write into out values from start (inclusively) to end (exclusively)"
  (declare i sp-time-t)
  (for ((set i start) (< i end) (set+ i 1)) (set (array-get out (- i start)) i)))

(define (sp-time-round-to-multiple a base) (sp-time-t sp-time-t sp-time-t)
  "round to the next integer multiple of base "
  (return
    (if* (= 0 a) base (sp-cheap-round-positive (* (/ a (convert-type base sp-sample-t)) base)))))

(define (sp-times-deduplicate a size out out-size)
  (status-t sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  "writes the unique elements of $a to $out.
   $out is lend by the owner. size of $out should be equal or greater than size of $a.
   $out-size will be set to the number of unique elements"
  status-declare
  (declare unique sp-time-set-t unique-count sp-time-t)
  (set unique.size 0)
  (if (sp-time-set-new size &unique) sp-memory-error)
  (set unique-count 0)
  (for-each-index i size
    (if (not (sp-time-set-get unique (array-get a i)))
      (begin
        (if (not (sp-time-set-add unique (array-get a i))) sp-memory-error)
        (set (array-get out unique-count) (array-get a i))
        (set+ unique-count 1))))
  (set *out-size unique-count)
  (label exit (if unique.size (sp-time-set-free unique)) status-return))

(define (sp-times-counted-sequences-sort-swap a b c) (void void* ssize-t ssize-t)
  (declare d sp-times-counted-sequences-t)
  (set
    d (array-get (convert-type a sp-times-counted-sequences-t*) b)
    (array-get (convert-type a sp-times-counted-sequences-t*) b)
    (array-get (convert-type a sp-times-counted-sequences-t*) c)
    (array-get (convert-type a sp-times-counted-sequences-t*) c) d))

(define (sp-times-counted-sequences-sort-less a b c) (uint8-t void* ssize-t ssize-t)
  (return
    (< (struct-get (array-get (convert-type a sp-times-counted-sequences-t*) b) count)
      (struct-get (array-get (convert-type a sp-times-counted-sequences-t*) c) count))))

(define (sp-times-counted-sequences-sort-greater a b c) (uint8-t void* ssize-t ssize-t)
  (return
    (> (struct-get (array-get (convert-type a sp-times-counted-sequences-t*) b) count)
      (struct-get (array-get (convert-type a sp-times-counted-sequences-t*) c) count))))

(define (sp-times-counted-sequences-add a size width out)
  (void sp-time-t* sp-time-t sp-time-t sp-sequence-hashtable-t)
  "associate in hash table $out overlapping sub-sequences of $width with their count in $a.
   memory for $out is lend and should be allocated with sp_sequence_hashtable_new(size - (width - 1), &out)"
  (declare key sp-sequence-set-key-t value sp-time-t*)
  (set key.size width)
  (for-each-index i (- size (- width 1))
    (set key.data (convert-type (+ i a) uint8-t*) value (sp-sequence-hashtable-get out key))
    (sc-comment "full-hashtable-error is ignored")
    (if value (set+ *value 1) (sp-sequence-hashtable-set out key 1))))

(define (sp-times-counted-sequences-count a width b)
  (sp-time-t sp-time-t* sp-time-t sp-sequence-hashtable-t)
  "return the current count of sequence a in hash b"
  (declare key sp-sequence-set-key-t value sp-time-t*)
  (set key.size width key.data (convert-type a uint8-t*) value (sp-sequence-hashtable-get b key))
  (return (if* value *value 0)))

(define (sp-times-counted-sequences-values known min out out-size)
  (void sp-sequence-hashtable-t sp-time-t sp-times-counted-sequences-t* sp-time-t*)
  "extract counts from a counted-sequences-hash and return as an array of structs"
  (declare count sp-time-t)
  (set count 0)
  (for-each-index i known.size
    (if (and (array-get known.flags i) (< min (array-get known.values i)))
      (begin
        (set
          (struct-get (array-get out count) count) (array-get known.values i)
          (struct-get (array-get out count) sequence)
          (convert-type (struct-get (array-get known.keys i) data) sp-time-t*))
        (set+ count 1))))
  (set *out-size count))

(define (sp-times-remove in size index count out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t*)
  "remove count subsequent elements at index from in and write the result to out"
  (cond ((= 0 index) (memcpy out (+ in count) (* (- size count) (sizeof sp-time-t))))
    ((= (- size 1) index) (memcpy out in (* (- size count) (sizeof sp-time-t))))
    (else (memcpy out in (* index (sizeof sp-time-t)))
      (memcpy (+ out index) (+ in index count) (* (- size index count) (sizeof sp-time-t))))))

(define (sp-times-insert-space in size index count out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t*)
  "insert an new area of unset elements before index while copying from in to out"
  (if (= 0 index) (memcpy (+ out count) in (* size (sizeof sp-time-t)))
    (begin
      (memcpy out in (* index (sizeof sp-time-t)))
      (memcpy (+ out index count) (+ in index) (* (- size index) (sizeof sp-time-t))))))

(define (sp-times-subdivide-difference a size index count out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t*)
  "insert equally spaced values between the two values at $index and $index + 1.
   a:(4 16) index:0 count:3 -> out:(4 7 10 13 16)
   the second value must be greater than the first.
   $index must not be the last index.
   $out size must be at least $size + $count"
  (declare i sp-time-t interval sp-time-t value sp-time-t)
  (sp-times-insert-space a size (+ index 1) count out)
  (set interval (- (array-get a (+ index 1)) (array-get a index)) value (/ interval (+ count 1)))
  (for ((set i (+ index 1)) (< i (+ index (+ count 1))) (set+ i 1))
    (set (array-get out i) (+ (array-get out (- i 1)) value))))

(define (sp-times-blend a b fraction size out)
  (void sp-time-t* sp-time-t* sp-sample-t sp-time-t sp-time-t*)
  "interpolate values between $a and $b with interpolation distance fraction 0..1"
  (for-each-index i size
    (set (array-get out i)
      (sp-cheap-round-positive
        (sp-time-interpolate-linear (array-get a i) (array-get b i) fraction)))))

(define (sp-times-mask a b coefficients size out)
  (void sp-time-t* sp-time-t* sp-sample-t* sp-time-t sp-time-t*)
  "interpolate values pointwise between $a and $b with interpolation distance 0..1 from $coefficients"
  (for-each-index i size
    (set (array-get out i)
      (sp-cheap-round-positive
        (sp-time-interpolate-linear (array-get a i) (array-get b i) (array-get coefficients i))))))

(define (sp-samples-blend a b fraction size out)
  (void sp-sample-t* sp-sample-t* sp-sample-t sp-time-t sp-sample-t*)
  "interpolate values pointwise between $a and $b with fraction as a fixed interpolation distance 0..1"
  (for-each-index i size
    (set (array-get out i) (sp-sample-interpolate-linear (array-get a i) (array-get b i) fraction))))

(define (sp-times-limit a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "set all values greater than n in array to n"
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1))
    (set (array-get out i) (if* (< n (array-get a i)) n (array-get a i)))))

(define (sp-samples-limit-abs in size n out) (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  "set all values greater than absolute n in array to n, keeping sign.
   in and out can be the same address"
  (declare i sp-time-t v sp-sample-t)
  (for ((set i 0) (< i size) (set+ i 1))
    (set v (array-get in i))
    (if (> 0 v) (set (array-get out i) (if* (> (* -1 n) v) (* -1 n) v))
      (set (array-get out i) (if* (< n v) n v)))))

(define (sp-times-extract-in-range a size min max out out-size)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t* sp-time-t*)
  "extract values between min/max inclusive and write to out"
  (set *out-size 0)
  (for ((define i sp-time-t 0) (< i size) (set+ i 1))
    (if (or (<= (array-get a i) max) (>= (array-get a i) min))
      (begin (set (array-get out i) (array-get a i)) (set+ *out-size 1)))))

(define (sp-times-make-seamless-right a a-size b b-size out)
  (void sp-time-t* sp-time-t sp-time-t* sp-time-t sp-time-t*)
  "untested. interpolate (b-size / 2) elements at the end of $a progressively more and more towards their mirrored correspondents in $b.
   example: (1 2 3) (4 5 6), interpolates 1 to 6 at 1/6 distance, then 2 to 6 at 2/6 distance, then 3 to 4 at 3/6 istance (3.5)"
  (declare start sp-time-t size sp-time-t i sp-time-t)
  (set size (sp-min a-size (/ b-size 2)) start (- a-size size))
  (for ((set i 0) (< i start) (set+ i 1)) (set (array-get out i) (array-get a i)))
  (for ((set i 0) (< i size) (set+ i 1))
    (set (array-get out (+ i start))
      (sp-time-interpolate-linear (array-get a (+ i start)) (array-get b (- size (+ 1 i)))
        (/ (+ 1 i) size)))))

(define (sp-times-make-seamless-left a a-size b b-size out)
  (void sp-time-t* sp-time-t sp-time-t* sp-time-t sp-time-t*)
  "untested. like sp_times_make_seamless_right but changing the first elements of $b to match the end of $a"
  (declare start sp-time-t size sp-time-t i sp-time-t)
  (set size (sp-min b-size (/ a-size 2)) start (- a-size size))
  (for ((set i 0) (< i size) (set+ i 1))
    (set (array-get out i)
      (sp-time-interpolate-linear (array-get b i) (array-get a (- a-size i)) (/ (+ 1 i) size))))
  (for ((set i (- size 1)) (< i b-size) (set+ i 1)) (set (array-get out i) (array-get b i))))