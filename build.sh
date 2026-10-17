#!/bin/sh
# Portable build helper for the OpenBOR v6391 libretro core.
#
#   ./build.sh                     native build (Linux, macOS, MSYS2/MinGW)
#   ./build.sh win                 cross build for Windows (x86_64-w64-mingw32-gcc)
#   ./build.sh test <game.pak>     native build + headless smoke test
#
# Extra make variables can be passed through the environment, e.g.
#   DEBUG=1 ./build.sh   or   CC=clang ./build.sh
set -e
cd "$(dirname "$0")"
JOBS=$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)

case "$1" in
  win)
    make -j"$JOBS" platform=win CC="${CC:-x86_64-w64-mingw32-gcc}"
    ;;
  test)
    [ -n "$2" ] || { echo "usage: $0 test <game.pak>"; exit 1; }
    make -j"$JOBS"
    ${CC:-cc} -O1 -Wall -Ideps/libretro-common/include -o test/frontend test/frontend.c -ldl -lm
    OUT=${TMPDIR:-/tmp}/openbor-libretro-test
    rm -rf "$OUT"
    # wait for the title screen, press Start, then pick "Quit" (4x Down + Start)
    ./test/frontend ./openbor_libretro.so "$2" "$OUT" 4000 \
        500:0:START:6 600:0:DOWN 640:0:DOWN 680:0:DOWN 720:0:DOWN 780:0:START:6 \
        dump=450,560,700
    echo "Frames and audio written to $OUT"
    ;;
  *)
    make -j"$JOBS"
    ;;
esac
