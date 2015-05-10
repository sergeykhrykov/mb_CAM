@echo off

if "%~3"=="" (
	echo "Usage: %0 <32|64> <Debug|RelWithDebInfo> <OpenGL|OpenGL2> [--install-dependency-dlls]"
	goto end
)

set BIT=%1
set CONFIG=%2
set OPENGL_VERSION=%3

if "%4"=="--install-dependency-dlls" (
	set INSTALL="1"
)
if NOT "%4"=="--install-dependency-dlls" (
	set INSTALL="0"
)
echo INSTALL = %INSTALL%

if %BIT%==32 (
	set GENERATOR="Visual Studio 12 2013"
	set QT_PATH="C:/dev/Qt/Qt5.4.1_32bit/5.4/msvc2013_opengl"
)
if %BIT%==64 (
	set GENERATOR="Visual Studio 12 2013 Win64"
	set QT_PATH="C:/dev/Qt/Qt5.4.1_64bit/5.4/msvc2013_64_opengl"
)

echo Creating directory: build.%BIT%.%CONFIG%.%OPENGL_VERSION%
set BUILD_DIR=build.%BIT%.%CONFIG%.%OPENGL_VERSION%
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo Running CMake...
cmake .. -DMB_BIT:STRING="%BIT%" -DMB_BUILD_CONFIG:STRING="%CONFIG%" -DMB_OPENGL_VERSION="%OPENGL_VERSION%" -DMB_INSTALL_DEPENDENCY_DLLS="%INSTALL%" -G%GENERATOR%

@cd ..

goto end


:end
