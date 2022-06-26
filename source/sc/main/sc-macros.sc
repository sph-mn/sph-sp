(sc-define-syntax (sp-for-each-index index limit body ...)
  (for-each-index index size-t limit body ...))

(sc-define-syntax (sp-init* rate) (begin (pre-include "sph-sp.h") (pre-define _sp-rate rate)))

(sc-define-syntax* (sp-declare-struct-type name-and-fields ...)
  "(declare-struct-type name (field/type ...):fields name/fields ...)"
  (pair (q begin)
    (map-slice 2
      (l (name fields)
        (qq (declare (unquote name) (type (struct (unquote-splicing (map-slice 2 list fields)))))))
      name-and-fields)))

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

(sc-define-syntax (sp-event-config* name type)
  (define name type (pointer-get (convert-type _event:config (sc-concat type *)))))

(sc-define-syntax* (sp-event-memory* a ...)
  (match a
    ((event size) (qq (status-require (sp-event-memory-init (unquote event) (unquote size)))))
    ((size) (qq (sp-event-memory* _event (unquote size))))))

(sc-define-syntax* (sp-group-add* a ...)
  (match a ((group event) (qq (srq (sp-group-add (unquote group) (unquote event)))))
    ((event) (qq (sp-group-add* _event (unquote event))))))

(sc-define-syntax* (sp-channel-config* channel-config-array (channel-index setting ...) ...)
  "set one or multiple channel config structs in an array"
  (pair (q begin)
    (map
      (l (i a)
        (qq
          (struct-set (array-get (unquote channel-config-array) (unquote i)) (unquote-splicing a))))
      channel-index setting)))

(sc-define-syntax* (sp-event-config-options* name options ...)
  "set event config values for pointer variable 'name'.
   usage: (sp-event-config-options* myconfig (config:key/value ...) channel-config:key/value ...)"
  (pair (q begin)
    (match options
      ( ( (config ...) channel-config ...)
        (append
          (if (null? config) null
            (qq ((struct-pointer-set (unquote name) (unquote-splicing config)))))
          (if (null? channel-config) null
            (qq
              ( (sp-channel-config* (struct-pointer-get (unquote name) channel-config)
                  (unquote-splicing channel-config)))))))
      (_ null))))

(sc-define-syntax* (sp-event-config-define-new* name type event options ...)
  (qq (begin (declare name (type *)) (sp-event-config-new* type (address-of name)))))

(sc-define-syntax* (sp-event-config-new* event type options ...)
  "allocate, set with optional channel config syntax, track memory"
  (qq
    (begin
      (status-require
        (sp-malloc-type 1 (unquote type) (address-of (struct-pointer-get (unquote event) config))))
      (sp-event-config-options*
        (convert-type (struct-pointer-get (unquote event) config) (sc-concat (unquote type) *))
        (unquote-splicing options))
      (sp-event-memory-add (unquote event) (struct-pointer-get (unquote event) config)))))

(sc-define-syntax* (sp-event-config-new2* pointer type-new event options ...)
  (qq
    (begin
      (status-require ((unquote type-new) (address-of (unquote pointer))))
      (sp-event-config-options* (unquote pointer) (unquote-splicing options))
      (sp-event-memory-add (unquote event) (unquote pointer)))))

(sc-define-syntax (sp-wave-config* name ...) "declare config variables"
  (begin (declare name sp-wave-event-config-t*) ...))

(sc-define-syntax (sp-wave-config-new* name event options ...)
  "allocate config object and set options. (name event (config ...) channel-config ...)"
  (sp-event-config-new2* name sp-wave-event-config-new event options ...))

(sc-define-syntax (sp-wave* event _config) "setup the event to be a wave event"
  (struct-pointer-set event config _config prepare sp-wave-event-prepare))

(sc-define-syntax (sp-sound-config-new* name event options ...)
  (sp-event-config-new2* name sp-sound-event-config-new event options ...))

(sc-define-syntax (sp-sound* event _config) "setup the event to be a wave event"
  (struct-pointer-set event config _config prepare sp-sound-event-prepare))

(sc-define-syntax (sp-sound-config* name ...) (begin (declare name sp-sound-event-config-t*) ...))
(sc-define-syntax (sp-noise-config* name ...) (begin (declare name sp-noise-event-config-t*) ...))

(sc-define-syntax (sp-noise-config-new* name event options ...)
  (sp-event-config-new2* name sp-noise-event-config-new event options ...))

