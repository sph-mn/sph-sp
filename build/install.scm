#!/usr/bin/guile
!#
(define scm-dir "/usr/share/guile/site/sph")
(if (not (file-exists? scm-dir)) (system* "mkdir" "-p" scm-dir))
(system* "chmod" "755" "-R" scm-dir)

(copy-file "temp/libguile-sp.so" "/usr/lib/libguile-sp.so")
(chmod "/usr/lib/libguile-sp.so" #o755)
(copy-file "source/sph/sp.scm" (string-append scm-dir "sp.scm"))
