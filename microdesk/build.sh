#!/bin/sh
set -e

MUD_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$MUD_DIR/../build"

CC="${CC:-clang}"
AR="${AR:-ar}"

MUD_FLAGS="-Wall -Wextra -Werror -g -std=c99 -fPIC -O3"

mkdir -p "$BUILD_DIR"
mkdir -p "$MUD_DIR/bindings/odin/lib"

"$CC" -c "$MUD_DIR/mud.c" $MUD_FLAGS -o "$BUILD_DIR/mud.o"

rm -f "$BUILD_DIR/libmud.a"
"$AR" rcs "$BUILD_DIR/libmud.a" "$BUILD_DIR/mud.o"

cp "$BUILD_DIR/libmud.a" "$MUD_DIR/bindings/odin/lib/libmud.a"
