(pre-include "sph-sp/arrays-template.c")
(sc-comment "times")

(sc-no-semicolon
  (arrays-template sp-time-t time times sp-inline-no-underflow-subtract sp-inline-abs))

(define (sp-time-sum in size) (sp-time-t sp-time-t* sp-time-t)
  (define sum sp-time-t 0)
  (sp-for-each-index i size (set+ sum (array-get in i)))
  (return sum))

(define (sp-times-sum a size) (sp-time-t sp-time-t* sp-time-t)
  (define sum sp-time-t 0)
  (sp-for-each-index i size (set+ sum (array-get a i)))
  (return sum))

(define (sp-times-display in count) (void sp-time-t* sp-size-t)
  "display a time array in one line"
  (printf sp-time-printf-format (array-get in 0))
  (for ((define i sp-size-t 1) (< i count) (set+ i 1))
    (printf (pre-concat-string " " sp-time-printf-format) (array-get in i)))
  (printf "\n"))

(define (sp-times-random-discrete cudist cudist-size count out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "generate random integers in the range 0..(cudist-size - 1)
   with probability distribution given via cudist, the cumulative sums of the distribution.
   generates a random number in rango 0..cudist-sum"
  (declare deviate sp-time-t sum sp-time-t)
  (set sum (array-get cudist (- cudist-size 1)))
  (sp-for-each-index i count
    (set deviate (sp-time-random-bounded sum))
    (sp-for-each-index i1 cudist-size
      (if (< deviate (array-get cudist i1)) (begin (set (array-get out i) i1) break)))))

(define (sp-time-random-discrete cudist cudist-size) (sp-time-t sp-time-t* sp-time-t)
  (define deviate sp-time-t (sp-time-random-bounded (array-get cudist (- cudist-size 1))))
  (sp-for-each-index i cudist-size (if (< deviate (array-get cudist i)) (return i)))
  (return cudist-size))

(define (sp-time-random-discrete-bounded cudist cudist-size range)
  (sp-time-t sp-time-t* sp-time-t sp-time-t)
  "get a random number in range with a custom probability distribution given by cudist,
   the cumulative sums of the distribution. the resulting number resolution is proportional to cudist-size"
  (sc-comment "cudist-size minus one because range end is exclusive")
  (return
    (sp-cheap-round-positive
      (* range
        (/ (sp-time-random-discrete cudist cudist-size)
          (convert-type (- cudist-size 1) sp-sample-t))))))

(define (sp-times-permutations size set set-size out out-size)
  (status-t sp-time-t sp-time-t* sp-time-t sp-time-t*** sp-time-t*)
  "return all permutations of length \"size\" for \"set\".
   allocates all needed memory in \"out\" and passes ownership to caller.
   https://en.wikipedia.org/wiki/Heap's_algorithm"
  status-declare
  (declare
    a sp-time-t*
    b sp-time-t*
    i sp-size-t
    temp-out-size sp-time-t
    temp-out sp-time-t**
    temp-out-used-size sp-time-t
    s sp-time-t*)
  (set a 0 b 0 s 0 temp-out 0 i 0 temp-out-size (sp-time-factorial size) temp-out-used-size 0)
  (srq (sp-times-new size &a))
  (srq (sp-times-new size &s))
  (srq (sp-calloc-type temp-out-size sp-time-t* &temp-out))
  (srq (sp-times-new size &b))
  (set (array-get temp-out 0) b temp-out-used-size 1)
  (sp-times-copy a size set)
  (sp-times-copy b size a)
  (while (< i size)
    (if (< (array-get s i) i)
      (begin
        (if (bit-and i 1) (sp-times-swap a (array-get s i) i) (sp-times-swap a 0 i))
        (set+ (array-get s i) 1)
        (set i 0)
        (status-require (sph-malloc (* size (sizeof sp-time-t)) &b))
        (memcpy b a (* size (sizeof sp-time-t)))
        (set (array-get temp-out temp-out-used-size) b)
        (set+ temp-out-used-size 1))
      (begin (set (array-get s i) 0) (set+ i 1))))
  (set *out temp-out *out-size temp-out-used-size)
  (label exit
    (if s (free s))
    (if a (free a))
    (if status-is-failure
      (if temp-out
        (begin
          (sp-for-each-index i temp-out-used-size (free (array-get temp-out i)))
          (free temp-out))))
    status-return))

(define (sp-times-sequence-increment in size set-size) (void sp-time-t* sp-size-t sp-size-t)
  "increment array as if its elements were digits of a written number of base set-size, lower endian"
  (sp-for-each-index i size
    (if (< (array-get in i) (- set-size 1)) (begin (set+ (array-get in i) 1) break)
      (set (array-get in i) 0))))

(define (sp-times-compositions sum out out-size out-sizes)
  (status-t sp-time-t sp-time-t*** sp-time-t* sp-time-t**)
  "return all permutations of integers that sum to \"sum\".
   Kelleher 2006, 'Encoding partitions as ascending compositions'"
  status-declare
  (declare
    a sp-time-t*
    b sp-time-t*
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
  (status-require (sph-calloc (* (+ 1 sum) (sizeof sp-time-t)) &a))
  (status-require (sph-calloc (* o-size (sizeof sp-time-t*)) &o))
  (status-require (sph-calloc (* o-size (sizeof sp-time-t)) &o-sizes))
  (set (array-get a 1) sum)
  (while (not (= 0 k))
    (set x (+ (array-get a (- k 1)) 1) y (- (array-get a k) 1) k (- k 1))
    (while (<= sigma y) (set (array-get a k) x x sigma y (- y x) k (+ k 1)))
    (set (array-get a k) (+ x y) b-size (+ k 1))
    (status-require (sph-malloc (* b-size (sizeof sp-time-t)) &b))
    (memcpy b a (* b-size (sizeof sp-time-t)))
    (set (array-get o o-used) b (array-get o-sizes o-used) b-size o-used (+ o-used 1)))
  (set *out o *out-size o-used *out-sizes o-sizes)
  (label exit
    (if a (free a))
    (if status-is-failure
      (if o (begin (sp-for-each-index i o-used (free (array-get o i))) (free o) (free o-sizes))))
    status-return))

(define (sp-times-select in indices count out) (void sp-time-t* sp-time-t* sp-time-t sp-time-t*)
  "take values at indices and write to out.
   a/out can be the same pointer"
  (sp-for-each-index i count (set (array-get out i) (array-get in (array-get indices i)))))

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

(define (sp-times-gt-indices a size n out out-size)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t* sp-time-t*)
  "write to out indices of a that are greater than n
   a/out can be the same pointer"
  (define i2 sp-time-t 0)
  (sp-for-each-index i size
    (if (< n (array-get a i)) (begin (set (array-get out i2) i) (set+ i2 1))))
  (set *out-size i2))

(define (sp-times-select-random a size out out-size)
  (void sp-time-t* sp-time-t sp-time-t* sp-time-t*)
  "a/out can not be the same pointer.
   out-size will be less or equal than size.
   memory is allocated and owned by caller"
  (sp-times-random-binary size out)
  (sp-times-gt-indices out size 0 out out-size)
  (sp-times-select a out *out-size out))

(define (sp-times-constant count value out) (status-t sp-size-t sp-time-t sp-time-t**)
  "allocate memory for out and set all elements to value"
  status-declare
  (declare temp sp-time-t*)
  (status-require (sp-times-new count &temp))
  (sp-times-set temp count value)
  (set *out temp)
  (label exit status-return))

(define (sp-times-scale in count factor out) (status-t sp-time-t* sp-size-t sp-time-t sp-time-t*)
  "scales in both dimensions, x and y.
   y is scaled by (y * factor), x is scaled by linear interpolation between elements of in-out.
   out size will be (count - 1) * factor"
  status-declare
  (declare temp sp-time-t*)
  (status-require (sp-times-new count &temp))
  (sp-times-copy in count temp)
  (sp-times-multiply temp count factor)
  (for ((define i sp-size-t 1) (< i count) (set+ i 1))
    (sp-for-each-index i2 factor
      (set (array-get out (+ (* factor (- i 1)) i2))
        (sp-time-interpolate-linear (array-get temp (- i 1)) (array-get temp i)
          (/ i2 (convert-type factor sp-sample-t))))))
  (free temp)
  (label exit status-return))

(define (sp-times-shuffle-swap a i1 i2) (void void* sp-size-t sp-size-t)
  "a array value swap function that can be used with sp-shuffle"
  (declare b sp-time-t*)
  (set
    b (array-get (convert-type a sp-time-t**) i1)
    (array-get (convert-type a sp-time-t**) i1) (array-get (convert-type a sp-time-t**) i2)
    (array-get (convert-type a sp-time-t**) i2) b))

(define (sp-times-scale-sum in count sum out) (void sp-time-t* sp-size-t sp-time-t sp-time-t*)
  "adjust all values, keeping relative sizes, so that the sum is n.
   a/out can be the same pointer"
  (define factor sp-sample-t (/ (convert-type sum sp-sample-t) (sp-times-sum in count)))
  (sp-for-each-index i count
    (set (array-get out i) (sp-cheap-round-positive (* (array-get out i) factor)))))

(define (sp-times-contains in count value) (uint8-t sp-time-t* sp-size-t sp-time-t)
  "true if array in contains element value"
  (sp-for-each-index i count (if (= value (array-get in i)) (return #t)))
  (return #f))

(define (sp-times-sequences base digits set-size out)
  (void sp-time-t sp-time-t sp-time-t sp-time-t*)
  "append a series of incremented sequences to out, starting with and not modifying the first element in out.
   out size must be at least digits * set-size or base ** digits.
   starts with sequence at out + 0. first generated element will be in out + digits"
  (declare i sp-time-t)
  (for ((set i digits) (< i (* digits set-size)) (set+ i digits))
    (memcpy (+ out i) (+ out (- i digits)) (* digits (sizeof sp-time-t)))
    (sp-times-sequence-increment (+ out i) digits base)))

(define (sp-times-blend a b fraction size out)
  (void sp-time-t* sp-time-t* sp-sample-t sp-time-t sp-time-t*)
  "interpolate values between $a and $b with interpolation distance fraction 0..1"
  (sp-for-each-index i size
    (set (array-get out i)
      (sp-cheap-round-positive
        (sp-time-interpolate-linear (array-get a i) (array-get b i) fraction)))))

(define (sp-times-mask a b coefficients size out)
  (void sp-time-t* sp-time-t* sp-sample-t* sp-time-t sp-time-t*)
  "interpolate values pointwise between $a and $b with interpolation distance 0..1 from $coefficients"
  (sp-for-each-index i size
    (set (array-get out i)
      (sp-cheap-round-positive
        (sp-time-interpolate-linear (array-get a i) (array-get b i) (array-get coefficients i))))))

(define (sp-times-extract-in-range a size min max out out-size)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t* sp-time-t*)
  "extract values between min/max inclusive and write to out"
  (set *out-size 0)
  (sp-for-each-index i size
    (if (or (<= (array-get a i) max) (>= (array-get a i) min))
      (begin (set (array-get out i) (array-get a i)) (set+ *out-size 1)))))

(define (sp-times-make-seamless-right a a-count b b-count out)
  (void sp-time-t* sp-time-t sp-time-t* sp-time-t sp-time-t*)
  "untested. interpolate (b-count / 2) elements at the end of $a progressively more and more towards their mirrored correspondents in $b.
   example: (1 2 3) (4 5 6), interpolates 1 to 6 at 1/6 distance, then 2 to 6 at 2/6 distance, then 3 to 4 at 3/6 istance (3.5)"
  (declare start sp-time-t count sp-size-t)
  (set count (sp-inline-min a-count (/ b-count 2)) start (- a-count count))
  (sp-for-each-index i start (set (array-get out i) (array-get a i)))
  (sp-for-each-index i count
    (set (array-get out (+ i start))
      (sp-time-interpolate-linear (array-get a (+ i start)) (array-get b (- count (+ 1 i)))
        (/ (+ 1 i) count)))))

(define (sp-times-make-seamless-left a a-count b b-count out)
  (void sp-time-t* sp-time-t sp-time-t* sp-time-t sp-time-t*)
  "untested. like sp_times_make_seamless_right but changing the first elements of $b to match the end of $a"
  (declare count sp-time-t i sp-time-t)
  (set count (sp-inline-min b-count (/ a-count 2)))
  (sp-for-each-index i count
    (set (array-get out i)
      (sp-time-interpolate-linear (array-get b i) (array-get a (- a-count i)) (/ (+ 1 i) count))))
  (for ((set i (- count 1)) (< i b-count) (set+ i 1)) (set (array-get out i) (array-get b i))))

(define (sp-times-limit a count n out) (void sp-time-t* sp-time-t sp-time-t sp-time-t*)
  "set all values greater than n in array to n"
  (sp-for-each-index i count (set (array-get out i) (if* (< n (array-get a i)) n (array-get a i)))))

(define (sp-times-scale-y in count target-y out) (void sp-time-t* sp-size-t sp-time-t sp-time-t*)
  "adjust all values, keeping relative sizes, so that the maximum value is target_y.
   a/out can be the same pointer"
  (define max sp-time-t (sp-times-absolute-max in count))
  (sp-times-multiply in count (/ target-y max)))

(define (sp-times-remove in size index count out)
  (void sp-time-t* sp-time-t sp-time-t sp-time-t sp-time-t*)
  "remove count subsequent elements at index from in and write the result to out"
  (cond
    ((= 0 index) (memcpy out (+ in count) (* (- size count) (sizeof sp-time-t))))
    ((= (- size 1) index) (memcpy out in (* (- size count) (sizeof sp-time-t))))
    (else
      (memcpy out in (* index (sizeof sp-time-t)))
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
  (declare i sp-size-t interval sp-time-t value sp-time-t)
  (sp-times-insert-space a size (+ index 1) count out)
  (set interval (- (array-get a (+ index 1)) (array-get a index)) value (/ interval (+ count 1)))
  (for ((set i (+ index 1)) (< i (+ index (+ count 1))) (set+ i 1))
    (set (array-get out i) (+ (array-get out (- i 1)) value))))

(define (sp-times->samples in count out) (void sp-time-t* sp-size-t sp-sample-t*)
  (sp-for-each-index i count (set (array-get out i) (array-get in i))))

(define (sp-times->samples-replace in count out) (status-t sp-time-t* sp-size-t sp-sample-t**)
  status-declare
  (sp-define-samples-new-srq temp count)
  (sp-for-each-index i count (set (array-get temp i) (array-get in i)))
  (free in)
  (set *out temp)
  (label exit status-return))

(sc-comment "samples")
(sc-no-semicolon (arrays-template sp-sample-t sample samples sp-subtract fabs))

(define (sp-samples-display in count) (void sp-sample-t* sp-size-t)
  "display a sample array in one line"
  (printf sp-sample-printf-format (array-get in 0))
  (for ((define i sp-size-t 1) (< i count) (set+ i 1))
    (printf (pre-concat-string " " sp-sample-printf-format) (array-get in i)))
  (printf "\n"))

(define (sp-samples->times in count out) (void sp-sample-t* sp-size-t sp-time-t*)
  (sp-for-each-index i count (set (array-get out i) (sp-sample->time (array-get in i)))))

(define (sp-samples->units in-out count) (void sp-sample-t* sp-size-t)
  (sp-for-each-index i count (set (array-get in-out i) (/ (+ (array-get in-out i) 1) 2))))

(define (sp-samples->times-replace in count out) (status-t sp-sample-t* sp-size-t sp-time-t**)
  status-declare
  (sp-define-times-srq temp count)
  (sp-for-each-index i count (set (array-get temp i) (sp-sample->time (array-get in i))))
  (free in)
  (set *out temp)
  (label exit status-return))

(define (sp-samples-set-gain in-out count amp) (void sp-sample-t* sp-size-t sp-sample-t)
  (declare in-max sp-sample-t difference sp-sample-t correction sp-sample-t)
  (set in-max (sp-samples-absolute-max in-out count))
  (if (or (= 0 amp) (= 0 in-max)) return)
  (set difference (/ in-max amp) correction (+ 1 (/ (- 1 difference) difference)))
  (sp-for-each-index i count (set* (array-get in-out i) correction)))

(define (sp-samples-set-unity-gain in-out reference count)
  (void sp-sample-t* sp-sample-t* sp-size-t)
  "match the peak of some reference buffer.
   scale amplitude of in-out to match the one of reference.
   no change if any array is zero"
  (sp-samples-set-gain in-out count (sp-samples-absolute-max reference count)))

(define (sp-samples-differences a count out) (void sp-sample-t* sp-time-t sp-sample-t*)
  "write to out the differences between subsequent values of a.
   count must be > 1.
   out length will be count - 1"
  (for ((define i sp-size-t 1) (< i count) (set+ i 1))
    (set (array-get out (- i 1)) (- (array-get a i) (array-get a (- i 1))))))

(define (sp-samples-divisions start divisor count out)
  (void sp-sample-t sp-sample-t sp-time-t sp-sample-t*)
  (sp-for-each-index i count (set (array-get out i) start) (set/ start divisor)))

(define (sp-samples-scale-y in count target-y) (void sp-sample-t* sp-time-t sp-sample-t)
  "adjust all values, keeping relative sizes, so that the maximum value is target_y.
   a/out can be the same pointer"
  (define max sp-sample-t (sp-samples-absolute-max in count))
  (sp-samples-multiply in count (/ count max)))

(define (sp-samples-scale-sum in count target-y out)
  (void sp-sample-t* sp-size-t sp-sample-t sp-sample-t*)
  "adjust all values, keeping relative sizes, so that the sum is n.
   a/out can be the same pointer"
  (define sum sp-sample-t (sp-samples-sum in count))
  (sp-samples-multiply in count (/ target-y sum)))

(define (sp-samples-blend a b fraction size out)
  (void sp-sample-t* sp-sample-t* sp-sample-t sp-time-t sp-sample-t*)
  "interpolate values pointwise between $a and $b with fraction as a fixed interpolation distance 0..1"
  (sp-for-each-index i size
    (set (array-get out i) (sp-sample-interpolate-linear (array-get a i) (array-get b i) fraction))))

(define (sp-samples-limit-abs in count limit out)
  (void sp-sample-t* sp-time-t sp-sample-t sp-sample-t*)
  "set all abs(values) greater than abs(limit) in array to limit, keeping sign.
   in and out can be the same address"
  (declare value sp-sample-t)
  (sp-for-each-index i count
    (set value (array-get in i))
    (if (> 0 value) (set (array-get out i) (if* (> (* -1 limit) value) (* -1 limit) value))
      (set (array-get out i) (if* (< limit value) limit value)))))

(define (sp-samples-limit in-out count limit) (void sp-sample-t* sp-time-t sp-sample-t)
  (sp-for-each-index i count (if (< limit (array-get in-out i)) (set (array-get in-out i) limit))))

(sc-comment "other")

(define (sp-shuffle swap in count)
  (void (function-pointer void void* sp-size-t sp-size-t) void* sp-size-t)
  "generic shuffle that works on any array type. fisher-yates algorithm"
  (declare j sp-size-t)
  (sp-for-each-index i count (set j (+ i (sp-time-random-bounded (- count i)))) (swap in i j)))

(define (sp-u64-from-array a count) (uint64-t uint8-t* sp-time-t)
  "lower value parts of large types are preferred if
   the system byte order is as expected little-endian"
  (case = count
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
    (8 (return (pointer-get (convert-type a uint64-t*))))
    (else (return 0))))
