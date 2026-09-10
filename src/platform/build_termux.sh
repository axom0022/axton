echo "building axton for termux (android)..."
gcc -o axton \
    src/core/*.c \
    src/platform/android_stub.c \
    -lm -lpthread -ldl \
    -D__ANDROID__ \
    -D_GNU_SOURCE
echo "done"
chmod +x axton
echo "run: ./axton example/cheat/cli_menu.ax"
