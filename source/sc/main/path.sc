(sc-include-once "./sc-macros")

(define (sp-path-samples-new path size out) (status-t sp-path-t sp-path-time-t sp-sample-t**)
  "out memory is allocated"
  status-declare
  (declare out-temp sp-sample-t*)
  (if (= 0 size) (set size (sp-path-size path)))
  (status-require (sp-samples-new size &out-temp))
  (spline-path-get path 0 size out-temp)
  (set *out out-temp)
  (label exit status-return))

(define (sp-path-times-new path size out) (status-t sp-path-t sp-path-time-t sp-time-t**)
  "return a sp_time_t array from path.
   memory is allocated and ownership transferred to the caller"
  status-declare
  (declare out-temp sp-time-t* temp sp-sample-t*)
  (set temp 0)
  (if (= 0 size) (set size (sp-path-size path)))
  (status-require (sp-path-samples-new path size &temp))
  (status-require (sp-times-new size &out-temp))
  (sp-samples->times temp size out-temp)
  (set *out out-temp)
  (label exit (free temp) status-return))

(define (sp-path-times-1 out size s1) (status-t sp-time-t** sp-time-t sp-path-segment-t)
  "return a newly allocated sp_time_t array for a path with one segment"
  (declare s (array sp-path-segment-t 1 s1) path sp-path-t)
  (spline-path-set &path s 1)
  (return (sp-path-times-new path size out)))

(define (sp-path-times-2 out size s1 s2)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t)
  (declare s (array sp-path-segment-t 2 s1 s2) path sp-path-t)
  (spline-path-set &path s 2)
  (return (sp-path-times-new path size out)))

(define (sp-path-times-3 out size s1 s2 s3)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (declare s (array sp-path-segment-t 3 s1 s2 s3) path sp-path-t)
  (spline-path-set &path s 3)
  (return (sp-path-times-new path size out)))

(define (sp-path-times-4 out size s1 s2 s3 s4)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (declare s (array sp-path-segment-t 4 s1 s2 s3 s4) path sp-path-t)
  (spline-path-set &path s 4)
  (return (sp-path-times-new path size out)))

(define (sp-path-samples-1 out size s1) (status-t sp-sample-t** sp-time-t sp-path-segment-t)
  (declare s (array sp-path-segment-t 1 s1) path sp-path-t)
  (spline-path-set &path s 1)
  (return (sp-path-samples-new path size out)))

(define (sp-path-samples-2 out size s1 s2)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t)
  (declare s (array sp-path-segment-t 2 s1 s2) path sp-path-t)
  (spline-path-set &path s 2)
  (return (sp-path-samples-new path size out)))

(define (sp-path-samples-3 out size s1 s2 s3)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (declare s (array sp-path-segment-t 3 s1 s2 s3) path sp-path-t)
  (spline-path-set &path s 3)
  (return (sp-path-samples-new path size out)))

(define (sp-path-samples-4 out size s1 s2 s3 s4)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (declare s (array sp-path-segment-t 4 s1 s2 s3 s4) path sp-path-t)
  (spline-path-set &path s 4)
  (return (sp-path-samples-new path size out)))

(define (sp-path-derivation path x-changes y-changes index out)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-path-t*)
  "changes contains per point an array of values which will be multiplied with x or y values for index.
   each is an array with the layout ((number:derivation_change ...):point_change ...).
   index is the current derivation_change index.
   caveats:
   * changes for segments of type constant or type path are not to be included
   * path size can change, sp_path_size(path) can give the new size
   * invalid paths are possible if x_changes exceed range between the new previous and next point"
  status-declare
  (declare
    p-i sp-path-time-t
    p sp-path-point-t*
    s-i sp-path-segment-count-t
    sp-i sp-path-time-t
    s sp-path-segment-t*
    ss sp-path-segment-t*)
  (sc-comment "copy segments")
  (set ss 0)
  (status-require (sph-helper-malloc (* path.segments-count (sizeof sp-path-segment-t)) &ss))
  (memcpy ss path.segments (* path.segments-count (sizeof sp-path-segment-t)))
  (sc-comment "modify points")
  (for ((set s-i 0 p-i 0) (< s-i path.segments-count) (set+ s-i 1))
    (set s (+ ss s-i))
    (for ((set sp-i 0) (< sp-i (spline-path-segment-points-count *s)) (set+ sp-i 1))
      (if (= spline-path-i-constant s:interpolator) break
        (if (= spline-path-i-path s:interpolator) continue))
      (set p (+ sp-i s:points))
      (set* p:x (array-get x-changes p-i index) p:y (array-get y-changes p-i index))
      (set+ p-i 1)))
  (sp-path-prepare-segments ss path.segments-count)
  (set out:segments ss out:segments-count path.segments-count)
  (label exit status-return))

