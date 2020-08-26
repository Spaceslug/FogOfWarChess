#!/usr/bin/env python3
import os, re, uploader

rootDir = os.path.dirname(__file__)
projectDir = os.path.join(rootDir, '..')
#avalBinDir = os.path.join(projectDir, 'bin')
updaterDir = os.path.join(projectDir, '../SlugChessUpdater')

# type can be "Release" or "Debug" 
# targets can be ["win-x64", "linux-x64", "osx-x64"]
def publish(type, targets): 
	if type == 'Debug':
		debug = True
	elif type == 'Release':
		debug = False 
	else:
		print('Error: invalid type')

	for target in targets:
		print('Building {} {}'.format(type, target))
		stream = os.popen('dotnet publish {}\\SlugChessAval.csproj -c {} -r {} --self-contained true -p:PublishSingleFile=false -p:PublishTrimmed=true'.format(projectDir, type, target))
		print(stream.read())
		print('Building {} updater {} '.format(type, target))
		stream = os.popen('dotnet publish {}\\SlugChessUpdater.csproj -c {} -r {} -p:PublishSingleFile=true -p:PublishTrimmed=true -o {}\\bin\\{}\\netcoreapp3.1\\{}\\publish\\'.format(updaterDir, type, target, projectDir, type, target))
		print(stream.read())


	stream = os.popen('dotnet-version --show --project-file {}\\SlugChessAval.csproj'.format(projectDir))
	output = stream.read()
	version = re.search('[0-9].+.+$',output).group(0)
	print('Finished publishing SlugChessAval ' + version + ' ' + target)
	uploader.prepereAndUpload(version, targets, debug)
	print('Finished uploading SlugChessAval ' + version + ' ' + target)

