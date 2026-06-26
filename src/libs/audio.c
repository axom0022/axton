#include "../core/axton.h"
#ifdef __linux__
#include <AL/al.h>
#include <AL/alc.h>
#endif

static ALCdevice *audiodev = NULL;
static ALCcontext *audiocontext = NULL;

object *audioinit(object **args, int argc, void *env) {
#ifdef __linux__
    audiodev = alcOpenDevice(NULL);
    if (!audiodev) throwexception("alc open device failed");
    audiocontext = alcCreateContext(audiodev, NULL);
    if (!audiocontext) { alcCloseDevice(audiodev); throwexception("alc create context failed"); }
    alcMakeContextCurrent(audiocontext);
#endif
    return makenone();
}

object *audioload(object **args, int argc, void *env) {
    if (argc<1) throwexception("load needs path");
    char *path = args[0]->sval;
    return makenone();
}

object *audioplay(object **args, int argc, void *env) {
    return makenone();
}

object *audiopause(object **args, int argc, void *env) { return makenone(); }
object *audiostop(object **args, int argc, void *env) { return makenone(); }
object *audiosetvolume(object **args, int argc, void *env) { return makenone(); }
object *audiosetpos(object **args, int argc, void *env) { return makenone(); }
object *audioupdate(object **args, int argc, void *env) { return makenone(); }

void registeraudiolib(environment *env) {
    object *mod = makemodule("audio", NULL);
    envset(mod->module.exports, "init", makebuiltin(audioinit), 0);
    envset(mod->module.exports, "load", makebuiltin(audioload), 0);
    envset(mod->module.exports, "play", makebuiltin(audioplay), 0);
    envset(mod->module.exports, "pause", makebuiltin(audiopause), 0);
    envset(mod->module.exports, "stop", makebuiltin(audiostop), 0);
    envset(mod->module.exports, "setvolume", makebuiltin(audiosetvolume), 0);
    envset(mod->module.exports, "setpos", makebuiltin(audiosetpos), 0);
    envset(mod->module.exports, "update", makebuiltin(audioupdate), 0);
    envset(env, "audio", mod, 0);
}
