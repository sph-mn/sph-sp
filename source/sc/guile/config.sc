(pre-cond
  (sp-sample-format-f64
    (pre-define
      (scm-from-sp-sample a) (scm-from-double a)
      (scm->sp-sample a) (scm->double a)
      (scm-c-make-sp-samples len)
      (scm-make-f64vector (scm-from-sp-sample-count len) (scm-from-uint8 0))
      (scm-c-take-samples a len) (scm-take-f64vector a len)))
  (sp-sample-format-f32
    (pre-define
      (scm-from-sp-sample a) (scm-from-double (convert-type a f32))
      (scm->sp-sample a) (convert-type (scm->double a) f32)
      (scm-c-make-sp-samples len)
      (scm-make-f32vector (scm-from-sp-sample-count len) (scm-from-uint8 0))
      (scm-c-take-samples a len) (scm-take-f32vector a len)))
  (sp-sample-format-int32
    (pre-define
      (scm-from-sp-sample a) (scm-from-int32 a)
      (scm->sp-sample a) (scm->int32 a)
      (scm-c-make-sp-samples len)
      (scm-make-s32vector (scm-from-sp-sample-count len) (scm-from-uint8 0))
      (scm-c-take-samples a len) (scm-take-s32vector a len)))
  (sp-sample-format-int16
    (pre-define
      (scm-from-sp-sample a) (scm-from-int16 a)
      (scm->sp-sample a) (scm->int16 a)
      (scm-c-make-sp-samples len)
      (scm-make-s16vector (scm-from-sp-sample-count len) (scm-from-uint8 0))
      (scm-c-take-samples a len) (scm-take-s16vector a len)))
  (sp-sample-format-int8
    (pre-define
      (scm-from-sp-sample a) (scm-from-int8 a)
      (scm->sp-sample a) (scm->int8 a)
      (scm-c-make-sp-samples len)
      (scm-make-s8vector (scm-from-sp-sample-count len) (scm-from-uint8 0))
      (scm-c-take-samples a len) (scm-take-s8vector a len))))

(pre-define
  (scm-from-sp-channel-count a) (scm-from-uint32 a)
  (scm-from-sp-sample-rate a) (scm-from-uint32 a)
  (scm-from-sp-sample-count a) (scm-from-size-t a)
  (scm-from-sp-float a) (scm-from-double a)
  (scm->sp-channel-count a) (scm->uint32 a)
  (scm->sp-sample-rate a) (scm->uint32 a)
  (scm->sp-sample-count a) (scm->size-t a)
  (scm->sp-float a) (scm->double a))