(sc-define-syntax (srq* a ...) (begin (srq a) ...))

(sc-define-syntax (sp-for-each-index index limit body ...)
  (for-each-index index sp-size-t limit body ...))

(sc-define-syntax* (sp-declare-struct-type name-and-fields ...)
  "(declare-struct-type name (field/type ...):fields name/fields ...)"
  (pair (q begin)
    (map-slice 2
      (l (name fields)
        (qq (declare (unquote name) (type (struct (unquote-splicing (map-slice 2 list fields)))))))
      name-and-fields)))

(sc-define-syntax (sp-include* rate) (begin (pre-include "sph-sp.h") (pre-define _sp-rate rate)))

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
  (sp-define* (name _event) sp-event-t*
    (define _duration sp-time-t (- _event:end _event:start)) body
    ... (if _event:prepare (status-require (_event:prepare _event)))))

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

(sc-define-syntax (sp-group-add* group event) (srq (sp-group-add group event)))
(sc-define-syntax (sp-render-file* event) (srq (sp-render event 0)))
(sc-define-syntax (sp-render-plot* event) (srq (sp-render event 1)))