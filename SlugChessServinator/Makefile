LDFLAGS = -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++1z -O2 -Wall
CXXFLAGS_DBG += -std=c++1z -g -O2 -Wall -DDEBUG 

CXXFLAGS_PB += -std=c++1z -O2

GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = ../protobuf
vpath %.proto $(PROTOS_PATH)
ROOT_DIR=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SC_LIB = $(ROOT_DIR)/../SlugChessCore/output
SHARED_LIB_OUT = /usr/local/lib
SRC  = src
OUT = output
OBJ = object
DEP = depends
CHESSCOM = chesscom
$(shell mkdir -p $(OUT) )
$(shell mkdir -p $(OBJ) )
$(shell mkdir -p $(DEP) )
$(shell mkdir -p $(CHESSCOM) )
SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.d,$(SOURCES))
OBJECTS_DBG := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.od,$(SOURCES))
CHESSCOM_OBJ = $(OBJ)/chesscom.pb.o $(OBJ)/chesscom.grpc.pb.o

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean clean-output

all: slugchess-server
dockerlib: 
	cd ../SlugChessCore && $(MAKE) export  
docker: dockerlib slugchess-server

slugchess-server: clean-output $(CHESSCOM_OBJ) $(OUT)/slugchess-server
slugchess-server-dbg:  $(CHESSCOM_OBJ) $(SC_LIB)/libslugchesscored.so $(OUT)/slugchess-server-dbg

run-debugserver: slugchess-server-dbg
	./$(OUT)/slugchess-server-dbg 43326

clean-output:
	rm -rf $(OUT)/*

$(SHARED_LIB_OUT)/libslugchesscore.so: 
	echo Building SlugChess shared lib. NO do this youself lazy man. Only need to exist runtome
#	cd ../SlugChessCore && $(MAKE) sudo_export  

$(SC_LIB)/libslugchesscored.so: 
	echo Building SlugChess local lib. NO do this youself lazy man. Only need to exist runtome
#	cd ../SlugChessCore && $(MAKE) slugchesscored

$(OUT)/slugchess-server: $(SHARED_LIB_OUT)/libslugchesscore.so $(OBJECTS) $(CHESSCOM_OBJ)
	$(CXX) $^ $(LDFLAGS) -L$(SHARED_LIB_OUT) -Wl,--enable-new-dtags -Wl,-rpath=$(SHARED_LIB_OUT) -o $@ -l:libslugchesscore.so.0

$(OUT)/slugchess-server-dbg: $(OBJECTS_DBG) $(CHESSCOM_OBJ)
	$(CXX) $^ $(LDFLAGS) -L$(SC_LIB) -Wl,--enable-new-dtags -Wl,-rpath=$(SC_LIB) -o $@ -l:libslugchesscored.so.0




-include $(DEPENDS)

.PRECIOUS: $(CHESSCOM)/%.grpc.pb.cc
$(CHESSCOM)/%.grpc.pb.cc: %.proto
	protoc -I $(PROTOS_PATH)  --grpc_out=$(CHESSCOM) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: $(CHESSCOM)/%.pb.cc
$(CHESSCOM)/%.pb.cc: %.proto
	protoc -I $(PROTOS_PATH)  --cpp_out=$(CHESSCOM) $<

clean:
	rm -rf $(OBJ)/* $(DEP)/* $(CHESSCOM)/* $(OUT)/* *.log
	
$(OBJ)/%.pb.o: $(CHESSCOM)/%.pb.cc Makefile 
	$(CXX) $(CXXFLAGS_PB) $(LDFLAGS) -MMD -MP -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.cpp Makefile $(CHESSCOM_OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -MMD -MP -c $< -o $@

$(OBJ)/%.od: $(SRC)/%.cpp Makefile $(CHESSCOM_OBJ)
	$(CXX) $(CXXFLAGS_DBG) $(LDFLAGS) -MMD -MP -c $< -o $@


