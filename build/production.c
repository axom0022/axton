#!/bin/bash
set -e
echo "Building Axton for production..."

mkdir -p bin

build_linux() {
    echo "Linux x86_64"
    gcc -O2 -o bin/axton_linux src/core/*.c src/libs/*.c -lm -ldl -lpthread -lX11 -lGL -lssl -lcrypto -lGLU -lglut
}

build_windows() {
    echo "Windows x86_64 (cross)"
    x86_64-w64-mingw32-gcc -O2 -o bin/axton.exe src/core/*.c src/libs/*.c -lm -lws2_32 -lssl -lcrypto -lgdi32
}

build_macos() {
    echo "macOS universal"
    gcc -O2 -o bin/axton_macos src/core/*.c src/libs/*.c -lm -ldl -lpthread -framework OpenGL -framework Cocoa
}

build_android() {
    echo "Android ARM64 (requires NDK)"
    if [ -z "$ANDROID_NDK" ]; then echo "ANDROID_NDK not set"; exit 1; fi
    $ANDROID_NDK/bin/aarch64-linux-android21-clang -O2 -o bin/axton_android src/core/*.c src/libs/*.c -lm -static
}

build_web() {
    echo "WebAssembly (requires emsdk)"
    emcc -O2 -o bin/axton.html src/core/*.c src/libs/*.c -s WASM=1 -s EXPORTED_FUNCTIONS='["_main"]' -s USE_GLFW=3
}

case "$1" in
    linux) build_linux ;;
    windows) build_windows ;;
    macos) build_macos ;;
    android) build_android ;;
    web) build_web ;;
    all) build_linux; build_windows; build_macos; build_android; build_web ;;
    *) echo "Usage: ./production.sh [linux|windows|macos|android|web|all]"; exit 1 ;;
esac
echo "Done."
