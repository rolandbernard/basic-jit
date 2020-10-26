
JIT compiler for BASIC
======================
This is a small JIT compiler for a version of the BASIC programming language, running on the x86-64 architecture.

## About
### Features
This is a relatively _basic_ version of BASIC, but it has some interesting features.
* Variable names can have any length and are case sensitive
* Variable names can include keyword if they also contain other characters
* Labels can be used instead of line numbers
* There is support for 64bit integers, double-precision floats and strings
* Being a JIT compiler the execution is relatively fast (There is some compilation overhead)

### Limitations
This implementation of the BASIC programming language has multiple limitations, not present in some others.
* It doesn't have any runtime errors
    * Array buffer overflows are not checked
    * There is no buffer overflow detection on `READ`
    * There is no type check on `READ`
    * There is no integer overflow detection
* Array elements can't be used in a `FOR`-loop
* Array dimentions must be constant
* When loading files, lines will not be sorted by number

## Hello world
The classic "Hello world"-program for basic works just fine:
```
10  Print "Hello world!"
20  Goto 10
```
