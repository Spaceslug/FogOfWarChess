CXX=cl.exe
LINK=link.exe
DEPS=..\slugchess.h ..\field.h
CXXFLAGS=/EHcs /O2 /W4
LIBFLAGS=/LD

INSTALLPATH=.

all: build

slugchess.dll: slugchess.lib

slugchess.lib: $(DEPS) ..\slugchess.cpp ..\field.cpp
	$(CXX) $(CXXFLAGS) $(LIBFLAGS) ..\slugchess.cpp ..\field.cpp
	

build: slugchess.dll

clean: 
	del *.dll *.lib *.exp 2>nul