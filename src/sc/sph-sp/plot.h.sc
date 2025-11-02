(pre-define (sp-block-plot-1 a) (sp-plot-samples (array-get a.samples 0) a.size))

(declare
  (sp-plot-samples a a-size) (void sp-sample-t* sp-time-t)
  (sp-plot-times a a-size) (void sp-time-t* sp-time-t)
  (sp-plot-samples->file a a-size path) (void sp-sample-t* sp-time-t char*)
  (sp-plot-times->file a a-size path) (void sp-time-t* sp-time-t char*)
  (sp-plot-samples-file path use-steps) (void char* uint8-t)
  (sp-plot-spectrum->file a a-size path) (void sp-sample-t* sp-time-t char*)
  (sp-plot-spectrum-file path) (void char*)
  (sp-plot-spectrum a a-size) (void sp-sample-t* sp-time-t))
