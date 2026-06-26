#include "../core/axton.h"

typedef struct framedebug {
    int enabled;
    int framecount;
    double lasttime;
    double fpstime;
    int fpscount;
    int currentfps;
    object *onframe;
    object *onfps;
} framedebug;

static framedebug *fdebug = NULL;

object *framedebugcreate(object **args, int argc, void *env) {
    framedebug *f = malloc(sizeof(framedebug));
    f->enabled = 1;
    f->framecount = 0;
    f->lasttime = platformtime();
    f->fpstime = f->lasttime;
    f->fpscount = 0;
    f->currentfps = 0;
    f->onframe = NULL;
    f->onfps = NULL;
    fdebug = f;
    return makenative(f, NULL);
}

object *framedebugenable(object **args, int argc, void *env) {
    if (argc < 1 || !fdebug) throwexception("enable needs on");
    fdebug->enabled = args[0]->bval;
    return makenone();
}

object *framedebugonframe(object **args, int argc, void *env) {
    if (argc < 1 || !fdebug) throwexception("onframe needs handler");
    fdebug->onframe = args[0];
    return makenone();
}

object *framedebugonfps(object **args, int argc, void *env) {
    if (argc < 1 || !fdebug) throwexception("onfps needs handler");
    fdebug->onfps = args[0];
    return makenone();
}

object *framedebugupdate(object **args, int argc, void *env) {
    if (!fdebug || !fdebug->enabled) return makenone();
    fdebug->framecount++;
    fdebug->fpscount++;
    double now = platformtime();
    if (now - fdebug->fpstime >= 1.0) {
        fdebug->currentfps = fdebug->fpscount;
        fdebug->fpscount = 0;
        fdebug->fpstime = now;
        if (fdebug->onfps) {
            object *args[1] = {makeint(fdebug->currentfps)};
            callfunc(fdebug->onfps, args, 1, env);
        }
    }
    if (fdebug->onframe) {
        object *args[1] = {makeint(fdebug->framecount)};
        callfunc(fdebug->onframe, args, 1, env);
    }
    return makenone();
}

object *framedebugfps(object **args, int argc, void *env) {
    if (!fdebug) return makeint(0);
    return makeint(fdebug->currentfps);
}

object *framedebugframecount(object **args, int argc, void *env) {
    if (!fdebug) return makeint(0);
    return makeint(fdebug->framecount);
}

void registerframedebuglib(environment *env) {
    object *mod = makemodule("framedebug", NULL);
    envset(mod->module.exports, "create", makebuiltin(framedebugcreate), 0);
    envset(mod->module.exports, "enable", makebuiltin(framedebugenable), 0);
    envset(mod->module.exports, "onframe", makebuiltin(framedebugonframe), 0);
    envset(mod->module.exports, "onfps", makebuiltin(framedebugonfps), 0);
    envset(mod->module.exports, "update", makebuiltin(framedebugupdate), 0);
    envset(mod->module.exports, "fps", makebuiltin(framedebugfps), 0);
    envset(mod->module.exports, "framecount", makebuiltin(framedebugframecount), 0);
    envset(env, "framedebug", mod, 0);
  }
