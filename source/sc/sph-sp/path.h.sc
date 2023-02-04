(pre-define
  sp-path-t spline-path-t
  sp-path-value-t spline-path-value-t
  sp-path-point-t spline-path-point-t
  sp-path-segment-t spline-path-segment-t
  sp-path-segment-count-t spline-path-segment-count-t
  sp-path-line spline-path-line
  sp-path-move spline-path-move
  sp-path-bezier spline-path-bezier
  sp-path-bezier-arc spline-path-bezier-arc
  sp-path-constant spline-path-constant
  sp-path-path spline-path-path
  sp-path-prepare-segments spline-path-prepare-segments
  sp-path-i-line spline-path-i-line
  sp-path-i-move spline-path-i-move
  sp-path-i-bezier spline-path-i-bezier
  sp-path-i-bezier-arc spline-path-i-bezier-arc
  sp-path-i-constant spline-path-i-constant
  sp-path-i-path spline-path-i-path
  sp-path-end spline-path-end
  sp-path-size spline-path-size
  sp-path-free spline-path-free
  sp-path-get spline-path-get
  sp-path-set spline-path-set
  (sp-path-times-constant out size value)
  (sp-path-times-2 out size (sp-path-move 0 value) (sp-path-constant))
  (sp-path-samples-constant out size value)
  (sp-path-samples-2 out size (sp-path-move 0 value) (sp-path-constant))
  (sp-path-curves-config-declare name _segment-count)
  (begin
    (declare
      name sp-path-curves-config-t
      (pre-concat name _x) (array sp-path-value-t _segment-count)
      (pre-concat name _y) (array sp-path-value-t _segment-count)
      (pre-concat name _c) (array sp-path-value-t _segment-count))
    (struct-set name
      segment-count _segment-count
      x (pre-concat name _x)
      y (pre-concat name _y)
      c (pre-concat name _c))))

(declare
  sp-path-curves-config-t
  (type
    (struct
      (segment-count sp-time-t)
      (x sp-path-value-t*)
      (y sp-path-value-t*)
      (c sp-path-value-t*)))
  (sp-path-samples-new path size out) (status-t sp-path-t sp-time-t sp-sample-t**)
  (sp-path-samples-1 out size s1) (status-t sp-sample-t** sp-time-t sp-path-segment-t)
  (sp-path-samples-2 out size s1 s2)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t)
  (sp-path-samples-3 out size s1 s2 s3)
  (status-t sp-sample-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-samples-4 out size s1 s2 s3 s4)
  (status-t sp-sample-t** sp-time-t
    sp-path-segment-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-times-new path size out) (status-t sp-path-t sp-time-t sp-time-t**)
  (sp-path-times-1 out size s1) (status-t sp-time-t** sp-time-t sp-path-segment-t)
  (sp-path-times-2 out size s1 s2)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t)
  (sp-path-times-3 out size s1 s2 s3)
  (status-t sp-time-t** sp-time-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-times-4 out size s1 s2 s3 s4)
  (status-t sp-time-t** sp-time-t
    sp-path-segment-t sp-path-segment-t sp-path-segment-t sp-path-segment-t)
  (sp-path-derivation path x-changes y-changes index out)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-path-t*)
  (sp-path-samples-derivation path x-changes y-changes index out out-size)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-sample-t** sp-time-t*)
  (sp-path-times-derivation path x-changes y-changes index out out-size)
  (status-t sp-path-t sp-sample-t** sp-sample-t** sp-time-t sp-time-t** sp-time-t*)
  (sp-path-multiply path x-factor y-factor) (void sp-path-t sp-sample-t sp-sample-t)
  (sp-path-derivations-normalized base count x-changes y-changes out)
  (status-t sp-path-t sp-time-t sp-sample-t** sp-sample-t** sp-path-t**)
  (sp-path-samples-derivations-normalized path count x-changes y-changes out out-sizes)
  (status-t sp-path-t sp-time-t sp-sample-t** sp-sample-t** sp-sample-t*** sp-time-t**)
  (sp-path-curves-config-new segment-count out) (status-t sp-time-t sp-path-curves-config-t*)
  (sp-path-curves-config-free a) (void sp-path-curves-config-t)
  (sp-path-curves-new config out) (status-t sp-path-curves-config-t sp-path-t*)
  (sp-path-curves-times-new config length out)
  (status-t sp-path-curves-config-t sp-time-t sp-time-t**)
  (sp-path-curves-samples-new config length out)
  (status-t sp-path-curves-config-t sp-time-t sp-sample-t**))