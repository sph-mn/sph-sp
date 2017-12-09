(sc-comment "return status handling")

(enum
  (sp-status-group-alsa
    sp-status-group-libc
    sp-status-group-sndfile
    sp-status-group-sp
    sp-status-id-file-channel-mismatch
    sp-status-id-file-encoding
    sp-status-id-file-header
    sp-status-id-file-incompatible
    sp-status-id-file-incomplete
    sp-status-id-eof
    sp-status-id-input-type
    sp-status-id-memory
    sp-status-id-not-implemented
    sp-status-id-port-closed sp-status-id-port-position sp-status-id-port-type sp-status-id-undefined))

(pre-define sp-status-init (status-init-group sp-status-group-sp))

(pre-define (sp-system-status-require-id id)
  (if (< id 0) (status-set-both-goto sp-status-group-libc id)))

(pre-define (sp-system-status-require! expression)
  (status-set-id expression)
  (if (< status.id 0) (status-set-group-goto sp-status-group-libc) status-reset))

(pre-define (sp-alsa-status-require! expression)
  (status-set-id expression)
  (if status-failure? (status-set-group-goto sp-status-group-alsa)))

(pre-define (sp-status-require-alloc a)
  (if (not a) (status-set-both-goto sp-status-group-sp sp-status-id-memory)))
