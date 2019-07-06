(sc-comment "return status as integer code with group identifier"
  "for exception handling with a local variable and a goto label"
  "status id 0 is success, everything else can be considered a special case or failure"
  "status ids are signed integers for compatibility with error return codes from other existing libraries"
  "group ids are strings used to categorise sets of errors codes from different libraries for example")

(declare s-t (type (struct (id int) (group uint8-t*))))

(pre-define
  s-success 0
  s-group-undefined (convert-type "" uint8-t*)
  s-declare (define s-current s-t (struct-literal s-success s-group-undefined))
  s-is-success (= s-success s-current.id)
  s-is-failure (not s-is-success)
  s-return (return s-current)
  (s-set group-id status-id)
  (set s-current.group (convert-type group-id uint8-t*) s-current.id status-id)
  (s-set-goto group-id status-id) (begin (s-set group-id status-id) (goto exit))
  (s expression) (begin (set s-current expression) (if s-is-failure (goto exit)))
  (si expression) (begin (set s-current.id expression) (if s-is-failure (goto exit))))