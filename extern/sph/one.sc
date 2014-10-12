(includep "string.h")

(define-macro string-length strlen
  string-length-n strnlen
  string-copy strcpy
  string-copy-n strncpy
  string-concat strcat
  string-concat-n strncat
  string-index strchr
  string-index-right strrchr
  string-index-string strstr
  string-duplicate strdup
  string-duplicate-n strndup
  string-index-ci strcasestr
  string-span strcspn string-break strpbrk string-compare strcmp memory-copy memcpy)

(define-macro (file-exists? path) (not (equal? (access path F-OK) -1)))

(define (ensure-trailing-slash str) (char* char*)
  (define str-len b8 (string-length str))
  (if (or (not str-len) (equal? #\/ (deref (+ str (- str-len 1))))) (return str)
    (begin (define new-str char* (malloc (+ 2 str-len))) (memory-copy new-str str str-len)
      (memory-copy (+ new-str str-len) "/" 1) (set (deref new-str (+ 1 str-len)) 0) (return new-str))))

(define-macro (array-contains-s array-start array-end search-value index-temp res)
  (set index-temp array-start) (set res #f)
  (while (<= index-temp array-end)
    (if (= (deref index-temp) search-value) (begin (set res #t) break)) (increment-one index-temp)))

(define-macro (require-goto a label) (if (not a) (goto label)))

(pre-if stability-typechecks
  (define-macro (if-typecheck expr action)
    (if (not expr)
      (begin
        (debug-log "type check failed %s"
          (if* (< (string-length (pre-stringify expr)) 24) (pre-stringify expr) ""))
        action)))
  (define-macro (if-typecheck expr action) null))

(define-macro (octet-write-string-binary target a)
  (sprintf target "%d%d%d%d%d%d%d%d"
    (if* (bit-and a 128) 1 0) (if* (bit-and a 64) 1 0)
    (if* (bit-and a 32) 1 0) (if* (bit-and a 16) 1 0)
    (if* (bit-and a 8) 1 0) (if* (bit-and a 4) 1 0) (if* (bit-and a 2) 1 0) (if* (bit-and a 1) 1 0)))