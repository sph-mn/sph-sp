(sc-comment
  "routines on arrays of sp-time-t or sp-sample-t"
  "sp-time-t subtraction is limited to zero."
  "sp-time-t addition is not limited and values larger than the type lead to overflows")

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

(define (sp-samples-set-unity-gain in in-size out) (void sp-sample-t* sp-time-t sp-sample-t*)
  "adjust amplitude of out to match the one of in"
  (declare
    i sp-time-t
    in-max sp-sample-t
    out-max sp-sample-t
    difference sp-sample-t
    correction sp-sample-t)
  (set in-max (sp-samples-absolute-max in in-size) out-max (sp-samples-absolute-max out in-size))
  (if (or (= 0 in-max) (= 0 out-max)) return)
  (set difference (/ out-max in-max) correction (+ 1 (/ (- 1 difference) difference)))
  (for ((set i 0) (< i in-size) (set i (+ 1 i)))
    (set (array-get out i) (* correction (array-get out i)))))

(pre-define
  (absolute-difference a b) (if* (> a b) (- a b) (- b a))
  (no-underflow-subtract a b) (if* (> a b) (- a b) 0)
  (no-zero-divide a b) (if* (= 0 b) 0 (/ a b))
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
    "functions that work on sp-sample-t* and sp-time-t*"
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
    (for ((set i 0) (< i size) (set+ i 1)) (set (array-get out i) transfer)))
  (sequence-set-equal a b)
  (and (= a.size b.size) (or (and (= 0 a.size) (= 0 b.size)) (not (memcmp a.data b.data a.size)))))

(define (sp-samples-every-equal a size n) (uint8-t sp-sample-t* sp-time-t sp-sample-t)
  (declare i sp-time-t)
  (for ((set i 0) (< i size) (set+ i 1)) (if (not (= n (array-get a i))) (return 0)))
  (return 1))

(define-value-functions sp-times sp-time-t)
(define-value-functions sp-samples sp-sample-t)
(define-array-functions sp-times sp-time-t)
(define-array-functions sp-samples sp-sample-t)
(define-array-mapper sp-times-square sp-time-t (* (array-get a i) (array-get a i)))
(define-array-mapper sp-samples-square sp-sample-t (* (array-get a i) (array-get a i)))
(define-array-combinator sp-times-add sp-time-t (+ (array-get a i) (array-get b i)))
(define-array-combinator sp-samples-add sp-sample-t (+ (array-get a i) (array-get b i)))

(define-array-combinator sp-times-subtract sp-time-t
  (no-underflow-subtract (array-get a i) (array-get b i)))

(define-array-combinator sp-samples-subtract sp-sample-t (- (array-get a i) (array-get b i)))
(define-array-combinator sp-times-multiply sp-time-t (* (array-get a i) (array-get b i)))
(define-array-combinator sp-samples-multiply sp-sample-t (* (array-get a i) (array-get b i)))
(define-array-combinator sp-times-divide sp-time-t (/ (array-get a i) (array-get b i)))
(define-array-combinator sp-samples-divide sp-sample-t (/ (array-get a i) (array-get b i)))
(define-array-combinator-1 sp-times-add-1 sp-time-t (+ (array-get a i) n))
(define-array-combinator-1 sp-samples-add-1 sp-sample-t (+ (array-get a i) n))
(define-array-combinator-1 sp-times-subtract-1 sp-time-t (no-underflow-subtract (array-get a i) n))
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

(declare sequence-set-key-t (type (struct (size sp-time-t) (data uint8-t*))))
(define sequence-set-null sequence-set-key-t (struct-literal 0 0))

(define (sequence-set-hash a memory-size) (uint64-t sequence-set-key-t sp-time-t)
  (modulo (sp-u64-from-array a.data a.size) memory-size))

(sph-set-declare-type-nonull sequence-set sequence-set-key-t
  sequence-set-hash sequence-set-equal sequence-set-null 2)

(sc-comment "samples")

