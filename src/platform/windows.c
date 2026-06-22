#include "platform.h"
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void *platformloadlib(const char *path) {
    return LoadLibrary(path);
}

int platformsocket(void) {
    return socket(AF_INET, SOCK_STREAM, 0);
}

int platformbind(int fd, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    return bind(fd, (struct sockaddr*)&addr, sizeof(addr));
}

int platformlisten(int fd, int backlog) {
    return listen(fd, backlog);
}

int platformaccept(int fd) {
    return accept(fd, NULL, NULL);
}

int platformsend(int fd, const char *data, int len) {
    return send(fd, data, len, 0);
}

int platformrecv(int fd, char *buf, int len) {
    return recv(fd, buf, len, 0);
}

char *platformgetenv(const char *name) {
    static char buf[1024];
    DWORD size = GetEnvironmentVariable(name, buf, sizeof(buf));
    if (size == 0) return NULL;
    return buf;
}

int platformsetenv(const char *name, const char *value) {
    return SetEnvironmentVariable(name, value) ? 1 : 0;
}

int platformkill(int pid, int sig) {
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!h) return -1;
    TerminateProcess(h, sig);
    CloseHandle(h);
    return 0;
}

int platformgetpid(void) {
    return GetCurrentProcessId();
}

void *platformopendir(const char *path) {
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*", path);
    HANDLE h = FindFirstFile(pattern, NULL);
    if (h == INVALID_HANDLE_VALUE) return NULL;
    return (void*)h;
}

char *platformreaddir(void *dir) {
    HANDLE h = (HANDLE)dir;
    WIN32_FIND_DATA f;
    if (FindNextFile(h, &f)) {
        return strdup(f.cFileName);
    }
    return NULL;
}

void platformclosedir(void *dir) {
    FindClose((HANDLE)dir);
}

int platformchmod(const char *path, int mode) {
    // Windows doesn't support chmod directly; we can use SetFileAttributes
    // but mode is not standard; we'll ignore for now
    return 0;
}

int platformchown(const char *path, int uid, int gid) {
    return 0;
}

void *platformallocate(int size) {
    return malloc(size);
}

void platformdeallocate(void *ptr) {
    free(ptr);
}

void *platformreallocate(void *ptr, int size) {
    return realloc(ptr, size);
}

int platformgetpagesize(void) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
}

int platformpcapopen(const char *iface, char *err) { return -1; }
int platformpcapnext(int handle, unsigned char **data, int *len) { return -1; }
int platformpcapinject(int handle, unsigned char *packet, int len) { return -1; }
void platformpcapclose(int handle) {}
int platformbluetoothopen(void) { return -1; }
int platformbluetoothscan(int handle, object *devices) { return -1; }
void platformbluetoothclose(int handle) {}

int platformprocessopen(int pid, int flags) {
    HANDLE h = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
    if (!h) return -1;
    return (int)(long)h;
}

int platformprocessread(int handle, long address, unsigned char *buf, int size) {
    HANDLE h = (HANDLE)(long)handle;
    SIZE_T read;
    if (!ReadProcessMemory(h, (void*)address, buf, size, &read)) return -1;
    return 0;
}

int platformprocesswrite(int handle, long address, unsigned char *buf, int size) {
    HANDLE h = (HANDLE)(long)handle;
    SIZE_T written;
    if (!WriteProcessMemory(h, (void*)address, buf, size, &written)) return -1;
    return 0;
}

void platformprocessclose(int handle) {
    HANDLE h = (HANDLE)(long)handle;
    CloseHandle(h);
}

int platformprocessfind(const char *name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return -1;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot, &pe)) {
        CloseHandle(snapshot);
        return -1;
    }
    do {
        if (strcmp(pe.szExeFile, name) == 0) {
            CloseHandle(snapshot);
            return pe.th32ProcessID;
        }
    } while (Process32Next(snapshot, &pe));
    CloseHandle(snapshot);
    return -1;
}
