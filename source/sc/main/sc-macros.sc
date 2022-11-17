(sc-define-syntax (sp-for-each-index index limit body ...)
  (for-each-index index size-t limit body ...))

(sc-define-syntax (sp-init* rate) (begin (pre-include "sph-sp.h") (pre-define _sp-rate rate)))

(sc-define-syntax* (sp-define* name-and-parameters types body ...)
  (let*
    ( (local-memory-used (sc-contains-expression (q local-memory-init) body))
      (error-memory-used (sc-contains-expression (q error-memory-init) body))
      (free-memory
        (if (or local-memory-used error-memory-used)
          (list
            (pairs (q if) (q status-is-failure)
              (append (if local-memory-used (list (q local-memory-free)) null)
                (if error-memory-used (list (q error-memory-free)) null))))
          null))
      (body
        (match body
          ( (body ... ((quote label) (quote exit) exit-content ...))
            (append body
              (qq ((label exit (unquote-splicing free-memory) (unquote-splicing exit-content))))))
          (_ (append body (qq ((label exit (unquote-splicing free-memory) status-return))))))))
    (qq
      (define (unquote name-and-parameters) (unquote (pair (q status-t) (any->list types)))
        status-declare
        (unquote-splicing body)))))

(sc-define-syntax (sp-define-event-prepare* name body ...)
  (sp-define* (name _event)
    sp-event-t*
    (define _duration sp-time-t (- _event:end _event:start))
    body
    ...
    (if _event:prepare (status-require (_event:prepare _event)))))

(sc-define-syntax* (sp-define-event* name-and-options body ...)
  (let*
    ( (name-and-options
        (match name-and-options ((name duration) (pair name duration))
          ((name) (pair name 0)) (name (pair name 0))))
      (name (first name-and-options)) (duration (tail name-and-options))
      (variable-name (symbol-append name (q -event)))
      (prepare-name (symbol-append name (q -prepare))))
    (qq
      (begin
        (sp-define-event-prepare* (unquote prepare-name) (unquote-splicing body))
        (sp-define-event (unquote variable-name) (unquote prepare-name) (unquote duration))))))

(sc-define-syntax (sp-define-group* name-and-options body ...)
  (sp-define-event* name-and-options (set _event:prepare sp-group-prepare) body ...))

(sc-define-syntax (sp-define-group-parallel* name-and-options body ...)
  (sp-define-event* name-and-options (set _event:prepare sp-group-prepare-parallel) body ...))

(sc-define-syntax* (sp-channel-config-set* channel-config-array (channel-index setting ...) ...)
  "set one or multiple channel config structs in an array"
  (pair (q begin)
    (map
      (l (i a)
        (qq
          (struct-set (array-get (unquote channel-config-array) (unquote i)) (unquote-splicing a))))
      channel-index setting)))

(sc-define-syntax (sp-local-samples* size pointer)
  (begin (status-require (sp-samples-new size (address-of pointer))) (local-memory-add pointer)))

(sc-define-syntax (sp-local-times* size pointer)
  (begin (status-require (sp-times-new size (address-of pointer))) (local-memory-add pointer)))

(sc-define-syntax (sp-local-units* size pointer)
  (begin (status-require (sp-units-new size (address-of pointer))) (local-memory-add pointer)))

(sc-define-syntax (sp-event-memory-new* event-pointer allocator allocator-arguments ... pointer)
  (begin
    (sp-event-memory* event-pointer 1)
    (srq (allocator allocator-arguments ... (address-of pointer)))
    (sp-event-memory-add event-pointer pointer)))

(sc-define-syntax (sp-event-malloc* event-pointer size pointer)
  (begin
    (sp-event-memory* event-pointer 1)
    (srq (sph-helper-malloc size (address-of pointer)))
    (sp-event-memory-add event-pointer pointer)))

(sc-define-syntax (sp-event-malloc-type-n* event-pointer count type pointer)
  (sp-event-malloc* event-pointer (* count (sizeof type)) pointer))

(sc-define-syntax (sp-event-malloc-type* event-pointer type pointer)
  (sp-event-malloc* event-pointer (sizeof type) pointer))

(sc-define-syntax (sp-event-samples* event-pointer size pointer)
  (sp-event-memory-new* event-pointer sp-samples-new size pointer))

(sc-define-syntax (sp-event-times* event-pointer size pointer)
  (sp-event-memory-new* event-pointer sp-times-new size pointer))

(sc-define-syntax (sp-event-units* event-pointer size pointer)
  (sp-event-memory-new* event-pointer sp-units-new size pointer))

(sc-define-syntax (sp-time-random*) (sp-time-random &sp-random-state))
(sc-define-syntax (sp-time-random-bounded* range) (sp-time-random-bounded &sp-random-state range))
(sc-define-syntax (sp-sample-random*) (sp-sample-random &sp-random-state))

(sc-define-syntax (sp-sample-random-bounded* range)
  (sp-sample-random-bounded &sp-random-state range))

(sc-define-syntax (sp-unit-random*) (sp-unit-random &sp-random-state))

(sc-define-syntax (sp-event-memory* event-pointer count)
  (srq (sp-event-memory-init event-pointer count)))

(sc-define-syntax (sp-group-add* group event) (srq (sp-group-add group event)))
(sc-define-syntax (sp-render-file* event) (srq (sp-render event 0)))
(sc-define-syntax (sp-render-plot* event) (srq (sp-render event 1)))

(sc-define-syntax* (sp-declare-struct-type name-and-fields ...)
  "(declare-struct-type name (field/type ...):fields name/fields ...)"
  (pair (q begin)
    (map-slice 2
      (l (name fields)
        (qq (declare (unquote name) (type (struct (unquote-splicing (map-slice 2 list fields)))))))
      name-and-fields)))

(sc-define-syntax* (sp-define-path* name type segment-type points ...)
  "automatically takes duration from points list.
   segment-type is used when elements of points arent prefixed with a segment-type.
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
        (sp-event-memory-add _event (unquote name))))))

(sc-define-syntax (sp-define-path-samples* name segment-type points ...)
  (sp-define-path* name samples segment-type points ...))

(sc-define-syntax (sp-define-path-times* name segment-type points ...)
  (sp-define-path* name times segment-type points ...))