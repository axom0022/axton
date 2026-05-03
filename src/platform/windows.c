#include "platform.h"
#include <windows.h>

void platforminit(void) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
}

void platformlog(const char *msg) {
    printf("%s", msg);
    fflush(stdout);
}

void platformsleep(double seconds) {
    Sleep(seconds * 1000);
}

double platformtime(void) {
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / freq.QuadPart;
}

char *platformreadfile(const char *path) {
    HANDLE h = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return NULL;
    DWORD size = GetFileSize(h, NULL);
    char *buf = malloc(size + 1);
    DWORD read;
    ReadFile(h, buf, size, &read, NULL);
    buf[size] = 0;
    CloseHandle(h);
    return buf;
}

int platformwritefile(const char *path, const char *content) {
    HANDLE h = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return 0;
    DWORD len = strlen(content);
    DWORD written;
    WriteFile(h, content, len, &written, NULL);
    CloseHandle(h);
    return 1;
}
