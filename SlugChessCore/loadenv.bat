@echo off
set ENV="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
set TARGET=x64

where nmake.exe >nul 2>nul || (
	call %ENV% %TARGET% || (
		echo.
		echo No building environment available. Edit ENV and TARGET variables in make file.
		exit /b 1
	)
)
