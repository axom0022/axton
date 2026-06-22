#include "../core/axton.h"
#include <tesseract/capi.h>
#include <string.h>

typedef struct ocrengine {
    TessBaseAPI *handle;
} ocrengine;

object *ocrcreate(object **args, int argc, void *env) {
    object *oc = makeocr(NULL);
    ocrengine *o = malloc(sizeof(ocrengine));
    o->handle = TessBaseAPICreate();
    TessBaseAPIInit3(o->handle, NULL, "eng");
    oc->native.data = o;
    return oc;
}

object *ocrload(object **args, int argc, void *env) {
    if (argc < 2) throwexception("ocr.load needs ocr and path");
    object *oc = args[0];
    char *path = args[1]->sval;
    if (oc->type != 118) throwexception("not an ocr object");
    ocrengine *o = (ocrengine*)oc->native.data;
    TessBaseAPISetImage2(o->handle, path);
    return makenone();
}

object *ocrrecognize(object **args, int argc, void *env) {
    if (argc < 1) throwexception("ocr.recognize needs ocr");
    object *oc = args[0];
    if (oc->type != 118) throwexception("not an ocr object");
    ocrengine *o = (ocrengine*)oc->native.data;
    char *text = TessBaseAPIGetUTF8Text(o->handle);
    object *res = makestring(text);
    TessBaseAPIDeleteText(text);
    return res;
}

void registerocrlib(environment *env) {
    object *mod = makemodule("ocr", NULL);
    envset(mod->module.exports, "create", makebuiltin(ocrcreate), 0);
    envset(mod->module.exports, "load", makebuiltin(ocrload), 0);
    envset(mod->module.exports, "recognize", makebuiltin(ocrrecognize), 0);
    envset(env, "ocr", mod, 0);
}
