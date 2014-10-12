(define-type port-alsa-data-t
  (struct (samples-per-second b32) (channel-count b32) (data snd-pcm-t*)))

(define scm_ptob_descriptor)

(define (scm-c-port-alsa-create type open-flags samples-per-second channel-count data)
  (SCM b8 b32-s b32 b32 pointer) (define port scm_t_port*)
  (SCM-PTAB-ENTRY port)
  (define port-data port-data-t* (scm-gc-malloc (sizeof port-data-t) "sp-port-data"))
  (struct-set (deref port-data) channel-count
    channel-count samples-per-second samples-per-second type type open-flags open-flags data data)
  (return (scm-new-smob scm-type-sp-port (convert-type port-data scm-t-bits))))

(define (scm-c-port-alsa-read port) (int SCM) (return))
(define (scm-c-port-alsa-write port data size) (b0 SCM b0* size-t))

(define (scm-c-port-alsa-print a output-port print-state) (int SCM SCM scm-print-state*)
  (define i port-data-t* (convert-type (SCM-SMOB-DATA a) port-data-t*)) (define r[255] char)
  (sprintf r "#<sp-port %lx type:%s samples-per-second:%d channel-count:%d closed?:%s>"
    (convert-type a pointer) (sp-port-type->name (struct-ref (deref i) type))
    (struct-ref (deref port-data) samples-per-second) (struct-ref (deref i) channel-count)
    (if* (struct-ref (deref i) closed?) "#t" "#f")
    (open-flags->string-rw (struct-ref (deref i) open-flags)))
  (scm-display (scm-from-locale-string r) output-port) (return 0))

(define (scm-c-type-port-alsa-init) scm-bits-t
  (define r scm-bits-t (scm-make-port-type "alsa" scm-c-port-alsa-read scm-c-port-alsa-write))
  (scm-set-port-print scm-c-port-alsa-print) (scm-set-port-close scm-c-port-alsa-close)
  (scm-set-port-flush scm-c-port-alsa-flush))