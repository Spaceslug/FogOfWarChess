LDFLAGS = -ldl 

CXX = g++
CXXFLAGS += -std=c++1z -Wall 

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SRC  = src
OUT = output
OBJ = object
DEP = depends
$(shell mkdir -p $(OUT) )
$(shell mkdir -p $(OBJ) )
$(shell mkdir -p $(DEP) )

SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRC)/%.cpp,$(DEP)/%.d,$(SOURCES))

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean

all: libslugchess

wild:
	@echo $(SOURCES)
	@echo $(OBJECTS)
	@echo $(DEPENDS)

libslugchess: $(OUT)/libslugchess.so
slugchesstest: $(OUT)/slugchesstest

$(OUT)/libslugchess.so: $(OBJECTS)
	$(CXX) -shared -fPIC  -o $@ $^

$(OUT)/slugchesstest: $(OUT)/libslugchess.so
	$(CXX) $(CXXFLAGS) -L$(ROOT_DIR)/$(OUT) -Wl,-rpath=$(ROOT_DIR)/$(OUT) -o $@ $(SRC)/slugchesstest.cpp -lslugchess
	
runtest: slugchesstest
	./$(OUT)/slugchesstest

-include $(DEPENDS)


clean:
	rm -rf $(OBJ) $(DEP) $(OUT)/*.so $(OUT)/slugchesstest
	
$(OBJ)/%.o: $(SRC)/%.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -fpic -MMD -MP -c $< -o $@
	