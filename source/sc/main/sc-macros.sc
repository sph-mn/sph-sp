(sc-comment
  "this file defines macros only available in sc, that are used as optional helpers to simplify common tasks where c syntax alone offers no good alternative")

(sc-define-syntax (for-each-index index limit body ...)
  (for ((define index sp-time-t 0) (< index limit) (set+ index 1)) body ...))

(sc-define-syntax* (sp-path target-address (type type-arg ...) ...)
  "type is sp_path_{type}"
  (let
    ( (segments
        (map (l (type type-arg) (pair (symbol-append (q sp-path-) type) type-arg)) type type-arg))
      (temp (sc-gensym)))
    (qq
      (begin
        (sc-insert "// sp-path\n")
        (declare (unquote temp) (array sp-path-segment-t (unquote (length segments))))
        (array-set* (unquote temp) (unquote-splicing segments))
        (spline-path-set (unquote target-address) (unquote temp) (unquote (length segments)))))))

(sc-define-syntax* (sp-path-array type path-array-f target-address duration segment ...)
  (let ((temp (sc-gensym)))
    (quasiquote
      (begin
        (declare (unquote temp) sp-path-t)
        (sp-path (address-of (unquote temp)) (unquote-splicing segment))
        (status-require
          ((unquote path-array-f) (unquote temp) (unquote duration) (unquote target-address)))))))

(sc-define-syntax (sp-path-samples target-address duration segment ...)
  (sp-path-array sp-sample-t* sp-path-samples-new target-address duration segment ...))

(sc-define-syntax (sp-path-times target-address duration segment ...)
  (sp-path-array sp-time-t* sp-path-times-new target-address duration segment ...))

(sc-define-syntax* (sp-define-event (name arguments ...) types body ...)
  "* first and last argument type, status_t and sp_event_t*, is implicit
   * exit label is optional
   * default sp_event_t variables _result, _event and _out
   * adds free-on-error-free/free-on-exit-free if a use of the feature is found"
  (let*
    ( (types (qq (status-t (unquote-splicing (any->list types)) sp-event-t*)))
      (free-on-error-count (sc-contains-expression (q free-on-error-init) body))
      (free-on-exit-count (sc-contains-expression (q free-on-exit-init) body))
      (free-memory
        (pairs (q if) (q status-is-failure)
          (if free-on-error-count (q free-on-error-free) (q (begin)))
          (if free-on-exit-count (list (q free-on-exit-free)) null)))
      (body
        (match body
          ( (body ... ((quote label) (quote exit) exit-content ...))
            (append body (qq ((label exit (unquote free-memory) (unquote exit-content))))))
          (_ (pair body (qq (label exit (unquote free-memory) status-return)))))))
    (qq
      (define ((unquote name) (unquote-splicing arguments) _out) (unquote types)
        status-declare
        (sp-declare-event _result)
        (sp-declare-event _event)
        (unquote-splicing (first body))
        (set *_out _result)
        (unquote (tail body))))))

(sc-define-syntax* (sp-channel-config channel-config-array (channel-index setting ...) ...)
  "set one or multiple channel config structs in an array"
  (pair (q begin)
    (map
      (l (i a)
        (qq
          (struct-set (array-get (unquote channel-config-array) (unquote i)) (unquote-splicing a))))
      channel-index setting)))

(sc-define-syntax*
  (sp-channel-config-event label make-event out start end config (setting ...) channel-config ...)
  "generic macro for creating similar kinds of events that receive a config struct and channel config"
  (qq
    (begin
      (sc-insert (unquote (string-append "// " label "\n")))
      (struct-set (unquote config) (unquote-splicing setting))
      (sp-channel-config (struct-get (unquote config) channel-config)
        (unquote-splicing channel-config))
      (srq ((unquote make-event) (unquote start) (unquote end) (unquote config) (unquote out))))))

(sc-define-syntax (sp-sine out start end config config-settings channel-config ...)
  (sp-channel-config-event "sp-sine" sp-wave-event
    out start end config config-settings channel-config ...))

(sc-define-syntax (sp-noise out start end config config-settings channel-config ...)
  (sp-channel-config-event "sp-noise" sp-noise-event
    out start end config config-settings channel-config ...))

(sc-define-syntax (sp-cheap-noise out start end config config-settings channel-config ...)
  (sp-channel-config-event "sp-chep-noise" sp-cheap-noise-event
    out start end config config-settings channel-config ...))