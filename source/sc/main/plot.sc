(define sp-plot-temp-file-index uint32-t 0)

(pre-define
  sp-plot-temp-path "/tmp/sp-plot"
  sp-plot-temp-file-index-maxlength 10
  sp-plot-command-pattern-lines
  "gnuplot --persist -e 'set key off; set size ratio 0.5; plot \"%s\" with lines lc rgb \"blue\"'"
  sp-plot-command-pattern-steps
  "gnuplot --persist -e 'set key off; set size ratio 0.5; plot \"%s\" with histeps lc rgb \"blue\"'")

(define (sp-plot-sample-array->file a a-size path) (void sp-sample-t* sp-time-t uint8-t*)
  (declare file FILE* i sp-time-t)
  (set file (fopen path "w"))
  (for ((set i 0) (< i a-size) (set i (+ 1 i))) (fprintf file "%.3f\n" (array-get a i)))
  (fclose file))

(define (sp-plot-time-array->file a a-size path) (void sp-time-t* sp-time-t uint8-t*)
  (declare file FILE* i sp-time-t)
  (set file (fopen path "w"))
  (for ((set i 0) (< i a-size) (set i (+ 1 i))) (fprintf file "%lu\n" (array-get a i)))
  (fclose file))

(define (sp-plot-sample-array-file path use-steps) (void uint8-t* uint8-t)
  (declare command uint8-t* command-pattern uint8-t* command-size size-t)
  (set
    command-pattern (if* use-steps sp-plot-command-pattern-steps sp-plot-command-pattern-lines)
    command-size (+ (strlen path) (strlen command-pattern))
    command (malloc command-size))
  (if (not command) return)
  (snprintf command command-size command-pattern path)
  (system command)
  (free command))

(define (sp-plot-sample-array a a-size) (void sp-sample-t* sp-time-t)
  (define path-size uint8-t (+ 1 sp-plot-temp-file-index-maxlength (strlen sp-plot-temp-path)))
  (define path uint8-t* (calloc path-size 1))
  (if (not path) return)
  (snprintf path path-size "%s-%lu" sp-plot-temp-path sp-plot-temp-file-index)
  (set sp-plot-temp-file-index (+ 1 sp-plot-temp-file-index))
  (sp-plot-sample-array->file a a-size path)
  (sp-plot-sample-array-file path #t)
  (free path))

(define (sp-plot-time-array a a-size) (void sp-time-t* sp-time-t)
  (define path-size uint8-t (+ 1 sp-plot-temp-file-index-maxlength (strlen sp-plot-temp-path)))
  (define path uint8-t* (calloc path-size 1))
  (if (not path) return)
  (snprintf path path-size "%s-%lu" sp-plot-temp-path sp-plot-temp-file-index)
  (set sp-plot-temp-file-index (+ 1 sp-plot-temp-file-index))
  (sp-plot-time-array->file a a-size path)
  (sp-plot-sample-array-file path #t)
  (free path))

(define (sp-plot-spectrum->file a a-size path) (void sp-sample-t* sp-time-t uint8-t*)
  "take the fft for given samples, convert complex values to magnitudes and write plot data to file"
  (declare file FILE* i sp-time-t imag double* real double*)
  (set imag (calloc a-size (sizeof sp-sample-t)))
  (if (not imag) return)
  (set real (malloc (* (sizeof sp-sample-t) a-size)))
  (if (not real) return)
  (memcpy real a (* (sizeof sp-sample-t) a-size))
  (if (sp-fft a-size real imag) return)
  (set file (fopen path "w"))
  (for ((set i 0) (< i a-size) (set i (+ 1 i)))
    (fprintf file "%.3f\n"
      (* 2
        (/
          (sqrt
            (+ (* (array-get real i) (array-get real i)) (* (array-get imag i) (array-get imag i))))
          a-size))))
  (fclose file)
  (free imag)
  (free real))

(define (sp-plot-spectrum-file path) (void uint8-t*) (sp-plot-sample-array-file path #t))

(define (sp-plot-spectrum a a-size) (void sp-sample-t* sp-time-t)
  (define path-size uint8-t (+ 1 sp-plot-temp-file-index-maxlength (strlen sp-plot-temp-path)))
  (define path uint8-t* (calloc path-size 1))
  (if (not path) return)
  (snprintf path path-size "%s-%lu" sp-plot-temp-path sp-plot-temp-file-index)
  (set sp-plot-temp-file-index (+ 1 sp-plot-temp-file-index))
  (sp-plot-spectrum->file a a-size path)
  (sp-plot-spectrum-file path)
  (free path))