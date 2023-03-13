(pre-define sp-path-point-count-t spline-path-segment-count-t sp-path-point-count-limit 16)

(declare
  (sp-path-samples out length point-count x y c)
  (status-t sp-sample-t** sp-time-t sp-path-point-count-t sp-sample-t* sp-sample-t* sp-sample-t*)
  (sp-path-samples2 out length y1 y2) (status-t sp-sample-t** sp-time-t sp-sample-t sp-sample-t)
  (sp-path-samples3 out length x1 y1 y2 y3)
  (status-t sp-sample-t** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-samples4 out length x1 x2 y1 y2 y3 y4)
  (status-t sp-sample-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-samples5 out length x1 x2 x3 y1 y2 y3 y4 y5)
  (status-t sp-sample-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-samples-c2 out length y1 y2 c1)
  (status-t sp-sample-t** sp-time-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-samples-c3 out length x1 y1 y2 y3 c1 c2)
  (status-t sp-sample-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-samples-c4 out length x1 x2 y1 y2 y3 y4 c1 c2 c3)
  (status-t sp-sample-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-samples-c5 out length x1 x2 x3 y1 y2 y3 y4 y5 c1 c2 c3 c4)
  (status-t sp-sample-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-times out length point-count x y c)
  (status-t sp-time-t** sp-time-t sp-path-point-count-t sp-sample-t* sp-sample-t* sp-sample-t*)
  (sp-path-times2 out length y1 y2) (status-t sp-time-t** sp-time-t sp-sample-t sp-sample-t)
  (sp-path-times3 out length x1 y1 y2 y3)
  (status-t sp-time-t** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-times4 out length x1 x2 y1 y2 y3 y4)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-times5 out length x1 x2 x3 y1 y2 y3 y4 y5)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-times-c2 out length y1 y2 c1)
  (status-t sp-time-t** sp-time-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-times-c3 out length x1 y1 y2 y3 c1 c2)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-times-c4 out length x1 x2 y1 y2 y3 y4 c1 c2 c3)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-path-times-c5 out length x1 x2 x3 y1 y2 y3 y4 y5 c1 c2 c3 c4)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-zero out length point-count x y c)
  (status-t sp-sample-t** sp-time-t sp-path-point-count-t sp-sample-t* sp-sample-t* sp-sample-t*)
  (sp-envelope-zero3 out length x1 y1) (status-t sp-sample-t** sp-time-t sp-sample-t sp-sample-t)
  (sp-envelope-zero4 out length x1 x2 y1 y2)
  (status-t sp-sample-t** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-zero5 out length x1 x2 x3 y1 y2 y3)
  (status-t sp-sample-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-zero-c3 out length x1 y1 c1 c2)
  (status-t sp-sample-t** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-zero-c4 out length x1 x2 y1 y2 c1 c2 c3)
  (status-t sp-sample-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-zero-c5 out length x1 x2 x3 y1 y2 y3 c1 c2 c3 c4)
  (status-t sp-sample-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-scaled out length point-count y-scalar x y c)
  (status-t sp-time-t** sp-time-t
    sp-path-point-count-t sp-sample-t sp-sample-t* sp-sample-t* sp-sample-t*)
  (sp-envelope-scaled3 out length y-scalar x1 y1 y2 y3)
  (status-t sp-time-t** sp-time-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-scaled4 out length y-scalar x1 x2 y1 y2 y3 y4)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-scaled5 out length y-scalar x1 x2 x3 y1 y2 y3 y4 y5)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-scaled-c3 out length y-scalar x1 y1 y2 y3 c1 c2)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-scaled-c4 out length y-scalar x1 x2 y1 y2 y3 y4 c1 c2 c3)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t)
  (sp-envelope-scaled-c5 out length y-scalar x1 x2 x3 y1 y2 y3 y4 y5 c1 c2 c3 c4)
  (status-t sp-time-t** sp-time-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t
    sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t sp-sample-t))