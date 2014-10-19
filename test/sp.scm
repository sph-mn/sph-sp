(import (sph) (sph test) (sph sp))

(define* (test-fail-if-error title input-result #:optional c)
  (if (error? input-result) (test-fail title input-result) (if c (c input-result) input-result)))

(define (test-file)
  (delete-file "/tmp/sp-test")
  (let (file (sp-io-file-open-output "/tmp/sp-test" 3))
    (test-fail-if-error "open" file
      (l (file)
        (assert-and
          (assert-equal (list #t #f 0 #t 2)
            (list (sp-io-port? file) (sp-io-port-input? file)
              (sp-io-port-position file) (sp-io-file-set-position file 2)
              (sp-io-port-position file)))
          (debug-log (sp-io-file-write file (list (make-f32vector 8000 1) (make-f32vector 8000 1) (make-f32vector 8000 1))))
          (test-fail-if-error "close" (sp-io-port-close file)))))))

(execute-tests (ql file))