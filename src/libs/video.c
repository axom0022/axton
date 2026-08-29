#include "../core/axton.h"

object *videocreate(object **args, int argc, void *env) {
    void *vid = platformvideocreate();
    if (!vid) throwexception("video create failed");
    return makevideo(vid, NULL, 0,0,0);
}

object *videoload(object **args, int argc, void *env) {
    if (argc < 2) throwexception("load needs video path");
    object *vid = args[0];
    char *path = args[1]->sval;
    int res = platformvideoload(vid->video.stream, path);
    if (!res) throwexception("video load failed");
    return makenone();
}

object *videoplay(object **args, int argc, void *env) {
    if (argc < 1) throwexception("play needs video");
    object *vid = args[0];
    int res = platformvideoplay(vid->video.stream);
    if (!res) throwexception("video play failed");
    return makenone();
}

object *videoframe(object **args, int argc, void *env) {
    if (argc < 1) throwexception("frame needs video");
    object *vid = args[0];
    unsigned char *data = NULL;
    int res = platformvideoframe(vid->video.stream, &data);
    if (!res) return makenone();
    object *list = makelist();
    int size = vid->video.w * vid->video.h * 3;
    for (int i=0;i<size;i++) listappend(list, makeint(data[i]));
    free(data);
    return list;
}

void registervideolib(environment *env) {
    object *mod = makemodule("video", NULL);
    envset(mod->module.exports, "create", makebuiltin(videocreate), 0);
    envset(mod->module.exports, "load", makebuiltin(videoload), 0);
    envset(mod->module.exports, "play", makebuiltin(videoplay), 0);
    envset(mod->module.exports, "frame", makebuiltin(videoframe), 0);
    envset(env, "video", mod, 0);
}