(define (sp-path-samples-derivation path x-changes y-changes index out out-size)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-sample-t** sp-path-time-t*)
  "get one derivation as a sp_sample_t array. out memory will be allocated"
  status-declare
  (declare out-temp sp-sample-t* size sp-path-time-t)
  (status-require (sp-path-derivation path x-changes y-changes index &path))
  (set size (sp-path-size path))
  (status-require (sp-samples-new size &out-temp))
  (spline-path-get path 0 size out-temp)
  (set *out out-temp *out-size size)
  (label exit status-return))

(define (sp-path-times-derivation path x-changes y-changes index out out-size)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-time-t** sp-path-time-t*)
  "get one derivation as a sp_time_t array. out memory will be allocated"
  status-declare
  (declare result sp-time-t* temp sp-sample-t* temp-size sp-time-t)
  (set temp 0)
  (status-require (sp-path-samples-derivation path x-changes y-changes index &temp &temp-size))
  (status-require (sp-times-new temp-size &result))
  (sp-samples->times temp temp-size result)
  (set *out result *out-size temp-size)
  (label exit (free temp) status-return))

(define (sp-path-multiply path x-factor y-factor) (void sp-path-t sp-sample-t sp-sample-t)
  "multiply all x and y values of path segments by x_factor and y_factor respectively"
  (declare s sp-path-segment-t* p sp-path-point-t*)
  (for-each-index s-i path.segments-count
    (set s (+ path.segments s-i))
    (for-each-index sp-i (< sp-i (spline-path-segment-points-count *s))
      (if (= spline-path-i-constant s:interpolator) break
        (if (= spline-path-i-path s:interpolator) continue))
      (set p (+ sp-i s:points)) (set* p:x x-factor p:y y-factor)))
  (sp-path-prepare-segments path.segments path.segments-count))

(define (sp-path-derivations-normalized base count x-changes y-changes out)
  (status-t sp-path-t sp-time-t sp-sample-t** sp-sample-t** sp-path-t**)
  "get count derived paths and adjust y values so that their sum follows base.y.
   algorithm
     for each (segment, point, path): get sum
     for each (segment, point, path): scale to sum"
  (declare
    paths sp-path-t*
    y-sum sp-sample-t
    bs sp-path-segment-t*
    bp sp-path-point-t*
    s sp-path-segment-t*
    p sp-path-point-t*
    factor sp-sample-t)
  status-declare
  (status-require (sph-helper-calloc (* count (sizeof sp-path-t)) &paths))
  (for-each-index path-i count
    (status-require (sp-path-derivation base x-changes y-changes path-i (+ paths path-i))))
  (for-each-index segment-i base.segments-count
    (set bs (+ base.segments segment-i))
    (for-each-index point-i (spline-path-segment-points-count *bs)
      (if (= spline-path-i-constant bs:interpolator) break
        (if (= spline-path-i-path bs:interpolator) continue))
      (set bp (+ bs:points point-i) y-sum 0)
      (for-each-index path-i count
        (set s (+ (struct-get (array-get paths path-i) segments) segment-i) p (+ s:points point-i))
        (set+ y-sum p:y))
      (set factor (if* (= 0 y-sum) 0 (/ bp:y y-sum)))
      (for-each-index path-i count
        (set s (+ (struct-get (array-get paths path-i) segments) segment-i) p (+ s:points point-i))
        (set* p:y factor))))
  (for-each-index path-i count
    (sp-path-prepare-segments (struct-get (array-get paths path-i) segments) base.segments-count))
  (set *out paths)
  (label exit (if status-is-failure (if paths (free paths))) status-return))

(define (sp-path-samples-derivations-normalized path count x-changes y-changes out out-sizes)
  (status-t sp-path-t sp-time-t sp-sample-t** sp-sample-t** sp-sample-t*** sp-time-t**)
  "get sp_path_derivations_normalized as sample arrays. out and out_sizes is allocated and passed to the caller"
  (declare size sp-time-t paths sp-path-t* samples sp-sample-t** sizes sp-time-t*)
  status-declare
  (memreg-init 2)
  (status-require (sph-helper-calloc (* count (sizeof sp-sample-t*)) &samples))
  (memreg-add samples)
  (status-require (sph-helper-malloc (* count (sizeof sp-time-t)) &sizes))
  (memreg-add sizes)
  (status-require (sp-path-derivations-normalized path count x-changes y-changes &paths))
  (for-each-index i count
    (set size (sp-path-size (array-get paths i)) (array-get sizes i) size)
    (status-require (sp-samples-new size (+ samples i)))
    (sp-path-get (array-get paths i) 0 size (array-get samples i)))
  (set *out samples *out-sizes sizes)
  (label exit
    (if status-is-failure (begin (for-each-index i count (free (array-get samples i))) memreg-free))
    (if paths (free paths))
    status-return))