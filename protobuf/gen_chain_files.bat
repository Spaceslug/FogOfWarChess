

protoc.exe chesscom.proto --grpc_out=csharp --plugin=protoc-gen-grpc=grpc_csharp_plugin.exe --proto_path=.\
protoc.exe chesscom.proto --csharp_out=csharp --proto_path=.\

xcopy /y /s ".\csharp" /d "..\DarkChess\chesscom"


pause