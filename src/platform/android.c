#include "platform.h"
#include <android/log.h>
#include <unistd.h>
#include <sys/time.h>

void platforminit(void) {}

void platformlog(const char *msg) {
    __android_log_print(ANDROID_LOG_INFO, "axton", "%s", msg);
}

void platformsleep(double seconds) {
    usleep(seconds * 1000000);
}

double platformtime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

char *platformreadfile(const char *path) {
    FILE *f = fopen(path, "r");
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
