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

all: client server

client: chesscom.pb.o chesscom.grpc.pb.o client.o
	$(CXX) $^ $(LDFLAGS) -o $@

server: chesscom.pb.o chesscom.grpc.pb.o server.o
	$(CXX) $^ $(LDFLAGS) -o $@

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto
	protoc -I $(PROTOS_PATH)  --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	protoc -I $(PROTOS_PATH)  --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h client server
