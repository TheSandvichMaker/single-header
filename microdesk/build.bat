@echo off
setlocal

set MUD_DIR=%~dp0
set BUILD_DIR=%MUD_DIR%..\build
set MUD_FLAGS=/nologo /W4 /WX /Z7

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%MUD_DIR%bindings\odin\lib" mkdir "%MUD_DIR%bindings\odin\lib"

cl /c "%MUD_DIR%mud.c" %MUD_FLAGS% /Fo"%BUILD_DIR%\mud.obj"
if errorlevel 1 exit /b 1

lib /nologo /OUT:"%BUILD_DIR%\mud.lib" "%BUILD_DIR%\mud.obj"
if errorlevel 1 exit /b 1

copy /Y "%BUILD_DIR%\mud.lib" "%MUD_DIR%bindings\odin\lib\mud.lib"
if errorlevel 1 exit /b 1
