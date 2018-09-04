(pre-define (scm-sp-port a) (convert-type (scm-sp-object-data a) sp-port-t*))

(define (scm-sp-port? a) (SCM SCM)
  (return
    (scm-from-bool
      (and (SCM-SMOB-PREDICATE scm-type-sp-object a) (= sp-object-type-port (scm-sp-object-type a))))))

(define (scm-sp-port-channel-count port) (SCM SCM)
  (return (scm-from-uint32 (: (scm-sp-port port) channel-count))))

(define (scm-sp-port-sample-rate port) (SCM SCM)
  (return (scm-from-uint32 (: (scm-sp-port port) sample-rate))))

(define (scm-sp-port-position? port) (SCM SCM)
  (return (scm-from-bool (bit-and sp-port-bit-position (: (scm-sp-port port) flags)))))

(define (scm-sp-port-input? port) (SCM SCM)
  (return (scm-from-bool (bit-and sp-port-bit-input (: (scm-sp-port port) flags)))))

(define (scm-sp-port-position port) (SCM SCM)
  "returns the current port position in number of octets"
  (declare position size-t)
  (sp-port-position &position (scm-sp-port port))
  (return (scm-from-size-t position)))

(define (scm-sp-port-close a) (SCM SCM)
  status-declare
  (set status (sp-port-close (scm-sp-port a)))
  (status->scm-return SCM-UNSPECIFIED))

(define (scm-sp-port-read scm-port scm-sample-count) (SCM SCM SCM)
  status-declare
  (declare
    port sp-port-t*
    sample-count uint32-t
    channel-count uint32-t
    data sp-sample-t**
    result SCM)
  (set
    port (scm-sp-port scm-port)
    sample-count (scm->uint32 scm-sample-count)
    channel-count (: port channel-count)
    data (sp-alloc-channel-array channel-count sample-count))
  (sp-status-require-alloc data)
  (status-require (sp-port-read data port sample-count))
  (set result (scm-take-channel-data data channel-count sample-count))
  (label exit
    (status->scm-return result)))

(define (scm-sp-port-write scm-port scm-channel-data scm-sample-count) (SCM SCM SCM SCM)
  status-declare
  (declare
    port sp-port-t*
    data sp-sample-t**
    channel-count uint32-t
    sample-count size-t)
  (local-memory-init 1)
  (set
    port (scm-sp-port scm-port)
    data (scm->channel-data scm-channel-data &channel-count &sample-count))
  (sp-status-require-alloc data)
  (local-memory-add data)
  (status-require (sp-port-write port sample-count data))
  (label exit
    local-memory-free
    (status->scm-return SCM-UNSPECIFIED)))

(define (scm-sp-port-set-position scm-port scm-sample-offset) (SCM SCM SCM)
  status-declare
  (set status (sp-port-set-position (scm-sp-port scm-port) (scm->uint64 scm-sample-offset)))
  (status->scm-return SCM-UNSPECIFIED))

(define (scm-sp-file-open scm-path scm-channel-count scm-sample-rate) (SCM SCM SCM SCM)
  status-declare
  (declare
    path uint8-t*
    channel-count uint32-t
    sample-rate uint32-t
    result SCM)
  (set
    path (scm->locale-string scm-path)
    channel-count (scm->uint32 scm-channel-count)
    sample-rate (scm->uint32 scm-sample-rate))
  (sp-alloc-define sp-port sp-port-t* (sizeof sp-port-t))
  (status-require (sp-file-open sp-port path channel-count sample-rate))
  (set result (scm-sp-object-create sp-port sp-object-type-port))
  (label exit
    (status->scm-return result)))

(define (scm-sp-alsa-open scm-device-name scm-input? scm-channel-count scm-sample-rate scm-latency)
  (SCM SCM SCM SCM SCM SCM)
  status-declare
  (declare
    device-name uint8-t*
    input? boolean
    channel-count uint32-t
    sample-rate uint32-t
    latency uint32-t
    result SCM)
  (set
    device-name (scm->locale-string scm-device-name)
    input? (scm->bool scm-input?)
    channel-count (scm->uint32 scm-channel-count)
    sample-rate (scm->uint32 scm-sample-rate)
    latency (scm->uint32 scm-latency))
  (sp-alloc-define-zero sp-port sp-port-t* (sizeof sp-port-t))
  (status-require (sp-alsa-open sp-port device-name input? channel-count sample-rate latency))
  (set result (scm-sp-object-create sp-port sp-object-type-port))
  (label exit
    (status->scm-return result)))

(define (init-sp-io) void
  scm-c-define-procedure-c-init
  (scm-c-define-procedure-c "sp-port-close" 1 0 0 scm-sp-port-close "sp-port -> boolean")
  (scm-c-define-procedure-c "sp-port-input?" 1 0 0 scm-sp-port-input? "sp-port -> boolean")
  (scm-c-define-procedure-c "sp-port-position?" 1 0 0 scm-sp-port-position? "sp-port -> boolean")
  (scm-c-define-procedure-c
    "sp-port-position" 1 0 0 scm-sp-port-position "sp-port -> integer/boolean")
  (scm-c-define-procedure-c
    "sp-port-channel-count" 1 0 0 scm-sp-port-channel-count "sp-port -> integer")
  (scm-c-define-procedure-c
    "sp-port-sample-rate" 1 0 0 scm-sp-port-sample-rate "sp-port -> integer/boolean")
  (scm-c-define-procedure-c "sp-port?" 1 0 0 scm-sp-port? "sp-port -> boolean")
  (scm-c-define-procedure-c
    "sp-alsa-open"
    5 0 0 scm-sp-alsa-open "device-name input? channel-count sample-rate latency -> sp-port")
  (scm-c-define-procedure-c
    "sp-file-open" 3 0 0 scm-sp-file-open "path channel-count sample-rate -> sp-port")
  (scm-c-define-procedure-c
    "sp-port-write"
    2
    1
    0
    scm-sp-port-write
    "sp-port (f32vector ...):channel-data [integer:sample-count] -> boolean
    write sample data to the channels of port")
  (scm-c-define-procedure-c
    "sp-port-read"
    2 0 0 scm-sp-port-read "sp-port integer:sample-count -> (f32vector ...):channel-data")
  (scm-c-define-procedure-c
    "sp-port-set-position"
    2
    0
    0
    scm-sp-port-set-position
    "sp-port integer:sample-offset -> boolean
    sample-offset can be negative, in which case it is from the end of the port"))