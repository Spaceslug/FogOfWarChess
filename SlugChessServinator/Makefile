LDFLAGS = -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++1z

GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = ../protobuf
vpath %.proto $(PROTOS_PATH)
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
SC_LIB = $(ROOT_DIR)/../SlugChessCore/output
SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
DEPENDS := $(patsubst %.cpp,%.d,$(SOURCES))

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean

all: server 

$(SC_LIB)/libslugchess.so: 
	echo Building SlugChess shared lib
	cd ../SlugChessCore && $(MAKE) libslugchess


server: $(SC_LIB)/libslugchess.so chesscom.pb.o chesscom.grpc.pb.o chesscomservice.o server.o gamebrowser.o matchmanager.o usermanager.o
	$(CXX) $^ $(LDFLAGS) -L$(SC_LIB) -Wl,-rpath=$(SC_LIB) -o $@ -lslugchess


runserver: server
	./server

-include $(DEPENDS)

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto
	protoc -I $(PROTOS_PATH)  --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	protoc -I $(PROTOS_PATH)  --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h $(DEPENDS) server 
	
%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -MMD -MP -c $< -o $@
