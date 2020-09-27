(sc-define-syntax (for-i index limit body ...)
  (for ((set index 0) (< index limit) (set+ index 1)) body ...))

(pre-define
  (define-sp-stat-range name value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    "out: min, max, range"
    (declare i sp-time-t min value-t max value-t b value-t)
    (set min (array-get a 0) max min)
    (for-i i size (set b (array-get a i)) (if (> b max) (set max b) (if (< b min) (set min b))))
    (set (array-get out 0) min (array-get out 1) max (array-get out 2) (- max min))
    (return 0))
  (define-sp-stat-deviation name stat-mean value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    "standard deviation"
    (declare i sp-time-t sum sp-sample-t dev sp-sample-t mean sp-sample-t)
    (stat-mean a size &mean)
    (set sum 0)
    (for-i i size (set dev (- (array-get a i) mean) sum (+ sum (* dev dev))))
    (set *out (sqrt (/ sum size)))
    (return 0))
  (define-sp-stat-skewness name stat-mean value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    "mean((x - mean(data)) ** 3) / (mean((x - mean(data)) ** 2) ** 3/2)"
    (declare i sp-time-t mean sp-sample-t m3 sp-sample-t m2 sp-sample-t b sp-sample-t)
    (set m2 0 m3 0)
    (stat-mean a size &mean)
    (for-i i size (set b (- (array-get a i) mean) m2 (+ m2 (* b b)) m3 (+ m3 (* b b b))))
    (set m3 (/ m3 size) m2 (/ m2 size) *out (/ m3 (sqrt (* m2 m2 m2))))
    (return 0))
  (define-sp-stat-kurtosis name stat-mean value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    "mean((x - mean(data)) ** 4) / (mean((x - mean(data)) ** 2) ** 2)"
    (declare b sp-sample-t i sp-time-t mean sp-sample-t m2 sp-sample-t m4 sp-sample-t)
    (set m2 0 m4 0)
    (stat-mean a size &mean)
    (for-i i size (set b (- (array-get a i) mean) m2 (+ m2 (* b b)) m4 (+ m4 (* b b b b))))
    (set m4 (/ m4 size) m2 (/ m2 size) *out (/ m4 (* m2 m2)))
    (return 0))
  (define-sp-stat-median name sort-less sort-swap value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    (declare temp value-t*)
    (set temp (malloc (* size (sizeof value-t))))
    (if (not temp) (return 1))
    (memcpy temp a (* size (sizeof value-t)))
    (quicksort sort-less sort-swap temp 0 (- size 1))
    (set *out
      (if* (bit-and size 1) (array-get temp (- (/ size 2) 1))
        (/ (+ (array-get temp (/ size 2)) (array-get temp (- (/ size 2) 1))) 2.0)))
    (return 0))
  (define-sp-stat-inharmonicity name value-t)
  (define (name a size out) (uint8-t value-t* sp-time-t sp-sample-t*)
    "calculate a value for total inharmonicity >= 0.
     the value is not normalised for different lengths of input.
     # formula
     n1: 0..n; n2: 0..n; half_offset(x) = 0.5 >= x ? x : x - 1;
     min(map(n1, mean(map(n2, half_offset(x(n2) / x(n1))))))"
    (declare i sp-time-t i2 sp-time-t b sp-sample-t sum sp-sample-t min sp-sample-t)
    (set sum 0 min size)
    (for ((set i 0) (< i size) (set+ i 1))
      (set sum 0)
      (for-i i2 size
        (set
          b (/ (array-get a i2) (convert-type (array-get a i) sp-sample-t))
          b (- b (sp-cheap-floor-positive b))
          b (if* (>= 0.5 b) b (- 1 b))
          sum (+ sum b)))
      (set sum (/ sum size))
      (if (< sum min) (set min sum)))
    (set *out min)
    (return 0)))

(sc-comment "times")

