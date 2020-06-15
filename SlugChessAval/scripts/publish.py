#!/usr/bin/env python3
import os, re, uploader
stream = os.popen('dotnet publish ..\\SlugChessAval.csproj -c release -r win-x64 --self-contained true -p:PublishSingleFile=false -p:PublishTrimmed=true')
print(stream.read())
stream = os.popen('dotnet publish ..\\SlugChessAval.csproj -c release -r linux-x64 --self-contained true -p:PublishSingleFile=false -p:PublishTrimmed=true')
print(stream.read())
stream = os.popen('dotnet publish ..\\SlugChessAval.csproj -c release -r osx-x64 --self-contained true -p:PublishSingleFile=false -p:PublishTrimmed=true')
print(stream.read())


stream = os.popen('dotnet-version --show --project-file ..\\SlugChessAval.csproj')
output = stream.read()
version = re.search('[0-9].+.+$',output).group(0)
print('Finished publishing SlugChessAval ' + version)
uploader.prepereAndUpload(version)

