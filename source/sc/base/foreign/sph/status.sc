(sc-comment
  "return status code and error handling. uses a local variable named \"status\" and a goto label named \"exit\".
   a status has an identifier and a group to discern between status identifiers of different libraries.
   status id 0 is success, everything else can be considered a failure or special case.
   status ids are 32 bit signed integers for compatibility with error return codes from many other existing libraries.
   bindings with a ! suffix update the status from an expression")

(define-type status-i-t b32_s)

(define-type status-t
  (struct
    (id status-i-t)
    (group b8)))

(pre-define
  status-id-success 0
  status-group-undefined 0
  status-init (define status status-t (struct-literal status-id-success status-group-undefined)))

(pre-define (status-init-group group)
  "like status init but sets a default group"
  (define status status-t (struct-literal status-id-success group)))

(pre-define status-reset (status-set-both status-group-undefined status-id-success))
(pre-define status-success? (equal? status-id-success (struct-get status id)))
(pre-define status-failure? (not status-success?))
(pre-define status-goto (goto exit))
(pre-define (status-set-group group-id) (struct-set status group group-id))
(pre-define (status-set-id status-id) (struct-set status id status-id))

(pre-define (status-set-both group-id status-id)
  (status-set-group group-id)
  (status-set-id status-id))

(pre-define status-require (if status-failure? status-goto))

(pre-define (status-require! expression)
  "update status with the result of expression, check for failure and goto error if so"
  (set status expression)
  (if status-failure? status-goto))

(pre-define (status-set-id-goto status-id)
  "set the status id and goto error"
  (status-set-id status-id)
  status-goto)

(pre-define (status-set-group-goto group-id)
  (status-set-group group-id)
  status-goto)

(pre-define (status-set-both-goto group-id status-id)
  (status-set-both group-id status-id)
  status-goto)

(pre-define (status-id-is? status-id) (equal? status-id (struct-get status id)))

(pre-define (status-i-require! expression)
  "update status with the result of expression, check for failure and goto error if so"
  (set status.id expression)
  (if status-failure? status-goto))
