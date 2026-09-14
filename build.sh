#!/bin/sh
set -e

cd "$(dirname "$0")"

CC="${CC:-clang}"
AR="${AR:-ar}"
export CC AR

WARNINGS="-Wall -Wextra -Werror"
FLAGS="-I../dc $WARNINGS -g -std=gnu11"

LIBS=""
if [ "$(uname -s)" = "Linux" ]; then
	LIBS="-lpthread"
fi

mkdir -p build

cp microdesk/api.mud build/api.mud

# microdesk static library for the odin bindings
sh ./microdesk/build.sh

cd build

# microdesk
"$CC" ../microdesk/mud_tests.c $FLAGS -o mud_tests $LIBS

# dc
"$CC" ../dc/dc_tests.c $FLAGS -o dc_tests $LIBS
