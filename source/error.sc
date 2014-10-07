;error handling is done with scm-exceptions and c-integers for error-numbers.
(define error-number-type-check 0)

(define-macro (sp-exception-arguments symbol string)
  (scm-list-2 (scm-from-locale-symbol symbol) (scm-from-locale-string string)))

(define (sp-exception-from-number a) (SCM b32)
  (scm-apply-0 scm-throw
    (case* ((= n 0) (sp-exception-arguments "type-check" "wrong type for argument")))))

(define-macro (sp-exception key text)
  (scm-throw (scm-from-locale-symbol key) (scm-list-1 (scm-from-locale-string text))))

(define-macro scm-typechecks #t)

(pre-if scm-typechecks
  (define-macro (if-typecheck expr)
    (if (not expr) (sp-exception-from-number error-number-type-check)))
  (define-macro (if-typecheck expr) null))