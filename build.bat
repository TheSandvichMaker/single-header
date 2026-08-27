@echo off

copy microdesk\api.mud build\api.mud

if not exist build mkdir build
pushd build

REM microdesk
cl /nologo ../microdesk/mud_tests.c /I../dc /W4 /WX /Zi /wd4201

popd build