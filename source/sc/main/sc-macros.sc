(sc-define-syntax (for-each-index index limit body ...)
  (for ((define index sp-time-t 0) (< index limit) (set+ index 1)) body ...))

(sc-define-syntax* (sp-define-event-prepare* name body ...)
  "* arguments and types are implicit
   * status-declare is implicit
   * exit label with status-return is optional
   * free-on-error-free/free-on-exit-free is added automatically if feature use found"
  (let*
    ( (free-on-error-used (sc-contains-expression (q free-on-error-init) body))
      (free-on-exit-used (sc-contains-expression (q free-on-exit-init) body))
      (free-memory
        (if (or free-on-error-used free-on-exit-used)
          (pairs (q if) (q status-is-failure)
            (if free-on-error-used (q free-on-error-free) null)
            (if free-on-exit-used (list (q free-on-exit-free)) null))
          (q (begin))))
      (body (append body (list (q (if _event:prepare (status-require (_event:prepare _event)))))))
      (body
        (match body
          ( (body ... ((quote label) (quote exit) exit-content ...))
            (append body (qq ((label exit (unquote free-memory) (unquote-splicing exit-content))))))
          (_ (append body (qq ((label exit (unquote free-memory) status-return))))))))
    (qq
      (define ((unquote name) _event) (status-t sp-event-t*)
        status-declare
        (define _duration sp-time-t (- _event:end _event:start))
        (unquote-splicing body)))))

(sc-define-syntax* (sp-define-event* name-and-options body ...)
  (let*
    ( (name-and-options
        (match name-and-options ((name duration) (pair name duration))
          ((name) (pair name 0)) (name (pair name 0))))
      (name (first name-and-options)) (duration (tail name-and-options))
      (prepare-name (symbol-append name (q -prepare))))
    (qq
      (begin
        (sp-define-event-prepare* (unquote prepare-name) (unquote-splicing body))
        (sp-define-event (unquote name) (unquote prepare-name) (unquote duration))))))

(sc-define-syntax (sp-define-group* name-and-options body ...)
  (sp-define-event* name-and-options (set _event:prepare sp-group-prepare) body ...))

(sc-define-syntax (sp-define-group-parallel* name-and-options body ...)
  (sp-define-event* name-and-options (set _event:prepare sp-group-prepare-parallel) body ...))

(sc-define-syntax* (sp-path* name type segment-type points ...)
  "automatically takes duration from points list.
   segment-type is a short symbol instead of the full function name (eg line instead of sp_path_line)
   segment-type can be omitted, in which case the default is line"
  (let*
    ( (segment-type?
        (l (a)
          (and (symbol? a)
            (or (eq? (q line) a) (eq? (q bezier) a)
              (eq? (q move) a) (eq? (q constant) a) (eq? (q path) a)))))
      (points (if (segment-type? segment-type) points (pair segment-type points)))
      (segment-type (if (segment-type? segment-type) segment-type (q line)))
      (sp-path-new (if (eq? type (q times)) (q sp-path-times-new) (q sp-path-samples-new)))
      (data-type (if (eq? type (q times)) (q sp-time-t*) (q sp-sample-t*)))
      (duration (second (reverse (last points))))
      (segments
        (map
          (l (points)
            (pair
              (symbol-append (q sp-path-)
                (if (segment-type? (first points)) (first points) segment-type))
              (if (segment-type? (first points)) (tail points) points)))
          points))
      (name-path (symbol-append name (q -path))) (name-segments (symbol-append name (q -segments))))
    (qq
      (begin
        (sc-insert "// sp-path*\n")
        (declare
          (unquote name) (unquote data-type)
          (unquote name-path) sp-path-t
          (unquote name-segments) (array sp-path-segment-t (unquote (length segments))))
        (array-set* (unquote name-segments) (unquote-splicing segments))
        (spline-path-set (address-of (unquote name-path)) (unquote name-segments)
          (unquote (length segments)))
        (status-require
          ((unquote sp-path-new) (unquote name-path) (unquote duration) (address-of (unquote name))))
        (sp-event-memory-add1 _event (unquote name))))))

(sc-define-syntax (sp-path-samples* name segment-type points ...)
  (sp-path* name samples segment-type points ...))

(sc-define-syntax (sp-path-times* name segment-type points ...)
  (sp-path* name times segment-type points ...))

(sc-define-syntax (sp-event-memory* size) (status-require (sp-event-memory-init _event size)))

(sc-define-syntax* (sp-channel-config* channel-config-array (channel-index setting ...) ...)
  "set one or multiple channel config structs in an array"
  (pair (q begin)
    (map
      (l (i a)
        (qq
          (struct-set (array-get (unquote channel-config-array) (unquote i)) (unquote-splicing a))))
      channel-index setting)))

