@echo off

copy microdesk\api.mud build\api.mud

if not exist build mkdir build
pushd build

set FLAGS=/nologo /I..\dc /W4 /WX /ZI /wd4201 /wd4324 /std:c11 /experimental:c11atomics

REM microdesk
cl ../microdesk/mud_tests.c %FLAGS%

REM dc
cl ../dc/dc_tests.c %FLAGS%

popd build