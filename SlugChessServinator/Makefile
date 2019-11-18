LDFLAGS = -L/usr/local/lib `pkg-config --libs protobuf grpc++`\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

CXX = g++
CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++11

GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = ../protobuf
vpath %.proto $(PROTOS_PATH)

SOURCES := $(wildcard *.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
DEPENDS := $(patsubst %.cpp,%.d,$(SOURCES))

# .PHONY means these rules get executed even if
# files of those names exist.
.PHONY: all clean

all: server 

server: chesscom.pb.o chesscom.grpc.pb.o field.o slugchess.o server.o 
	$(CXX) $^ $(LDFLAGS) -o $@

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