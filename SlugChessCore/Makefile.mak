
INSTALLPATH=c:\temp

CC=cl.exe
CXX=cl.exe
LINK=link
DEPS=libWers.h json.hpp
CFLAGS = /DINTERPTYPE=$(INTERPTYPE) /EHsc /O2 /W4 /wd4996

OUT    = output
OBJ    = obj
SRC    = src

!IF [mkdir $(OUT) 2> NUL]
!ELSE
!  MESSAGE Created $(OUT) folder	
!ENDIF
!IF [mkdir $(OBJ) 2> NUL]
!ELSE
!  MESSAGE Created $(OBJ) folder	
!ENDIF


all: libWers testWers validateDatabase validateDatabaseC

build: libWers
	
install: libWers
	copy $(OUT)\libWers.dll $(INSTALLPATH)

clean:
	del /q $(OBJ) $(OUT)

libWers 			: $(OUT)\libWers.dll
testWers 			: $(OUT)\testWers.exe
validateDatabase 	: $(OUT)\validateDatabase.exe
validateDatabaseC 	: $(OUT)\validateDatabaseC.exe

$(OUT)\libWers.lib           : $(OUT)\libWers.dll
$(OUT)\libWers.dll           : $(OBJ)\libWers.obj
$(OUT)\testWers.exe          : $(OBJ)\testWers.obj $(OUT)\libWers.lib
$(OUT)\validateDatabase.exe  : $(OBJ)\validateDatabase.obj $(OUT)\libWers.lib
$(OUT)\validateDatabaseC.exe : $(OBJ)\validateDatabaseC.obj $(OUT)\libWers.lib

{$(SRC)}.cpp{$(OBJ)}.obj:
	$(CXX) /c $(CFLAGS) $< /Fo:$@

{$(SRC)}.c{$(OBJ)}.obj: 
	$(CC) /c $(CFLAGS) $< /Fo:$@

{$(OBJ)}.obj{$(OUT)}.dll:
	$(LINK) $< /DLL /OUT:$@
	copy $(SRC)\libWers.h $(OUT)

{$(OBJ)}.obj{$(OUT)}.exe:
	$(LINK) $< $(OUT)\libWers.lib /OUT:$@