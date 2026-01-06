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
  (sp-convolution-filter-state-set ir-f ir-f-arguments ir-f-arguments-len out-state)
  (status-t sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t**)
  (sp-convolution-filter in in-len ir-f ir-f-arguments ir-f-arguments-len out-state out-samples)
  (status-t sp-sample-t* sp-time-t
    sp-convolution-filter-ir-f-t void* uint8-t sp-convolution-filter-state-t** sp-sample-t*)
  (sp-convolution-filter-state-uninit state) (void sp-convolution-filter-state-t*)
  (sp-bessel-i0 value) (sp-sample-t sp-sample-t)
  (sp-kaiser-window sample-index window-length beta-value)
  (sp-sample-t sp-time-t sp-time-t sp-sample-t)
  (sp-kaiser-window-length transition-width beta-value) (sp-time-t sp-sample-t sp-sample-t)
  (sp-force-center-unity impulse-response sample-count) (void sp-sample-t* sp-time-t)
  (sp-sinc-make-minimum-phase impulse-response sample-count) (status-t sp-sample-t* sp-time-t)
  (sp-resonator-ir cutoff-low cutoff-high transition out-ir out-len)
  (status-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t** sp-time-t*)
  (sp-resonator-ir-f arguments out-ir out-len) (status-t void* sp-sample-t** sp-time-t*))
