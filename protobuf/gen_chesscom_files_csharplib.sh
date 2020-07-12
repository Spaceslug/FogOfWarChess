#!/bin/bash
mkdir csharp
protoc chesscom.proto --grpc_out=csharp --plugin=protoc-gen-grpc=`which grpc_csharp_plugin` --proto_path=./
protoc chesscom.proto --csharp_out=csharp --proto_path=./

cp -R ./csharp ../ChesscomCSharpLib