(sc-define-syntax* (sp-core-event-config* label type type-new name options ...)
  (qq
    (begin
      (sc-insert (unquote (string-append "// " label "\n")))
      (declare (unquote name) (unquote type))
      (status-require ((unquote type-new) (address-of (unquote name))))
      (unquote-splicing
        (match options
          ( ( (config ...) channel-config ...)
            (append
              (if (null? config) null
                (qq ((struct-pointer-set (unquote name) (unquote-splicing config)))))
              (if (null? channel-config) null
                (qq
                  ( (sp-channel-config* (struct-pointer-get (unquote name) channel-config)
                      (unquote-splicing channel-config)))))))
          (_ null)))
      (sp-event-memory-add1 _event (unquote name)))))

(sc-define-syntax (sp-noise-config* name options ...)
  (sp-core-event-config* "sp-noise-config*" sp-noise-event-config-t*
    sp-noise-event-config-new name options ...))

(sc-define-syntax (sp-wave-config* name options ...)
  (sp-core-event-config* "sp-wave-config*" sp-wave-event-config-t*
    sp-wave-event-config-new name options ...))

(sc-define-syntax (sp-cheap-noise-config* name options ...)
  (sp-core-event-config* "sp-cheap-noise-config*" sp-cheap-noise-event-config-t*
    sp-cheap-noise-event-config-new name options ...))

(sc-define-syntax (sp-noise* config)
  (begin
    (sc-insert "// sp-noise*\n")
    (struct-pointer-set _event data config prepare sp-noise-event-prepare)))

(sc-define-syntax (sp-wave* config)
  (begin
    (sc-insert "// sp-wave*\n")
    (struct-pointer-set _event data config prepare sp-wave-event-prepare)))

(sc-define-syntax (sp-cheap-noise* config)
  (begin
    (sc-insert "// sp-cheap-noise*\n")
    (struct-pointer-set _event data config prepare sp-cheap-noise-event-prepare)))

(sc-define-syntax (sp-array* type type-new name size values ...)
  (begin
    (declare name type)
    (status-require (type-new size (address-of name)))
    (sp-event-memory-add1 _event name)
    (array-set* name values ...)))

(sc-define-syntax* (sp-times* name size-and-values ...)
  (match size-and-values (() (qq (declare (unquote name) sp-time-t*)))
    ( (size values ...)
      (qq
        (sp-array* sp-time-t* sp-times-new (unquote name) (unquote size) (unquote-splicing values))))))

(sc-define-syntax* (sp-samples* name size-and-values ...)
  (match size-and-values (() (qq (declare (unquote name) sp-sample-t*)))
    ( (size values ...)
      (qq
        (sp-array* sp-sample-t* sp-samples-new
          (unquote name) (unquote size) (unquote-splicing values))))))

(sc-define-syntax* (sp-time* name/value ...)
  (pair (q begin)
    (map-slice 2 (l (name value) (qq (define (unquote name) sp-time-t (unquote value)))) name/value)))

(sc-define-syntax* (sp-sample* name/value ...)
  (pair (q begin)
    (map-slice 2 (l (name value) (qq (define (unquote name) sp-sample-t (unquote value))))
      name/value)))

(sc-define-syntax (sp-group-add* start duration volume data event)
  (status-require (sp-group-add-set _event start duration volume (convert-type data void*) event)))

(sc-define-syntax (sp-group-append* volume data event)
  (status-require (sp-group-append-set _event volume (convert-type data void*) event)))

(sc-define-syntax* (define-array* name type values ...)
  (qq
    (declare (unquote name)
      (array (unquote type) (unquote (length values)) (unquote-splicing values)))))

(sc-define-syntax (sp-define-times* name values ...) (define-array* name sp-time-t values ...))
(sc-define-syntax (sp-define-samples* name values ...) (define-array* name sp-sample-t values ...))
(sc-define-syntax (sp-render-file*) (status-require (sp-render-quick *_event 0)))
(sc-define-syntax (sp-render-plot*) (status-require (sp-render-quick *_event 1)))
(sc-define-syntax (sp-init* rate) (begin (pre-include "sph-sp.h") (pre-define _sp-rate rate)))

(sc-define-syntax (sp-define-song* parallelization channels body ...)
  (define (main) status-t
    status-declare
    (sp-declare-group _song)
    (define _event sp-event-t* &_song)
    (sp-initialize parallelization channels _sp-rate)
    body
    ...
    (label exit status-return)))

(sc-define-syntax* (sp-intervals* name tempo values ...)
  (let ((count (length values)) (count-name (symbol-append name (q -length))))
    (qq
      (begin
        (sc-insert "// sp-intervals*\n")
        (define (unquote count-name) sp-time-t (unquote count))
        (sp-times* (unquote name) (unquote count-name) (unquote-splicing values))
        (sp-times-multiply-1 (unquote name) (unquote count-name) (unquote tempo) (unquote name))
        (sp-times-cusum (unquote name) (unquote count-name) (unquote name))))))