

## Building instructions

Library can be built with gcc or Microsofts cl.exe. Use `nmake` for Visual Studio version
and gnu `make` for gcc version. 

gcc-version:
*  Copy `Makefile.gcc` to `Makefile` 
*  `make libWers` builds the shared library
   -  `make libWers INTERPTYPE=1` to use nearest interpolation
*  `make testWers` builds test binary
*  `make validateDatabase` builds database validation utility
*  `make validateDatabaseC` builds plain c version of validation utility
*  `make all` to build all of the above
*  `make build` builds only shared library
*  `make clean` to clean output amd build folders
*  In addition some tests can be compilled:
   - `make testWersVC` builds test utility using Visual Studio Compiler and gcc built library
   - `make validateDatabaseVC` same as above

Visual Studio version:
*  Open `x64 Native Tools Command Prompt for VS 2017`
   or `x86 Native Tools Command Prompt for VS 2017` and change current directory.
   -  Alternativly you can call the supplied `loadenv.bat`
   -  Or copy `buildall_example.bat` to `buildall.bat`, edit it and run it.
*  NB! Copy `Makefile.mak` to `Makefile` or use `nmake -f Makefile.mak <build target>`
*  `nmake libWers` builds shared library
   -  `nmake libWers INTERPTYPE=1` to use nearest interpolation
*  `nmake testWers` builds test utility
*  `nmake validateDatabase` builds validation utility
*  `nmake validateDatabaseC` builds plain c version of validation utility
*  `nmake all` to build all of the above
*  `nmake build` builds only shared library
*  `nmake clean` to clean output and build folders

