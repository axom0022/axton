#include "axton.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void platforminit(void) {}

void platformlog(const char *msg) {
    printf("%s", msg);
    fflush(stdout);
}

void platformsleep(double seconds) {
    usleep(seconds * 1000000);
}

double platformtime(void) {
    return time(NULL);
}

char *platformreadfile(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = 0;
    fclose(f);
    return buf;
}

int platformwritefile(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

void *platformloadlib(const char *path) { return NULL; }
int platformsocket(void) { return -1; }
int platformbind(int fd, int port) { return -1; }
int platformlisten(int fd, int backlog) { return -1; }
int platformaccept(int fd) { return -1; }
int platformsend(int fd, const char *data, int len) { return -1; }
int platformrecv(int fd, char *buf, int len) { return -1; }
char *platformgetenv(const char *name) { return getenv(name); }
int platformsetenv(const char *name, const char *value) { return setenv(name, value, 1); }
int platformkill(int pid, int sig) { return -1; }
int platformgetpid(void) { return -1; }
void *platformopendir(const char *path) { return NULL; }
char *platformreaddir(void *dir) { return NULL; }
void platformclosedir(void *dir) {}
int platformchmod(const char *path, int mode) { return -1; }
int platformchown(const char *path, int uid, int gid) { return -1; }
void *platformallocate(int size) { return malloc(size); }
void platformdeallocate(void *ptr) { free(ptr); }
void *platformreallocate(void *ptr, int size) { return realloc(ptr, size); }
int platformgetpagesize(void) { return 4096; }
int platformprocessopen(int pid, int flags) { return -1; }
int platformprocessread(int handle, long address, unsigned char *buf, int size) { return -1; }
int platformprocesswrite(int handle, long address, unsigned char *buf, int size) { return -1; }
void platformprocessclose(int handle) {}
int platformprocessfind(const char *name) { return -1; }
int platformsslinit(void) { return -1; }
void *platformsslctxnew(void) { return NULL; }
void *platformsslnew(void *ctx) { return NULL; }
int platformsslconnect(void *ssl, int fd) { return -1; }
int platformsslwrite(void *ssl, const char *data, int len) { return -1; }
int platformsslread(void *ssl, char *buf, int len) { return -1; }
void platformsslclose(void *ssl) {}
void platformssldestroy(void *ssl) {}
int platformunicodechar(const char *utf8) { return -1; }
void platformlspstart(int port) {}
void platformlinteraddrule(void *linter, const char *name, const char *pattern) {}
