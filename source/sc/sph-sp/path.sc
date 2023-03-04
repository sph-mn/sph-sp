(define (sp-path-samples-new path size out) (status-t sp-path-t sp-time-t sp-sample-t**)
  "out memory is allocated"
  status-declare
  (declare out-temp sp-sample-t*)
  (if (= 0 size) (set size (sp-path-size path)))
  (status-require (sp-samples-new size &out-temp))
  (spline-path-get &path 0 size out-temp)
  (set *out out-temp)
  (label exit status-return))

(define (sp-path-times-new path size out) (status-t sp-path-t sp-time-t sp-time-t**)
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

(define (sp-path-curves-config-new segment-count out) (status-t sp-time-t sp-path-curves-config-t*)
  status-declare
  (srq (sp-samples-new segment-count &out:x))
  (srq (sp-samples-new segment-count &out:y))
  (srq (sp-samples-new segment-count &out:c))
  (set out:segment-count segment-count)
  (label exit status-return))

(define (sp-path-curves-config-free a) (void sp-path-curves-config-t)
  (free a.x)
  (free a.y)
  (free a.c))

(define (sp-path-curves-new config out) (status-t sp-path-curves-config-t sp-path-t*)
  "a path that uses linear or circular interpolation depending on the values of the config.c.
   config.c 0 is linear and other values between -1.0 and 1.0 add curvature"
  status-declare
  (declare ss sp-path-segment-t* x sp-time-t y sp-sample-t c sp-sample-t)
  (srq (sp-malloc-type config.segment-count sp-path-segment-t &ss))
  (set (array-get ss 0) (sp-path-move (array-get config.x 0) (array-get config.y 0)))
  (define ss-index sp-time-t 1)
  (for ((define i sp-time-t 1) (< i config.segment-count) (set+ i 1))
    (set x (array-get config.x i))
    (if (= x (array-get config.x (- i 1))) continue)
    (set
      y (array-get config.y i)
      c (array-get config.c i)
      (array-get ss ss-index) (if* (< c 1.0e-5) (sp-path-line x y) (sp-path-bezier-arc c x y)))
    (set+ ss-index 1))
  (spline-path-set out ss ss-index)
  (label exit status-return))

(define (sp-path-curves-times-new config length out)
  (status-t sp-path-curves-config-t sp-time-t sp-time-t**)
  status-declare
  (declare path sp-path-t)
  (error-memory-init 1)
  (srq (sp-path-curves-new config &path))
  (error-memory-add path.segments)
  (srq (sp-path-times-new path length out))
  (label exit (if status-is-failure error-memory-free) status-return))

(define (sp-path-curves-samples-new config length out)
  (status-t sp-path-curves-config-t sp-time-t sp-sample-t**)
  status-declare
  (declare path sp-path-t)
  (error-memory-init 1)
  (srq (sp-path-curves-new config &path))
  (error-memory-add path.segments)
  (srq (sp-path-samples-new path length out))
  (label exit (if status-is-failure error-memory-free) status-return))