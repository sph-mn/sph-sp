;error handling is done with scm-exceptions and c-integers for error-numbers.
(define-macro error-type-check 0)
(define-macro error-no-memory 1)

(define-macro (sp-error-values symbol string)
  (scm-values SCM-BOOL-F (scm-from-locale-symbol symbol) (scm-from-locale-string string)))

(define (sp-scm-error key description) (scm-values key description))

(define (sp-scm-error-from-number a) (SCM b32)
  (case ((= n sp-error-type-check) (sp-error-values "type-check" "wrong type for argument"))
    ((= n sp-error-no-memory) (sp-error-values "memory" "could not allocate memory"))))

(define-macro (sp-scm-return-error-from-number a)
  (return (sp-scm-error-from-number a)))

(define-macro (sp-scm-return-error k d)
  (return (sp-scm-error k d)))

(define-macro scm-typechecks #t)

(pre-if scm-typechecks
  (define-macro (if-typecheck expr)
    (if (not expr) (return (sp-scm-error-from-number error-number-type-check))))
  (define-macro (if-typecheck expr) null))