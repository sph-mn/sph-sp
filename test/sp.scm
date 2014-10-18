(import (sph) (sph test) (sph sp))

(define* (test-fail-if-error title input-result #:optional c)
  (if (error? input-result) (test-fail title input-result) (if c (c input-result) input-result)))

(define (test-file)
  (let (file (sp-io-file-open-output (tmpnam)))
    (test-fail-if-error "open" file
      (l (file) (sp-io-port-input? file)
        (debug-log (sp-io-port? file) (sp-io-port-position file) (sp-io-port-input? file))
        (test-fail-if-error "close" (sp-io-port-close file))))))

(execute-tests (ql file))