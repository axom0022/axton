#include "../core/axton.h"

object *audiocreate(object **args, int argc, void *env) {
    void *audio = platformaudiocreate();
    if (!audio) throwexception("audio create failed");
    return makeaudioobj(audio);
}

object *audioload(object **args, int argc, void *env) {
    if (argc < 2) throwexception("load needs path");
    object *audio = args[0];
    char *path = args[1]->sval;
    platformaudioload(audio->audioobj.audio, path);
    return makenone();
}

object *audioplay(object **args, int argc, void *env) {
    if (argc < 1) throwexception("play needs audio");
    object *audio = args[0];
    platformaudioplay(audio->audioobj.audio);
    return makenone();
}

object *audiostop(object **args, int argc, void *env) {
    if (argc < 1) throwexception("stop needs audio");
    object *audio = args[0];
    platformaudiostop(audio->audioobj.audio);
    return makenone();
}

void registeraudiomodule(environment *env) {
    object *mod = makemodule("audio", NULL);
    envset(mod->module.exports, "create", makebuiltin(audiocreate), 0);
    envset(mod->module.exports, "load", makebuiltin(audioload), 0);
    envset(mod->module.exports, "play", makebuiltin(audioplay), 0);
    envset(mod->module.exports, "stop", makebuiltin(audiostop), 0);
    envset(env, "audio", mod, 0);
}
