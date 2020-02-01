LDFLAGS = -ldl 

CXX = g++
CXXFLAGS += -std=c++1z -Wall 

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SRC  = src
OUT = output
OBJ = object
$(shell mkdir -p $(OUT) )
$(shell mkdir -p $(OBJ) )

SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
DEPENDS := $(patsubst %.cpp,%.d,$(SOURCES))

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean

all: libslugchess.so



libslugchess.so: $(OBJECTS)
	$(CXX) -shared -fPIC  -o $@ $^

slugchesstest: libslugchess.so
	$(CXX) $(CXXFLAGS) -L$(ROOT_DIR) -Wl,-rpath=$(ROOT_DIR) -o $@ slugchesstest.cpp -lslugchess
	
runtest: slugchesstest
	./slugchesstest

-include $(DEPENDS)


clean:
	rm -f *.o  $(DEPENDS) *.so slugchesstest
	
%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -fpic -MMD -MP -c $< -o $@
	