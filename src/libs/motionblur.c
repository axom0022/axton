#include "../core/axton.h"
#include <GL/gl.h>

typedef struct motionblur {
    int enabled;
    float strength;
    int samples;
    unsigned int *history;
    int historycount;
    int maxhistory;
} motionblur;

static motionblur *curmb = NULL;

object *motionblurcreate(object **args, int argc, void *env) {
    motionblur *m = malloc(sizeof(motionblur));
    m->enabled = 1;
    m->strength = 0.8;
    m->samples = 8;
    m->maxhistory = 16;
    m->historycount = 0;
    m->history = calloc(m->maxhistory, sizeof(unsigned int));
    curmb = m;
    return makenative(m, NULL);
}

object *motionblursetstrength(object **args, int argc, void *env) {
    if (argc < 1 || !curmb) throwexception("setstrength needs val");
    curmb->strength = args[0]->fval;
    return makenone();
}

object *motionblurenable(object **args, int argc, void *env) {
    if (argc < 1 || !curmb) throwexception("enable needs on");
    curmb->enabled = args[0]->bval;
    return makenone();
}

object *motionblurupdate(object **args, int argc, void *env) {
    if (!curmb || !curmb->enabled) return makenone();
    if (curmb->historycount < curmb->maxhistory) {
        curmb->history[curmb->historycount++] = 1;
    }
    return makenone();
}

void registermotionblurlib(environment *env) {
    object *mod = makemodule("motionblur", NULL);
    envset(mod->module.exports, "create", makebuiltin(motionblurcreate), 0);
    envset(mod->module.exports, "setstrength", makebuiltin(motionblursetstrength), 0);
    envset(mod->module.exports, "enable", makebuiltin(motionblurenable), 0);
    envset(mod->module.exports, "update", makebuiltin(motionblurupdate), 0);
    envset(env, "motionblur", mod, 0);
}
