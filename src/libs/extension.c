#include "../core/axton.h"
#include <dlfcn.h>
#include <ffi.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static ffi_type *axtontoffi(int type) {
    switch(type) {
        case 0: return &ffi_type_sint64;
        case 1: return &ffi_type_double;
        case 2: return &ffi_type_pointer;
        case 3: return &ffi_type_sint32;
        case 4: return &ffi_type_void;
        default: return &ffi_type_pointer;
    }
}

object *ffiwrap(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("wrap needs lib and funcname");
    void *lib = args[0]->native.handle;
    char *funcname = args[1]->sval;
    void *fn = platformgetproc(lib, funcname);
    if (!fn) throwexception("function not found");
    object *wrap = makenative(NULL, NULL);
    wrap->type = 70;
    ffiwrapper *fw = malloc(sizeof(ffiwrapper));
    fw->function = fn;
    fw->argcount = 0;
    fw->argtypes = NULL;
    fw->argtypes_code = NULL;
    fw->rettype = 0;
    if (argc > 2) {
        object *retspec = args[2];
        if (retspec->type == 0) fw->rettype = retspec->ival;
        else if (retspec->type == 2) {
            if (strcmp(retspec->sval, "void")==0) fw->rettype = 4;
            else if (strcmp(retspec->sval, "int")==0) fw->rettype = 0;
            else if (strcmp(retspec->sval, "float")==0) fw->rettype = 1;
            else if (strcmp(retspec->sval, "string")==0) fw->rettype = 2;
        }
    }
    if (argc > 3 && args[3]->type == 5) {
        fw->argcount = args[3]->list.count;
        fw->argtypes = malloc(fw->argcount * sizeof(ffi_type*));
        fw->argtypes_code = malloc(fw->argcount * sizeof(int));
        for (int i = 0; i < fw->argcount; i++) {
            int code = args[3]->list.items[i]->ival;
            fw->argtypes_code[i] = code;
            fw->argtypes[i] = axtontoffi(code);
        }
    }
    ffi_status status = ffi_prep_cif(&fw->cif, FFI_DEFAULT_ABI, fw->argcount,
                                     axtontoffi(fw->rettype), fw->argtypes);
    if (status != FFI_OK) throwexception("ffi prep failed");
    wrap->native.data = fw;
    return wrap;
}

object *fficall(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("call needs wrapper");
    ffiwrapper *fw = args[0]->native.data;
    void *values[fw->argcount];
    void *ptrs[fw->argcount];
    for (int i = 0; i < fw->argcount && i+1 < argc; i++) {
        object *arg = args[i+1];
        int code = fw->argtypes_code[i];
        if (code == 0) {
            long v = arg->ival;
            values[i] = malloc(sizeof(long));
            memcpy(values[i], &v, sizeof(long));
        } else if (code == 1) {
            double v = arg->fval;
            values[i] = malloc(sizeof(double));
            memcpy(values[i], &v, sizeof(double));
        } else if (code == 2) {
            values[i] = arg->sval;
        } else if (code == 3) {
            int v = arg->bval;
            values[i] = malloc(sizeof(int));
            memcpy(values[i], &v, sizeof(int));
        } else {
            values[i] = NULL;
        }
        ptrs[i] = &values[i];
    }
    void *retptr = NULL;
    if (fw->rettype != 4) {
        int retsize = fw->rettype == 2 ? sizeof(void*) : (fw->rettype == 0 ? sizeof(long) : (fw->rettype == 1 ? sizeof(double) : sizeof(int)));
        retptr = malloc(retsize);
    }
    ffi_call(&fw->cif, FFI_FN(fw->function), retptr, ptrs);
    object *result = makenone();
    if (fw->rettype == 4) result = makenone();
    else if (fw->rettype == 0) {
        long r = *(long*)retptr;
        result = makeint(r);
    } else if (fw->rettype == 1) {
        double r = *(double*)retptr;
        result = makefloat(r);
    } else if (fw->rettype == 2) {
        char *s = *(char**)retptr;
        result = makestring(s);
    } else if (fw->rettype == 3) {
        int r = *(int*)retptr;
        result = makebool(r);
    }
    for (int i = 0; i < fw->argcount && i+1 < argc; i++) {
        if (fw->argtypes_code[i] != 2) free(values[i]);
    }
    if (retptr) free(retptr);
    return result;
}

object *extensionload(object **args, int argc, void *env) {
    if (argc < 1) throwexception("load needs path");
    void *lib = platformloadlib(args[0]->sval);
    if (!lib) throwexception("cannot load lib");
    object *libobj = makenative(lib, NULL);
    libobj->type = 71;
    return libobj;
}

object *extensionpointer(object **args, int argc, void *env) {
    if (argc < 1) throwexception("pointer needs address");
    long addr = args[0]->ival;
    void *ptr = (void*)addr;
    object *pobj = makenative(ptr, NULL);
    pobj->type = 72;
    return pobj;
}

object *extensionstruct(object **args, int argc, void *env) {
    if (argc < 1) throwexception("struct needs data");
    object *data = args[0];
    if (data->type != 2 && data->type != 5) throwexception("struct needs string or list");
    int size;
    char *buf;
    if (data->type == 2) {
        size = strlen(data->sval) + 1;
        buf = strdup(data->sval);
    } else {
        size = data->list.count * 4;
        buf = malloc(size);
        for (int i = 0; i < data->list.count; i++) {
            if (data->list.items[i]->type == 0) {
                long v = data->list.items[i]->ival;
                memcpy(buf + i*4, &v, 4);
            }
        }
    }
    object *sobj = makenative(buf, NULL);
    sobj->type = 73;
    return sobj;
}

object *extensioncallback(object **args, int argc, void *env) {
    if (argc < 1) throwexception("callback needs function");
    object *fn = args[0];
    object *cb = makenative(fn, NULL);
    cb->type = 74;
    return cb;
}

void registerextensionlib(environment *env) {
    object *mod = makemodule("ext", NULL);
    envset(mod->module.exports, "load", makebuiltin(extensionload), 0);
    envset(mod->module.exports, "wrap", makebuiltin(ffiwrap), 0);
    envset(mod->module.exports, "call", makebuiltin(fficall), 0);
    envset(mod->module.exports, "pointer", makebuiltin(extensionpointer), 0);
    envset(mod->module.exports, "struct", makebuiltin(extensionstruct), 0);
    envset(mod->module.exports, "callback", makebuiltin(extensioncallback), 0);
    envset(env, "ext", mod, 0);
}
