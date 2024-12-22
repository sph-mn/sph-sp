(pre-define
  sp-filter-state-t sp-convolution-filter-state-t
  sp-filter-state-free sp-convolution-filter-state-free)

(declare
  sp-convolution-filter-ir-f-t (type (function-pointer status-t void* sp-sample-t** sp-time-t*))
  sp-convolution-filter-state-t
  (type
    (struct
      (carryover sp-sample-t*)
      (carryover-len sp-time-t)
      (carryover-alloc-len sp-time-t)
      (ir sp-sample-t*)
      (ir-f sp-convolution-filter-ir-f-t)
      (ir-f-arguments void*)
      (ir-f-arguments-len uint8-t)
      (ir-len sp-time-t)))
  sp-state-variable-filter-t
  (type
    (function-pointer void sp-sample-t* sp-sample-t* sp-sample-t sp-sample-t sp-time-t sp-sample-t*))
  (sp-moving-average in in-size prev next radius out)
  (void sp-sample-t* sp-time-t sp-sample-t* sp-sample-t* sp-time-t sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir-length transition) (sp-time-t sp-sample-t)
  (sp-windowed-sinc-ir cutoff transition result-len result-ir)
  (status-t sp-sample-t sp-sample-t sp-time-t* sp-sample-t**)
  (sp-convolution-filter-state-free state) (void sp-convolution-filter-state-t*)
  (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-bool-t sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-bp-br-ir cutoff-l cutoff-h transition-l transition-h is-reject out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-bool-t sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-lp-hp-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-bp-br-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*)
  (sp-windowed-sinc-lp-hp in in-len cutoff transition is-high-pass out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-sample-t sp-sample-t sp-bool-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-bp-br in in-len
    cutoff-l cutoff-h transition-l transition-h is-reject out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-bool-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-windowed-sinc-lp-hp-ir cutoff transition is-high-pass out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-bool-t sp-sample-t** sp-time-t*))