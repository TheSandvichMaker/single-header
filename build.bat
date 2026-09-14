@echo off

if not exist build mkdir build

copy microdesk\api.mud build\api.mud

pushd build

set FLAGS=/nologo /I..\dc /W4 /WX /ZI /wd4201 /wd4324 /std:c11 /experimental:c11atomics
set LIB_FLAGS=/nologo /W4 /WX /Z7 /wd4201 /wd4324 /std:c11

REM microdesk
cl ../microdesk/mud_tests.c %FLAGS%

REM microdesk static library for the odin bindings
cl /c ../microdesk/mud.c %LIB_FLAGS% /Fomud.obj
lib /nologo /OUT:mud.lib mud.obj

if not exist ..\microdesk\bindings\odin\lib mkdir ..\microdesk\bindings\odin\lib
copy /Y mud.lib ..\microdesk\bindings\odin\lib\mud.lib

REM dc
cl ../dc/dc_tests.c %FLAGS%

popd build
