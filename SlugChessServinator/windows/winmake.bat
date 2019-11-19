set ENV="c:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat"
set TARGET=x64

where nmake.exe >nul 2>nul || (
	call %ENV% %TARGET% || (
		echo.
		echo No buildig envoriomen. Edit ENV and TARGET ti make file work
		exit /b 1
	)
)

nmake %*