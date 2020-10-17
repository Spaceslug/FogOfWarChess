
LDFLAGS = -ldl 

CXX = g++
CXXFLAGS += -std=c++1z -Wall 

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SRC = src
OUT = output
OBJ = object
DEP = depends
SHARED_LIB_OUT = /usr/local/lib
$(shell mkdir -p $(OUT) )
$(shell mkdir -p $(OBJ) )
$(shell mkdir -p $(DEP) )

SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.d,$(SOURCES))

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean

all: libslugchess

wild:
	@echo $(SOURCES)
	@echo $(OBJECTS)
	@echo $(DEPENDS)

libslugchess: $(OUT)/libslugchess.so
export: $(SHARED_LIB_OUT)/libslugchess.so
sudo_export: libslugchess
	sudo $(MAKE) export
slugchesstest: $(OUT)/slugchesstest

$(OUT)/libslugchess.so: $(OBJECTS)
	$(CXX) -shared -fPIC  -o $(OUT)/libslugchess.so $^

$(SHARED_LIB_OUT)/libslugchess.so: libslugchess
	cp $(OUT)/libslugchess.so $@.0.0.1
	ln -sf $@.0.0.1 $@

$(OUT)/slugchesstest: $(OUT)/libslugchess.so
	$(CXX) $(CXXFLAGS) -Wl,-rpath=$(ROOT_DIR)/$(OUT) -o $@ $(SRC)/slugchesstest.cpp -lslugchess
	
runtest: slugchesstest
	$(OUT)/slugchesstest

-include $(DEPENDS)


clean:
	rm -rf $(OBJ) $(DEP) $(OUT)/*.so $(OUT)/slugchesstest
	
$(OBJ)/%.o: $(SRC)/%.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -fpic -MMD -MP -c $< -o $@
	
