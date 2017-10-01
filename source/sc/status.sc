; return status handling

(enum
  (sp-status-id-undefined sp-status-id-input-type sp-status-id-not-implemented
    sp-status-id-memory sp-status-id-file-incompatible
    sp-status-id-file-encoding sp-status-id-file-header
    sp-status-id-port-closed sp-status-id-port-position
    sp-status-id-file-channel-mismatch sp-status-id-file-incomplete
    sp-status-id-port-type sp-status-group-sp sp-status-group-libc sp-status-group-alsa))

(define (sp-status-description a) (b8* status-t)
  (return
    (case* = a.group
      (sp-status-group-sp
        (convert-type
          (case* = a.id
            (sp-status-id-input-type "input argument is of wrong type")
            (sp-status-id-not-implemented "not implemented")
            (sp-status-id-memory "not enough memory or other memory allocation error")
            (sp-status-id-file-incompatible
              "file exists but channel count or sample rate is different from what was requested")
            (else ""))
          b8*))
      (sp-status-group-alsa (convert-type (snd-strerror a.id) b8*)) (else (convert-type "" b8*)))))

(define (sp-status-name a) (b8* status-t)
  (return
    (case* = a.group
      (sp-status-group-sp
        (convert-type
          (case* = a.id
            (sp-status-id-input-type "input-type") (sp-status-id-not-implemented "not-implemented")
            (sp-status-id-memory "memory") (else "unknown"))
          b8*))
      (sp-status-group-alsa (convert-type "alsa" b8*)) (else (convert-type "unknown" b8*)))))

(pre-define sp-status-init (status-init-group sp-status-group-sp))
(define scm-rnrs-raise SCM)

(pre-define (scm-c-error name description)
  (scm-call-1 scm-rnrs-raise
    (scm-list-3 (scm-from-latin1-symbol name)
      (scm-cons (scm-from-latin1-symbol "description") (scm-from-utf8-string description))
      (scm-cons (scm-from-latin1-symbol "c-routine") (scm-from-latin1-symbol __FUNCTION__)))))

(pre-define (status->scm-error a) (scm-c-error (sp-status-name a) (sp-status-description a)))
(pre-define (status->scm result) (if* status-success? result (status->scm-error status)))

(pre-define (status->scm-return result)
  "call scm-c-error if status is not success or return result" (return (status->scm result)))

(pre-define (sp-system-status-require-id id)
  (if (< id 0) (status-set-both-goto sp-status-group-libc id)))

(pre-define (sp-system-status-require! expression) (status-set-id expression)
  (if (< status.id 0) (status-set-group-goto sp-status-group-libc) status-reset))
