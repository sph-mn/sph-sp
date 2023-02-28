(declare
  (sp-sequence-max size min-size) (sp-time-t sp-time-t sp-time-t)
  (sp-set-sequence-max set-size selection-size) (sp-time-t sp-time-t sp-time-t)
  (sp-permutations-max set-size selection-size) (sp-time-t sp-time-t sp-time-t)
  (sp-compositions-max sum) (sp-time-t sp-time-t)
  sp-stat-times-f-t (type (function-pointer uint8-t sp-time-t* sp-time-t sp-sample-t*))
  sp-stat-samples-f-t (type (function-pointer uint8-t sp-sample-t* sp-time-t sp-sample-t*))
  (sp-stat-times-range a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-mean a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-deviation a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-median a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-center a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-inharmonicity a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-kurtosis a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-times-skewness a size out) (uint8-t sp-time-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-mean a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-deviation a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-inharmonicity a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-median a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-center a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-range a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-kurtosis a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-stat-samples-skewness a size out) (uint8-t sp-sample-t* sp-time-t sp-sample-t*)
  (sp-samples-scale->times a size max out) (void sp-sample-t* sp-time-t sp-time-t sp-time-t*)
  (sp-stat-unique-max size width) (sp-time-t sp-time-t sp-time-t)
  (sp-stat-unique-all-max size) (sp-time-t sp-time-t)
  (sp-stat-repetition-all-max size) (sp-time-t sp-time-t)
  (sp-stat-repetition-max size width) (sp-time-t sp-time-t sp-time-t))