(sc-define-syntax (sp-noise* event _config)
  (struct-pointer-set event config _config prepare sp-noise-event-prepare))

(sc-define-syntax (sp-cheap-noise-config* name ...)
  (begin (declare name sp-cheap-noise-event-config-t*) ...))

(sc-define-syntax (sp-cheap-noise-config-new* name event options ...)
  (sp-event-config-new2* name sp-cheap-noise-event-config-new event options ...))

(sc-define-syntax (sp-cheap-noise* event _config)
  (struct-pointer-set event config _config prepare sp-cheap-noise-event-prepare))

(sc-define-syntax* (sp-event* a ...)
  (match a
    ( (name value)
      (qq (begin (sp-declare-event (unquote name)) (set (unquote name) (unquote value)))))
    ((name) (qq (sp-declare-event (unquote name))))))

(sc-define-syntax* (sp-time* name/value ...)
  (pair (q begin)
    (map-slice 2 (l (name value) (qq (define (unquote name) sp-time-t (unquote value)))) name/value)))

(sc-define-syntax* (sp-sample* name/value ...)
  (pair (q begin)
    (map-slice 2 (l (name value) (qq (define (unquote name) sp-sample-t (unquote value))))
      name/value)))

(sc-define-syntax (sp-array* type type-new pointer size values ...)
  "declare array variable and length variable, allocate array,\n   register memory with current event and set optional given values in order"
  (begin
    (status-require (type-new size pointer))
    (sp-event-memory-add* _event pointer)
    (array-set* pointer values ...)))

(sc-define-syntax* (sp-times* name size-and-values ...)
  (match size-and-values (() (qq (declare (unquote name) sp-time-t*)))
    ( (size values ...)
      (qq
        (sp-array* sp-time-t* sp-times-new (unquote name) (unquote size) (unquote-splicing values))))))

(sc-define-syntax (sp-samples* size pointer) (status-require (sp-samples-new size pointer)))
(sc-define-syntax (sp-times* size pointer) (status-require (sp-times-new size pointer)))
(sc-define-syntax (sp-units* size pointer) (status-require (sp-units-new size pointer)))

(sc-define-syntax (sp-local-samples* size pointer)
  (begin (status-require (sp-samples-new size pointer)) (local-memory-add pointer)))

(sc-define-syntax (sp-local-times* size pointer)
  (begin (status-require (sp-times-new size pointer)) (local-memory-add pointer)))

(sc-define-syntax (sp-local-units* size pointer)
  (begin (status-require (sp-units-new size pointer)) (local-memory-add pointer)))

(sc-define-syntax (sp-event-samples* size pointer)
  (begin (status-require (sp-samples-new size pointer)) (sp-event-memory-add* pointer)))

(sc-define-syntax (sp-event-times* size pointer)
  (begin (status-require (sp-times-new size pointer)) (sp-event-memory-add* pointer)))

(sc-define-syntax (sp-event-units* size pointer)
  (begin (status-require (sp-units-new size pointer)) (sp-event-memory-add* pointer)))

(sc-define-syntax (sp-render-file*) (status-require (sp-render-quick *_event 0)))
(sc-define-syntax (sp-render-plot*) (status-require (sp-render-quick *_event 1)))

(sc-define-syntax (sp-define-main* name parallelization channels body ...)
  (define (name) status-t
    status-declare
    (sp-declare-group _group)
    (define _event sp-event-t* &_group)
    (sp-initialize parallelization channels _sp-rate)
    body
    ...
    (label exit status-return)))

(sc-define-syntax* (sp-path* name type segment-type points ...)
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

(sc-define-syntax (sp-path-samples* name segment-type points ...)
  (sp-path* name samples segment-type points ...))

(sc-define-syntax (sp-path-times* name segment-type points ...)
  (sp-path* name times segment-type points ...))

(sc-define-syntax* (sp-intervals* name tempo values ...)
  (let ((count (length values)) (count-name (symbol-append name (q -length))))
    (qq
      (begin
        (sc-insert "// sp-intervals*\n")
        (define (unquote count-name) sp-time-t (unquote count))
        (sp-times* (unquote name) (unquote count-name) (unquote-splicing values))
        (sp-times-multiply-1 (unquote name) (unquote count-name) (unquote tempo) (unquote name))
        (sp-times-cusum (unquote name) (unquote count-name) (unquote name))))))