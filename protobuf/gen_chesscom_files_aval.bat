IF "%1"=="" (
  echo no args
) ELSE (
  cd %~1
  echo %~1
)
dir




if not exist "csharp\" mkdir "csharp\"

protoc.exe chesscom.proto --grpc_out=csharp --plugin=protoc-gen-grpc=grpc_csharp_plugin.exe --proto_path=.\
protoc.exe chesscom.proto --csharp_out=csharp --proto_path=.\

xcopy /y /s ".\csharp" /d "..\SlugChessAval\Chesscom"

pause