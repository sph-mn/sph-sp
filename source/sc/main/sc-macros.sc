(sc-define-syntax (for-i index limit body ...)
  (for ((set index 0) (< index limit) (set+ index 1)) body ...))

(sc-define-syntax* (define-array name type values ...)
  "define an array with data. sets dimensions automatically.
   example:
     (define-array a int (0 0 0 0) (0 0 0 0))
     ->
     int a[2][4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };"
  (let
    ( (dimensions
        (let loop ((a values))
          (if (null? a) null
            (let ((b (first a)))
              (pair (length a)
                (if (and (list? b) (not (and (symbol? (first b)) (sc-syntax? (first b))))) (loop b)
                  null)))))))
    (list (q declare) name (pairs (q array) type dimensions values))))

(sc-define-syntax* (define-nested-array name type values ...)
  "define an array of arrays with data. sets dimensions automatically.
   example:
     (define-nested-array a int (0 0 0 0) (0 0 0 0))
     ->
     int _t1[2][4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
     int* a[2] = { _t1[0], _t1[1] };"
  (let
    ( (id (sc-gensym))
      (nested?
        (lambda (a)
          (and (not (null? a))
            (let ((a (first a))) (and (list? a) (or (null? a) (not (sc-syntax? (first a))))))))))
    (if (nested? values)
      (list (q begin) (pairs (q define-array) id type values)
        (pairs (q define-array) name
          (symbol-append type (q *))
          (map-with-index (lambda (index value) (list (q array-get) id index)) values)))
      (pairs (q define-array) name type values))))

(sc-define-syntax* (define-array-set* name type values ...)
  "define an array with data but set values one by one with array-set*.
   allows to set runtime calculated values.
   example:
     (define-array-set* a int 3 4)
     ->
     int a[2];
     a[0] = calc(3);
     a[1] = 4;"
  (list (q begin) (list (q declare) name (list (q array) type (length values)))
    (pairs (q array-set*) name values)))

(sc-define-syntax* (sp-define-cluster id (key-and-values ...) ...)
  "defines a new function that returns an event group created by sp-sine-cluster.
   (sp-define-cluster cluster-1
     (amp (line 100 1) (line 200 0))
     (frq (move 0 1) (line 200 1))
     (phs 0 0 0.5 0)
     (ax (1 1 1 1) (1 1 1 1))
     (ay (1 1 1.5 1) (1 1 1 1))
     (fx (1 1 1 1) (1 2 1 1))
     (fy (1 2 3 4) (1 2 3 4)))
     ->
     status_t cluster_1(sp_sample_t volume, sp_time_t pitch, sp_time_t duration, sp_event_t* out);"
  (let
    ( (map-segments
        (lambda (a) (map (lambda (a) (pair (symbol-append (q sp-path-) (first a)) (tail a))) a)))
      (prt-count (length (alist-ref key-and-values (q phs)))))
    (qq
      (define ((unquote id) volume pitch duration out)
        (status-t sp-sample-t sp-time-t sp-time-t sp-event-t*)
        (declare amp-path sp-path-t frq-path sp-path-t)
        (define-array-set* amp sp-path-segment-t
          (unquote-splicing (map-segments (alist-ref key-and-values (q amp)))))
        (define-array-set* frq sp-path-segment-t
          (unquote-splicing (map-segments (alist-ref key-and-values (q frq)))))
        (define-array phs sp-time-t (unquote-splicing (alist-ref key-and-values (q phs))))
        (define-nested-array ax sp-sample-t (unquote-splicing (alist-ref key-and-values (q ax))))
        (define-nested-array ay sp-sample-t (unquote-splicing (alist-ref key-and-values (q ay))))
        (define-nested-array fx sp-sample-t (unquote-splicing (alist-ref key-and-values (q fx))))
        (define-nested-array fy sp-sample-t (unquote-splicing (alist-ref key-and-values (q fy))))
        (set
          amp-path.segments amp
          amp-path.segments-count (unquote (length (alist-ref key-and-values (q amp))))
          frq-path.segments frq
          frq-path.segments-count (unquote (length (alist-ref key-and-values (q amp)))))
        (sp-path-multiply amp-path duration volume)
        (sp-path-multiply frq-path duration pitch)
        (sp-times-multiply-1 phs (unquote prt-count) pitch phs)
        (return (sp-sine-cluster (unquote prt-count) amp-path frq-path phs ax ay fx fy out))))))