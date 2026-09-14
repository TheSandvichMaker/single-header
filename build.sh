#!/bin/sh
set -e

cd "$(dirname "$0")"

CC="${CC:-clang}"
AR="${AR:-ar}"

WARNINGS="-Wall -Wextra -Werror"
FLAGS="-I../dc $WARNINGS -g -std=gnu11"
LIB_FLAGS="$WARNINGS -g -std=c11 -fPIC"

LIBS=""
if [ "$(uname -s)" = "Linux" ]; then
	LIBS="-lpthread"
fi

mkdir -p build

cp microdesk/api.mud build/api.mud

cd build

# microdesk
"$CC" ../microdesk/mud_tests.c $FLAGS -o mud_tests $LIBS

# microdesk static library for the odin bindings
"$CC" -c ../microdesk/mud.c $LIB_FLAGS -o mud.o
rm -f libmud.a
"$AR" rcs libmud.a mud.o

mkdir -p ../microdesk/bindings/odin/lib
cp libmud.a ../microdesk/bindings/odin/lib/libmud.a

# dc
"$CC" ../dc/dc_tests.c $FLAGS -o dc_tests $LIBS
