# PicoC
 PicoC interpreter, Qt Console, usable for MCUs

## as Qt Console (makefile) project
- as Windows-64 Console program (use Qt to build)
- usable for MCUs (if large memory available for scripts)

## C-code (script) interpreter
- works in interactive mode or to lauanch C-code scripts
- a bit limited on the C-code statements, e.g. no arrays of struc,
  no typedef

## TODO
- add a native 64bit long long type
- fix issue with #include on interactive command line (RunScript("file") for now

