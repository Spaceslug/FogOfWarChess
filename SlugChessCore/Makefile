
API_VERSION   = 0
MAJOR_VERSION = 0
MINOR_VERSION = 1
FULL_VERSION  = $(API_VERSION).$(MAJOR_VERSION).$(MINOR_VERSION)

NAME = slugchesscore
NAME_DBG = slugchesscored

LDFLAGS = -ldl 

CXX = g++
CXXFLAGS += -std=c++1z -O2 -Wall 
CXXFLAGS_DBG += -std=c++1z -g -O2 -Wall -DDEBUG 

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SRC = src
OUT = output
OBJ = object
DEP = depends
SHARED_LIB_OUT = /usr/local/lib
SHARED_BIN_OUT = /usr/local/bin
$(shell mkdir -p $(OUT) )
$(shell mkdir -p $(OBJ) )
$(shell mkdir -p $(DEP) )

SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.d,$(SOURCES))
OBJECTS_DBG := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.od,$(SOURCES))

LINK_NAME   = lib$(NAME).so
SONAME      = $(LINK_NAME).$(API_VERSION)
REAL_NAME   = $(LINK_NAME).$(FULL_VERSION)

LINK_NAME_DBG   = lib$(NAME_DBG).so
SONAME_DBG      = $(LINK_NAME_DBG).$(API_VERSION)
REAL_NAME_DBG   = $(LINK_NAME_DBG).$(FULL_VERSION)


# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean

all: slugchesscore

wild:
	@echo $(SOURCES)
	@echo $(OBJECTS)
	@echo $(DEPENDS)

slugchesscore: $(OUT)/$(LINK_NAME)
slugchesscored: $(OUT)/$(LINK_NAME_DBG)
export: $(SHARED_LIB_OUT)/$(LINK_NAME)
export_dbg: $(SHARED_LIB_OUT)/$(LINK_NAME_DBG)
sudo_export:
	sudo $(MAKE) export
slugchesstest: $(OUT)/slugchesstest
slugchesstestd: $(OUT)/slugchesstestd
install_test_dbg: slugchesstestd 
	cp $(OUT)/slugchesstestd $(SHARED_BIN_OUT)

$(OUT)/$(LINK_NAME): $(OBJECTS)
	$(CXX) -shared -fPIC -Wl,-soname,$(SONAME)  -o $(OUT)/$(REAL_NAME) $^
	ln -sf $(REAL_NAME) $(OUT)/$(SONAME)
	ln -sf $(SONAME) $(OUT)/$(LINK_NAME)

$(OUT)/$(LINK_NAME_DBG): $(OBJECTS_DBG)
	$(CXX) -shared -fPIC -Wl,-soname,$(SONAME_DBG)  -o $(OUT)/$(REAL_NAME_DBG) $^
	ln -sf $(REAL_NAME_DBG) $(OUT)/$(SONAME_DBG)
	ln -sf $(SONAME_DBG) $(OUT)/$(LINK_NAME_DBG)

$(SHARED_LIB_OUT)/$(LINK_NAME): $(OUT)/$(LINK_NAME)
	cp $(OUT)/$(REAL_NAME) $@.$(FULL_VERSION)
	ln -sf $(REAL_NAME) $@.$(API_VERSION)
	ln -sf $(SONAME) $@


$(SHARED_LIB_OUT)/$(LINK_NAME_DBG): $(OUT)/$(LINK_NAME_DBG)
	cp $(OUT)/$(REAL_NAME_DBG) $@.$(FULL_VERSION)
	ln -sf $(REAL_NAME_DBG) $@.$(API_VERSION)
	ln -sf $(SONAME_DBG) $@

$(OUT)/slugchesstest: $(OUT)/$(LINK_NAME)
	$(CXX) $(CXXFLAGS) -Wl,--enable-new-dtags -Wl,-rpath=$(SHARED_LIB_OUT) -o $@ $(SRC)/slugchesstest.cpp -l:$(SONAME)

$(OUT)/slugchesstestd: $(OUT)/$(LINK_NAME_DBG)
	$(CXX) $(CXXFLAGS_DBG) -Wl,--enable-new-dtags -Wl,-rpath=$(SHARED_LIB_OUT) -o $@ $(SRC)/slugchesstest.cpp -l:$(SONAME_DBG)

runtest: slugchesstest
	LD_LIBRARY_PATH=./output:$LD_LIBRARY_PATH  $(OUT)/slugchesstest

runtestd: slugchesstestd
	LD_LIBRARY_PATH=./output:$LD_LIBRARY_PATH  $(OUT)/slugchesstestd


-include $(DEPENDS)


clean:
	rm -rf $(OBJ) $(DEP) $(OUT)/*.so* $(OUT)/slugchesstest $(OUT)/slugchesstestd
	
$(OBJ)/%.o: $(SRC)/%.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -fpic -MMD -MP -c $< -o $@

$(OBJ)/%.od: $(SRC)/%.cpp Makefile
	$(CXX) $(CXXFLAGS_DBG) $(LDFLAGS) -fpic -MMD -MP -c $< -o $@

	
