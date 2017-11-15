(define-type sp-port-t
  ; generic input/output port handle.
  ; type: any of sp-port-type-* value
  ; position?: true if the port supports random access
  ; position-offset: header length
  (struct
    (type b8)
    (flags b8)
    (sample-rate b32)
    (channel-count b32)
    (data b0*)))

(pre-define
  sp-port-type-alsa 0
  sp-port-type-file 1
  sp-port-bit-input 1
  sp-port-bit-output 2
  sp-port-bit-position 4
  sp-port-bit-closed 8)

(define (sp-port-read result port sample-count) (status-t sp-sample-t** sp-port-t* b32))
(define (sp-port-write port sample-count channel-data) (status-t sp-port-t* size-t sp-sample-t**))
(define (sp-port-position result port) (status-t size-t* sp-port-t*))
(define (sp-port-set-position port sample-index) (status-t sp-port-t* size-t))
(define (sp-file-open result path channel-count sample-rate) (status-t sp-port-t* b8* b32 b32))

(define (sp-alsa-open result device-name input? channel-count sample-rate latency)
  (status-t sp-port-t* b8* boolean b32 b32 b32-s))

(define (sp-port-close a) (status-t sp-port-t*))