(define (sp-samples-copy a size out) (status-t sp-sample-t* sp-time-t sp-sample-t**)
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

(define (sp-times-copy a size out) (status-t sp-time-t sp-time-t sp-time-t**)
  status-declare
  (declare temp sp-time-t*)
  (status-require (sp-times-new size &temp))
  (set *out temp)
  (label exit status-return))

(define (sp-times-differences a size out) (void sp-time-t* sp-time-t sp-time-t*)
  "a.size must be > 1. a.size minus 1 elements will be written to out"
  (declare i sp-time-t)
  (for ((set i 1) (< i size) (set+ i 1))
    (set (array-get out (- i 1)) (absolute-difference (array-get a i) (array-get a (- i 1))))))

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

(define (sp-times-sequence-increment-le a size set-size) (void sp-time-t* sp-time-t sp-time-t)
  "increment array as if its elements were digits of a written number of base set-size"
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

(define (sp-samples-scale-y-sum a size n out)
  (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  "adjust all values, keeping relative sizes, so that the sum is n.
   a/out can be the same pointer"
  (define sum sp-sample-t (sp-samples-sum a size))
  (sp-samples-multiply-1 a size (/ n sum) a))

(define (sp-times-multiplications start factor count out)
  (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  "write count cumulative multiplications with factor from start to out"
  (declare i sp-time-t)
  (for ((set i 0) (< i count) (set+ i 1)) (set (array-get out i) start) (set* start factor)))

(define (sp-times-additions start summand count out)
  (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  "write count cumulative additions with summand from start to out"
  (declare i sp-time-t)
  (for ((set i 0) (< i count) (set+ i 1)) (set (array-get out i) start) (set+ start summand)))

(define (sp-samples-additions start summand count out)
  (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  "write count cumulative additions with summand from start to out"
  (declare i sp-time-t)
  (for ((set i 0) (< i count) (set+ i 1)) (set (array-get out i) start) (set+ start summand)))

(define (sp-times-extract-at-indices a indices size out)
  (void sp-time-t* sp-time-t* sp-time-t sp-time-t*)
  "take values at indices and write in order to out.
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

(define (sp-times-extract-random state a size out out-size)
  (void sp-random-state-t* sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  "a/out can not be the same pointer.
   out-size will be less or equal than size.
   memory is allocated and owned by caller"
  status-declare
  (sp-times-random-binary state size out)
  (sp-times-gt-indices out size 0 out out-size)
  (sp-times-extract-at-indices a out *out-size out))

(define (sp-times-constant a size value out) (status-t sp-time-t sp-time-t sp-time-t sp-time-t**)
  "allocate memory for out and set all elements to value"
  (declare temp sp-time-t*)
  status-declare
  (status-require (sp-times-new size &temp))
  (sp-times-set-1 temp size value temp)
  (set *out temp)
  (label exit status-return))

(define (sp-times-scale a a-size factor out) (status-t sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "expand by factor. y is scaled by (y * factor), x is scaled by linear interpolation between elements of a.
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
  "a array value swap function usable with sp-shuffle"
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
  (declare i sp-time-t a sp-time-t remaining sp-time-t)
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
    (sp-times-sequence-increment-le (+ out i) digits base)))

(define (sp-times-range start end out) (void sp-time-t sp-time-t sp-time-t*)
  "write into out values from start (inclusively) to end (exclusively)"
  (declare i sp-time-t)
  (for ((set i start) (< i end) (set+ i 1)) (set (array-get out (- i start)) i)))

(define (sp-time-round-to-multiple a base) (sp-time-t sp-time-t sp-time-t)
  "round to the next integer multiple of base "
  (return
    (if* (= 0 a) base (sp-cheap-round-positive (* (/ a (convert-type base sp-sample-t)) base)))))

(define (sp-times-limit a size n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  (declare i sp-time-t)
  "set all values greater than n in array to n"
  (for ((set i 0) (< i size) (set+ i 1))
    (set (array-get out i) (if* (< n (array-get a i)) n (array-get a i)))))