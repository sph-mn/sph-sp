(declare
  sp-render-config-t
  (type (struct (channel-count sp-channel-count-t) (rate sp-time-t) (block-size sp-time-t)))
  (sp-render-config channel-count rate block-size)
  (sp-render-config-t sp-channel-count-t sp-time-t sp-time-t)
  (sp-render-file event start end config path)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t uint8-t*)
  (sp-render-block event start end config out)
  (status-t sp-event-t sp-time-t sp-time-t sp-render-config-t sp-block-t*)
  (sp-render event file-or-plot) (status-t sp-event-t uint8-t))