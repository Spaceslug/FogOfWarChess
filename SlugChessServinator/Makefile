LDFLAGS = -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++1z -Wall

GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = ../protobuf
vpath %.proto $(PROTOS_PATH)
ROOT_DIR=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SC_LIB = $(ROOT_DIR)/../SlugChessCore/output
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
DEPENDS := $(patsubst $(SRC)/%.cpp,$(DEP)/%.d,$(SOURCES))
CHESSCOM_OBJ = $(OBJ)/chesscom.pb.o $(OBJ)/chesscom.grpc.pb.o

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean

all: server

server: $(CHESSCOM_OBJ) $(OUT)/server

$(SC_LIB)/libslugchess.so: 
	echo Building SlugChess shared lib
	cd ../SlugChessCore && $(MAKE) libslugchess


$(OUT)/server: $(SC_LIB)/libslugchess.so $(OBJECTS) $(CHESSCOM_OBJ)
	$(CXX) $^ $(LDFLAGS) -L$(SC_LIB) -Wl,-rpath=$(SC_LIB) -o $@ -lslugchess


runserver: server
	./$(OUT)/server

-include $(DEPENDS)

.PRECIOUS: $(CHESSCOM)/%.grpc.pb.cc
$(CHESSCOM)/%.grpc.pb.cc: %.proto
	echo i am aslp run
	protoc -I $(PROTOS_PATH)  --grpc_out=$(CHESSCOM) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: $(CHESSCOM)/%.pb.cc
$(CHESSCOM)/%.pb.cc: %.proto
	echo i am run
	protoc -I $(PROTOS_PATH)  --cpp_out=$(CHESSCOM) $<

clean:
	rm -rf $(OBJ)/* $(DEP)/* $(CHESSCOM)/* $(OUT)/* *.log
	
$(OBJ)/%.pb.o: $(CHESSCOM)/%.pb.cc Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -MMD -MP -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -MMD -MP -c $< -o $@


