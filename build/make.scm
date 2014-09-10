#!/usr/bin/guile
!#

(import (sph))

(define optimise 3)
(define debugging-symbols #f)
(if (not (file-exists? "temp")) (mkdir "temp"))
(define source-files (list (list "source/" "main")))
(define format (not (null? (tail (program-arguments)))))

(each
  (l (path+filenames)
    (let (path (first path+filenames))
      (each
        (l (name) (system* "sc" (string-append path name ".sc") (string-append "temp/" name ".c"))
          (if format (system* "astyle" (string-append "temp/" name ".c"))))
        (tail path+filenames))))
  source-files)

(exit
  (system
    (string-append "gcc" (if debugging-symbols " -g" "")
      " -std=c11 -funsigned-char -Wall -Werror -Wfatal-errors -O" (number->string optimise)
      " -shared -fPIC $(guile-config compile)"
      "  $(guile-config link) -o temp/libguile-sp.so temp/main.c -Wl,--version-script=build/export && chmod 755 -R temp")))