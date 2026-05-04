#!/bin/bash

mkdir -p bin

buildlinux() {
    echo "building for linux"
    gcc -o bin/axton_linux \
        src/core/*.c \
        src/platform/linux.c \
        src/tools/*.c \
        src/libs/*.c \
        -lm -ldl -lpthread -lsqlite3 -lz -lX11 -lssl -lcrypto
    echo "done"
}

buildlinuxarm() {
    echo "building for linux arm64"
    aarch64-linux-gnu-gcc -o bin/axton_linux_arm64 \
        src/core/*.c \
        src/platform/linux.c \
        src/tools/*.c \
        src/libs/*.c \
        -lm -ldl -lpthread -lsqlite3 -lz -static
    echo "done"
}

buildwindows() {
    echo "building for windows"
    x86_64-w64-mingw32-gcc -o bin/axton.exe \
        src/core/*.c \
        src/platform/windows.c \
        src/tools/*.c \
        src/libs/*.c \
        -lm -lws2_32 -lsqlite3 -lz
    echo "done"
}

buildwindows32() {
    echo "building for windows 32bit"
    i686-w64-mingw32-gcc -o bin/axton32.exe \
        src/core/*.c \
        src/platform/windows.c \
        src/tools/*.c \
        src/libs/*.c \
        -lm -lws2_32 -lsqlite3 -lz
    echo "done"
}

buildmacos() {
    echo "building for macos intel"
    gcc -o bin/axton_macos_intel \
        src/core/*.c \
        src/platform/macos.c \
        src/tools/*.c \
        src/libs/*.c \
        -lm -ldl -lpthread -lsqlite3 -lz
    echo "done"
}

buildmacosarm() {
    echo "building for macos arm64"
    gcc -arch arm64 -o bin/axton_macos_arm \
        src/core/*.c \
        src/platform/macos.c \
        src/tools/*.c \
        src/libs/*.c \
        -lm -ldl -lpthread -lsqlite3 -lz
    echo "done"
}

buildandroid() {
    echo "building for android"
    if [ -z "$ANDROID_NDK" ]; then
        echo "ANDROID_NDK not set"
        exit 1
    fi
    local compiler="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang"
    $compiler -o bin/axton_android \
        src/core/*.c \
        src/platform/android.c \
        src/tools/*.c \
        src/libs/*.c \
        -lm -static
    echo "done"
}

buildios() {
    echo "building for ios"
    xcrun -sdk iphoneos clang -arch arm64 \
        -o bin/axton_ios \
        src/core/*.c \
        src/platform/ios.c \
        src/tools/*.c \
        src/libs/*.c \
        -lm
    echo "done"
}

buildweb() {
    echo "building for web"
    emcc -o bin/axton.html \
        src/core/*.c \
        src/platform/webasm.c \
        src/tools/*.c \
        src/libs/*.c \
        -s WASM=1 -s EXPORTED_FUNCTIONS='["_main"]'
    echo "done"
}

buildfreebsd() {
    echo "building for freebsd"
    clang -o bin/axton_freebsd \
        src/core/*.c \
        src/platform/freebsd.c \
        src/tools/*.c \
        src/libs/*.c \
        -lm
    echo "done"
}

buildall() {
    buildlinux
    buildlinuxarm
    buildwindows
    buildwindows32
    buildmacos
    buildmacosarm
    buildandroid
    buildios
    buildweb
    buildfreebsd
}

clean() {
    echo "cleaning"
    rm -rf bin/*
    echo "done"
}

case "$1" in
    linux) buildlinux ;;
    linuxarm) buildlinuxarm ;;
    windows) buildwindows ;;
    windows32) buildwindows32 ;;
    macos) buildmacos ;;
    macosarm) buildmacosarm ;;
    android) buildandroid ;;
    ios) buildios ;;
    web) buildweb ;;
    freebsd) buildfreebsd ;;
    all) buildall ;;
    clean) clean ;;
    *)
        echo "usage: ./build.sh [linux|linuxarm|windows|windows32|macos|macosarm|android|ios|web|freebsd|all|clean]"
        ;;
esac