(define (sp-stat-times-center a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  "center of mass. the distribution of mass is balanced around the center of mass, and the average of
   the weighted position coordinates of the distributed mass defines its coordinates.
   sum(n * x(n)) / sum(x(n))"
  (declare i sp-time-t sum sp-time-t index-sum sp-time-t)
  (set index-sum 0 sum (array-get a 0))
  (for-i i size (set+ sum (array-get a i) index-sum (* i (array-get a i))))
  (set *out (/ index-sum (convert-type sum sp-sample-t)))
  (return 0))

(define-sp-stat-range sp-stat-times-range sp-time-t)

(sc-comment
  "repetition:
   * out: ratio
   * x: count of overlapping subsequences of widths 1..$size
   * y: the maximum number of possible unique subsequences
   * the result is x divided by y normalised by $size.
   * examples
     * low: 11111 121212
     * high: 12345 112212")

(define (sp-stat-times-repetition a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (declare
    count sp-time-t
    i sp-time-t
    key sp-sequence-set-key-t
    known sp-sequence-set-t
    possible-count sp-time-t
    ratios sp-sample-t
    value sp-sequence-set-key-t*)
  (if (sp-sequence-set-new size &known) (return 1))
  (set ratios 0)
  (for ((set key.size 1) (< key.size size) (set+ key.size 1))
    (set count 0 possible-count (- size key.size))
    (for-i i (- size (- key.size 1))
      (set key.data (convert-type (+ i a) uint8-t*) value (sp-sequence-set-get known key))
      (if value (set+ count 1)
        (if (not (sp-sequence-set-add known key)) (begin (sp-sequence-set-free known) (return 1)))))
    (set+ ratios (/ count possible-count))
    (sp-sequence-set-clear known))
  (set (array-get out 0) (/ ratios (- size 1)))
  (sp-sequence-set-free known)
  (return 0))

(define (sp-stat-times-mean a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (declare i sp-time-t sum sp-time-t)
  (set sum 0)
  (for-i i size (set+ sum (array-get a i)))
  (set *out (/ sum (convert-type size sp-sample-t)))
  (return 0))

(define-sp-stat-deviation sp-stat-times-deviation sp-stat-times-mean sp-time-t)
(define-sp-stat-median sp-stat-times-median sp-times-sort-less sp-times-sort-swap sp-time-t)
(define-sp-stat-skewness sp-stat-times-skewness sp-stat-times-mean sp-time-t)
(define-sp-stat-kurtosis sp-stat-times-kurtosis sp-stat-times-mean sp-time-t)
(sc-comment "samples")

(define (sp-stat-samples-center a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (declare i sp-time-t sum sp-sample-t index-sum sp-sample-t)
  (set index-sum 0 sum (sp-samples-sum a size))
  (for-i i size (set+ index-sum (* i (array-get a i))))
  (set *out (/ index-sum sum))
  (return 0))

(define-sp-stat-range sp-stat-samples-range sp-sample-t)
(define-sp-stat-inharmonicity sp-stat-times-inharmonicity sp-time-t)

(define (sp-samples-scale->times a size max out) (void sp-sample-t* sp-time-t sp-time-t sp-time-t*)
  "map input samples into the time range 0..max.
   makes all values positive by adding the absolute minimum
   then scales with multiplication so that the largest value is max
   then rounds to sp-time-t"
  (declare i sp-time-t range (array sp-sample-t 3) addition sp-sample-t)
  (sc-comment "returns min, max, range")
  (sp-stat-samples-range a size range)
  (set addition (if* (> 0 (array-get range 0)) (fabs (array-get range 0)) 0))
  (for-i i size
    (set (array-get out i)
      (sp-cheap-round-positive (* (+ (array-get a i) addition) (/ max (array-get range 2)))))))

(define (sp-stat-samples-repetition a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (declare b sp-time-t*)
  status-declare
  (status-require (sp-times-new size &b))
  (sp-samples-scale->times a size 1000 b)
  (sp-stat-times-repetition b size out)
  (free b)
  (label exit (return status.id)))

(define (sp-stat-samples-mean a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (set *out (/ (sp-samples-sum a size) size))
  (return 0))

(define-sp-stat-deviation sp-stat-samples-deviation sp-stat-samples-mean sp-sample-t)
(define-sp-stat-median sp-stat-samples-median sp-samples-sort-less sp-samples-sort-swap sp-sample-t)
(define-sp-stat-skewness sp-stat-samples-skewness sp-stat-samples-mean sp-sample-t)
(define-sp-stat-kurtosis sp-stat-samples-kurtosis sp-stat-samples-mean sp-sample-t)
(define-sp-stat-inharmonicity sp-stat-samples-inharmonicity sp-sample-t)