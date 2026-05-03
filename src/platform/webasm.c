#include "platform.h"
#include <emscripten.h>

void platforminit(void) {}

void platformlog(const char *msg) {
    emscripten_log(EM_LOG_CONSOLE, "%s", msg);
}

void platformsleep(double seconds) {
    emscripten_sleep(seconds * 1000);
}

double platformtime(void) {
    return emscripten_get_now() / 1000.0;
}

char *platformreadfile(const char *path) {
    return NULL;
}

int platformwritefile(const char *path, const char *content) {
    return 0;
}
