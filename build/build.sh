#!/bin/bash
mkdir -p bin

find_sources() {
    find src -name "*.c" 2>/dev/null | tr '\n' ' '
}

detect_lib() {
    local libname=$1
    if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists $libname 2>/dev/null; then
        pkg-config --libs $libname
    else
        echo "-l$libname"
    fi
}

get_platform_file() {
    local platform=$1
    case "$platform" in
        linux) echo "src/platform/linux.c" ;;
        linuxarm) echo "src/platform/linux.c" ;;
        windows) echo "src/platform/windows.c" ;;
        windows32) echo "src/platform/windows.c" ;;
        macos) echo "src/platform/macos.c" ;;
        macosarm) echo "src/platform/macos.c" ;;
        android) echo "src/platform/android.c" ;;
        ios) echo "src/platform/ios.c" ;;
        web) echo "src/platform/webasm.c" ;;
        freebsd) echo "src/platform/freebsd.c" ;;
        *) echo "" ;;
    esac
}

buildplatform() {
    local name=$1
    local cc=$2
    local cflags=$3
    local libs=$4
    local platformfile=$5
    local output=$6

    echo "building for $name"
    SOURCES=$(find_sources)
    if [ -z "$SOURCES" ]; then
        echo "error: no source files found"
        exit 1
    fi
    $cc -o $output $SOURCES $platformfile $cflags $libs
    echo "done: $output"
}

LIBS="-lm -ldl -lpthread -lsqlite3 -lz -lssl -lcrypto -lcurl -lffi"
LIBS="$LIBS $(detect_lib glfw3) $(detect_lib BulletDynamics) $(detect_lib BulletCollision) $(detect_lib LinearMath) $(detect_lib openal)"

buildlinux() {
    buildplatform "linux" "gcc" "" "$LIBS -lX11 -lpcap -lbluetooth -ltesseract -ljpeg -lpng -lGL -lGLU" "src/platform/linux.c" "bin/axton_linux"
}

buildlinuxarm() {
    buildplatform "linux arm64" "aarch64-linux-gnu-gcc" "-static" "$LIBS -lpcap -lbluetooth -ltesseract -ljpeg -lpng" "src/platform/linux.c" "bin/axton_linux_arm64"
}

buildwindows() {
    buildplatform "windows" "x86_64-w64-mingw32-gcc" "" "$LIBS -lws2_32" "src/platform/windows.c" "bin/axton.exe"
}

buildwindows32() {
    buildplatform "windows 32bit" "i686-w64-mingw32-gcc" "" "$LIBS -lws2_32" "src/platform/windows.c" "bin/axton32.exe"
}

buildmacos() {
    buildplatform "macos intel" "gcc" "" "$LIBS -lpcap -ltesseract -ljpeg -lpng -framework OpenGL -framework Cocoa" "src/platform/macos.c" "bin/axton_macos_intel"
}

buildmacosarm() {
    buildplatform "macos arm64" "gcc" "-arch arm64" "$LIBS -lpcap -ltesseract -ljpeg -lpng -framework OpenGL -framework Cocoa" "src/platform/macos.c" "bin/axton_macos_arm"
}

buildandroid() {
    if [ -z "$ANDROID_NDK" ]; then
        echo "ANDROID_NDK not set"
        exit 1
    fi
    local compiler="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang"
    buildplatform "android" "$compiler" "-static" "$LIBS" "src/platform/android.c" "bin/axton_android"
}

buildios() {
    buildplatform "ios" "xcrun -sdk iphoneos clang" "-arch arm64" "$LIBS" "src/platform/ios.c" "bin/axton_ios"
}

buildweb() {
    buildplatform "web" "emcc" "" "-s WASM=1 -s EXPORTED_FUNCTIONS='[\"_main\"]' -s USE_GLFW=3 -s USE_OPENGL=1 $LIBS" "src/platform/webasm.c" "bin/axton.html"
}

buildfreebsd() {
    buildplatform "freebsd" "clang" "" "$LIBS -lpcap -lbluetooth -ltesseract -ljpeg -lpng" "src/platform/freebsd.c" "bin/axton_freebsd"
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
