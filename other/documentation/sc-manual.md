# sc manual
sc extends the [c api](c-manual.md) with new syntax that can save boilerplate code.

using the sc api requires the use of [sc](https://github.com/sph-mn/sph-sc) to compile your sc code.
compiling sc to c can be as simple as calling `sc input.sc output.c`. to compile a directory structure with .sc files to a directory structure with .c files, the following can be be used: `find -type f -name "*.sc" -exec sc --parents '{}' "target_dir" \;`.

refer to other/example.sc

# general
* sc-macro names end with * to avoid name conflicts with plain c functions

# setup
~~~
(sc-include-once "/usr/share/sph-sp/sc-macros")
~~~

* loads all sph-sp sc macros
* has to be called in each source file that uses macros

# bindings

## sp-init*
~~~
(sp-include* sample-rate)
~~~

* includes sph-sp.h
* defines the preprocessor variable _sp_rate

## sp-define*
~~~
(sp-define* (name parameter ...) (parameter-types ...) body ...)
~~~

* defines a function of name {name}
* automatically declares status-t as the return type
* automatically adds (label exit status-return) if no exit label specified in body
* automatically calls local-memory-free or error-memory-free after the exit label if local-memory-init or error-memory-init was used in body

## sp-define-event*
~~~
(sp-define-event* name body ...)
(sp-define-event* (name duration) body ...)
~~~

* defines an event prepare function, {name}-prepare
* defines a variable, {name}-event
* sets {name}-event.end if duration was given
* sets {name}-event.prepare to {name}-prepare
* adds the local variable _event of type sp-event-t* available in body
* _event is the currently prepared event instance
* adds the local variable _duration to body, set to (event.end - event.start)
* _event:prepare will be automatically called if set in body
* otherwise like sp-define*

## sp-define-group*
* like sp-define-event* but sets _event:prepare to sp-group-prepare

## sp-define-group-parallel*
* like sp-define-event* but sets _event:prepare to sp-group-prepare-parallel

## more
see [../../source/sc/main/sc-macros.sc] for undocumented macros.