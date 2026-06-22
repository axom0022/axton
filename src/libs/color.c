#include "../core/axton.h"

object *colorprint(object **a, int c, void *e) {
    if (c < 2) throwexception("print needs color and text");
    char *color = a[0]->sval;
    char *text = a[1]->sval;
    if (strcmp(color, "red") == 0) platformlog("\033[31m");
    else if (strcmp(color, "green") == 0) platformlog("\033[32m");
    else if (strcmp(color, "yellow") == 0) platformlog("\033[33m");
    else if (strcmp(color, "blue") == 0) platformlog("\033[34m");
    platformlog(text);
    platformlog("\033[0m\n");
    return makenone();
}

void registercolorlib(environment *env) {
    object *mod = makemodule("color", NULL);
    envset(mod->module.exports, "print", makebuiltin(colorprint), 0);
    envset(env, "color", mod, 0);
}
