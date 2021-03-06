# General reference

## General rules

### Reserved tokens

#### Angular C structs

As we earlier introduced, C <> code will be compiled to C++ code. This means that new classes and methods will be added to the rest of the vanilla C++ code.

All of the C <> features will be wrapped in public service structs that will be generated at parse time. Those service structs will be made unaccessible to the user.


> Every C <> service structure in the parsed C++ code will be either in the form `__ngc_specifier__` (for general structures) or `__ngc_id_specifier__` (for parse-specific structures), where `id` is a progressive integer number that ensures the uniqueness of the structure name and `specifier` determines the purpose of the service structure.

This means that in C <> code no sequence in the following form will be allowed:

```js
/__ngc_[A-Za-z\d_]+__/
```

any input file to a C <> parser will be checked against this regexp and an error will be thrown if a match is found.

C <> will make use of the following parse-specific structures:

 * `__ngc_id_conditional_true__` and `__ngc_id_conditional_false__`: these structures will be used to wrap the two scopes of an `if <>` clause. See `if` reference for further information.

## Preliminary injection

At the beginning of every C <> parsed code a C++ header will be added, providing templates and definitions that will be used throught the parsed code.
