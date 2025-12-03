(declare
  sp-render-config-t
  (type
    (struct
      (channel-count sp-channel-count-t)
      (rate sp-time-t)
      (block-size sp-time-t)
      (display-progress sp-bool-t)))
  (sp-render-config channel-count rate block-size display-progress)
  (sp-render-config-t sp-channel-count-t sp-time-t sp-time-t sp-bool-t)
  (sp-render-range-file event start end config path)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t char*)
  (sp-render-range-block event start end config out)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t sp-block-t*)
  (sp-render-file event path) (status-t sp-event-t char*)
  (sp-render-plot event) (status-t sp-event-t))
