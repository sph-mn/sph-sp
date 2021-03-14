(sc-define-syntax (for-each-index index limit body ...)
  (for ((define index sp-time-t 0) (< index limit) (set+ index 1)) body ...))

(sc-define-syntax* (sp-channel-config* channel-config-array (channel-index setting ...) ...)
  "set one or multiple channel config structs in an array"
  (pair (q begin)
    (map
      (l (i a)
        (qq
          (struct-set (array-get (unquote channel-config-array) (unquote i)) (unquote-splicing a))))
      channel-index setting)))

(sc-define-syntax*
  (sp-channel-config-event* label prepare-event event config (setting ...) channel-config ...)
  "generic macro for creating similar kinds of events that receive a config struct and channel config"
  (qq
    (begin
      (sc-insert (unquote (string-append "// " label "\n")))
      (struct-pointer-set (unquote config) (unquote-splicing setting))
      (sp-channel-config* (struct-get (unquote config) channel-config)
        (unquote-splicing channel-config))
      (struct-pointer-set (unquote event) data (unquote config) prepare (unquote prepare-event)))))

(sc-define-syntax (sp-wave* event config config-settings channel-config ...)
  (sp-channel-config-event* "sp-wave*" sp-wave-event-prepare
    event config config-settings channel-config ...))

(sc-define-syntax (sp-noise* event config config-settings channel-config ...)
  (sp-channel-config-event* "sp-noise*" sp-noise-event-prepare
    event config config-settings channel-config ...))

(sc-define-syntax (sp-cheap-noise* event config config-settings channel-config ...)
  (sp-channel-config-event* "sp-cheap-noise*" sp-cheap-noise-event-prepare
    event config config-settings channel-config ...))

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
      (body (append body (list (q (if _event:prepare (srq (_event:prepare _event)))))))
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

(sc-define-syntax* (sp-path* name type segment-types points ...)
  (let*
    ( (sp-path-new (if (eq? type (q times)) (q sp-path-times-new) (q sp-path-samples-new)))
      (data-type (if (eq? type (q times)) (q sp-time-t*) (q sp-sample-t*)))
      (duration (second (reverse (last points))))
      (segment-types (if (list? segment-types) segment-types (map (l (x) segment-types) points)))
      (segments
        (map (l (segment-type points) (pair (symbol-append (q sp-path-) segment-type) points))
          segment-types points))
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

(sc-define-syntax (sp-noise-config* name)
  (begin
    (sc-insert "// sp-noise-config*\n")
    (declare name sp-noise-event-config-t*)
    (status-require (sp-noise-event-config-new (address-of name)))
    (sp-event-memory-add1 _event name)))

(sc-define-syntax (sp-event-memory* size) (srq (sp-event-memory-init _event size)))

(sc-define-syntax* (sp-times* name values ...)
  (qq
    (declare (unquote name) (array sp-time-t (unquote (length values)) (unquote-splicing values)))))

(sc-define-syntax* (sp-samples* name values ...)
  (qq
    (declare (unquote name) (array sp-sample-t (unquote (length values)) (unquote-splicing values)))))