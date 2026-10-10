#include "axton.h"

static void axitoa(long v, char *out) {
    char tmp[24];
    int n = 0;
    unsigned long u;
    if (v < 0) {
        *out++ = '-';
        u = (unsigned long)(-(v + 1)) + 1;
    } else {
        u = (unsigned long)v;
    }
    do {
        tmp[n++] = (char)('0' + (u % 10));
        u /= 10;
    } while (u);
    while (n > 0) *out++ = tmp[--n];
    *out = 0;
}

#include "../libs/register.c"

object *builtinprint(object **args, int argc, environment *env) {
    for (int i = 0; i < argc; i++) {
        if (otype(args[i]) == 2) platformlog(args[i]->sval);
        else if (otype(args[i]) == 0) {
            char buf[64];
            axitoa(getival(args[i]), buf);
            platformlog(buf);
        }
        else if (otype(args[i]) == 1) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", args[i]->fval);
            platformlog(buf);
        }
        else if (otype(args[i]) == 3) platformlog(getbval(args[i]) ? "true" : "false");
        else if (otype(args[i]) == 4) platformlog("none");
        else if (otype(args[i]) == 5) platformlog("list");
        else if (otype(args[i]) == 6) platformlog("dict");
        else if (otype(args[i]) == 7) platformlog("function");
        else if (otype(args[i]) == 9) platformlog("class");
        else if (otype(args[i]) == 10) platformlog("instance");
        else platformlog("object");
        if (i < argc - 1) platformlog(" ");
    }
    platformlog("\n");
    return makenone();
}

object *builtinlen(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("len needs 1 argument");
    if (otype(args[0]) == 2) return makeint(strlen(args[0]->sval));
    if (otype(args[0]) == 5) return makeint(args[0]->list.count);
    if (otype(args[0]) == 6) return makeint(args[0]->dict.count);
    if (otype(args[0]) == 20) return makeint(args[0]->tensor.rows * args[0]->tensor.cols);
    throwexception("object has no length");
    return NULL;
}

object *builtinstr(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("str needs 1 argument");
    char buf[256];
    switch (otype(args[0])) {
        case 0: axitoa(getival(args[0]), buf); break;
        case 1: snprintf(buf, sizeof(buf), "%g", args[0]->fval); break;
        case 2: return makestring(args[0]->sval);
        case 3: return makestring(getbval(args[0]) ? "true" : "false");
        case 4: return makestring("none");
        case 5: {
            char *s = malloc(1024);
            strcpy(s, "[");
            for (int i = 0; i < args[0]->list.count; i++) {
                object *itemstr = builtinstr(&args[0]->list.items[i], 1, env);
                strcat(s, itemstr->sval);
                if (i < args[0]->list.count - 1) strcat(s, ", ");
            }
            strcat(s, "]");
            return makestring(s);
        }
        case 6: {
            char *s = malloc(1024);
            strcpy(s, "{");
            for (int i = 0; i < args[0]->dict.count; i++) {
                object *keystr = builtinstr(&args[0]->dict.keyvals[i], 1, env);
                object *valstr = builtinstr(&args[0]->dict.vals[i], 1, env);
                char entry[256];
                snprintf(entry, sizeof(entry), "%s: %s", keystr->sval, valstr->sval);
                strcat(s, entry);
                if (i < args[0]->dict.count - 1) strcat(s, ", ");
            }
            strcat(s, "}");
            return makestring(s);
        }
        default: return makestring("object");
    }
    return makestring(buf);
}

object *builtinint(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("int needs 1 argument");
    if (otype(args[0]) == 0) return makeint(getival(args[0]));
    if (otype(args[0]) == 1) return makeint((long)args[0]->fval);
    if (otype(args[0]) == 2) {
        long val = atol(args[0]->sval);
        if (val == 0 && args[0]->sval[0] != '0') throwexception("invalid literal");
        return makeint(val);
    }
    if (otype(args[0]) == 3) return makeint(getbval(args[0]) ? 1 : 0);
    throwexception("cannot convert to int");
    return NULL;
}

object *builtinfloat(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("float needs 1 argument");
    if (otype(args[0]) == 1) return makefloat(args[0]->fval);
    if (otype(args[0]) == 0) return makefloat((double)getival(args[0]));
    if (otype(args[0]) == 2) {
        double val = atof(args[0]->sval);
        if (val == 0 && args[0]->sval[0] != '0') throwexception("invalid literal");
        return makefloat(val);
    }
    if (otype(args[0]) == 3) return makefloat(getbval(args[0]) ? 1.0 : 0.0);
    throwexception("cannot convert to float");
    return NULL;
}

object *builtinbool(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("bool needs 1 argument");
    return makebool(istruthy(args[0]));
}

object *builtininput(object **args, int argc, environment *env) {
    if (argc > 0) builtinprint(args, argc, env);
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stdin)) return makestring("");
    buf[strcspn(buf, "\n")] = 0;
    return makestring(buf);
}

object *builtinrange(object **args, int argc, environment *env) {
    long start = 0, stop = 0, step = 1;
    if (argc == 1) {
        if (otype(args[0]) != 0) throwexception("range needs integer");
        stop = getival(args[0]);
    } else if (argc == 2) {
        if (otype(args[0]) != 0 || otype(args[1]) != 0) throwexception("range needs integer");
        start = getival(args[0]);
        stop = getival(args[1]);
    } else if (argc == 3) {
        if (otype(args[0]) != 0 || otype(args[1]) != 0 || otype(args[2]) != 0)
            throwexception("range needs integer");
        start = getival(args[0]);
        stop = getival(args[1]);
        step = getival(args[2]);
    } else {
        throwexception("range needs 1-3 arguments");
    }
    return makerange(start, stop, step);
}

object *builtintype(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("type needs 1 argument");
    switch (otype(args[0])) {
        case 0: return makestring("int");
        case 1: return makestring("float");
        case 2: return makestring("str");
        case 3: return makestring("bool");
        case 4: return makestring("none");
        case 5: return makestring("list");
        case 6: return makestring("dict");
        case 7: return makestring("function");
        case 8: return makestring("builtin");
        case 9: return makestring("class");
        case 10: return makestring("instance");
        case 11: return makestring("range");
        case 20: return makestring("tensor");
        case 21: return makestring("generator");
        case 22: return makestring("coroutine");
        default: return makestring("object");
    }
}

object *builtinabs(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("abs needs 1 argument");
    if (otype(args[0]) == 0) return makeint(labs(getival(args[0])));
    if (otype(args[0]) == 1) return makefloat(fabs(args[0]->fval));
    throwexception("bad operand for abs");
    return NULL;
}

object *builtinmax(object **args, int argc, environment *env) {
    if (argc == 1 && otype(args[0]) == 5) {
        if (args[0]->list.count == 0) throwexception("max of empty list");
        object *max = args[0]->list.items[0];
        for (int i = 1; i < args[0]->list.count; i++) {
            if (greaterthan(args[0]->list.items[i], max)) max = args[0]->list.items[i];
        }
        return max;
    }
    if (argc >= 2) {
        object *max = args[0];
        for (int i = 1; i < argc; i++) {
            if (greaterthan(args[i], max)) max = args[i];
        }
        return max;
    }
    throwexception("max needs arguments");
    return NULL;
}

object *builtinmin(object **args, int argc, environment *env) {
    if (argc == 1 && otype(args[0]) == 5) {
        if (args[0]->list.count == 0) throwexception("min of empty list");
        object *min = args[0]->list.items[0];
        for (int i = 1; i < args[0]->list.count; i++) {
            if (lessthan(args[0]->list.items[i], min)) min = args[0]->list.items[i];
        }
        return min;
    }
    if (argc >= 2) {
        object *min = args[0];
        for (int i = 1; i < argc; i++) {
            if (lessthan(args[i], min)) min = args[i];
        }
        return min;
    }
    throwexception("min needs arguments");
    return NULL;
}

object *builtinsum(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("sum needs list");
    object *list = args[0];
    if (otype(list) != 5) throwexception("sum expects list");
    double total = 0;
    for (int i = 0; i < list->list.count; i++) {
        object *item = list->list.items[i];
        if (otype(item) == 0) total += getival(item);
        else if (otype(item) == 1) total += item->fval;
        else throwexception("sum of non-number");
    }
    return makefloat(total);
}

object *builtinany(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("any needs iterable");
    if (otype(args[0]) == 5) {
        for (int i = 0; i < args[0]->list.count; i++) {
            if (istruthy(args[0]->list.items[i])) return makebool(1);
        }
        return makebool(0);
    }
    throwexception("argument not iterable");
    return NULL;
}

object *builtinall(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("all needs iterable");
    if (otype(args[0]) == 5) {
        for (int i = 0; i < args[0]->list.count; i++) {
            if (!istruthy(args[0]->list.items[i])) return makebool(0);
        }
        return makebool(1);
    }
    throwexception("argument not iterable");
    return NULL;
}

object *builtinsorted(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("sorted needs list");
    object *list = args[0];
    if (otype(list) != 5) throwexception("sorted expects list");
    object *result = makelist();
    gcaddroot(result);
    for (int i = 0; i < list->list.count; i++) listappend(result, list->list.items[i]);
    listsort(result);
    gcremoveroot(result);
    return result;
}

object *builtinenumerate(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("enumerate needs iterable");
    object *list = args[0];
    if (otype(list) != 5) throwexception("enumerate expects list");
    object *result = makelist();
    gcaddroot(result);
    for (int i = 0; i < list->list.count; i++) {
        object *pair = makelist();
        gcaddroot(pair);
        listappend(pair, makeint(i));
        listappend(pair, list->list.items[i]);
        listappend(result, pair);
        gcremoveroot(pair);
    }
    gcremoveroot(result);
    return result;
}

object *builtinzip(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("zip needs lists");
    int minlen = 0;
    for (int i = 0; i < argc; i++) {
        if (otype(args[i]) != 5) throwexception("zip expects lists");
        if (i == 0 || args[i]->list.count < minlen) minlen = args[i]->list.count;
    }
    object *result = makelist();
    gcaddroot(result);
    for (int i = 0; i < minlen; i++) {
        object *group = makelist();
        gcaddroot(group);
        for (int j = 0; j < argc; j++) {
            listappend(group, args[j]->list.items[i]);
        }
        listappend(result, group);
        gcremoveroot(group);
    }
    gcremoveroot(result);
    return result;
}

object *builtinmap(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("map needs function and list");
    if (otype(args[0]) != 7 && otype(args[0]) != 8) throwexception("first argument must be callable");
    if (otype(args[1]) != 5) throwexception("second argument must be list");
    object *result = makelist();
    gcaddroot(result);
    for (int i = 0; i < args[1]->list.count; i++) {
        object *mapped = callfunc(args[0], &args[1]->list.items[i], 1, env, NULL);
        listappend(result, mapped);
    }
    gcremoveroot(result);
    return result;
}

object *builtinfilter(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("filter needs function and list");
    if (otype(args[0]) != 7 && otype(args[0]) != 8) throwexception("first argument must be callable");
    if (otype(args[1]) != 5) throwexception("second argument must be list");
    object *result = makelist();
    gcaddroot(result);
    for (int i = 0; i < args[1]->list.count; i++) {
        object *filtered = callfunc(args[0], &args[1]->list.items[i], 1, env, NULL);
        if (istruthy(filtered)) listappend(result, args[1]->list.items[i]);
    }
    gcremoveroot(result);
    return result;
}

object *builtinreduce(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("reduce needs function and list");
    if (otype(args[0]) != 7 && otype(args[0]) != 8) throwexception("first argument must be callable");
    if (otype(args[1]) != 5) throwexception("second argument must be list");
    if (args[1]->list.count == 0) throwexception("reduce of empty list");
    object *result = args[1]->list.items[0];
    for (int i = 1; i < args[1]->list.count; i++) {
        object *argslist[2] = {result, args[1]->list.items[i]};
        result = callfunc(args[0], argslist, 2, env, NULL);
    }
    return result;
}

object *builtinexit(object **args, int argc, environment *env) {
    int code = 0;
    if (argc > 0 && otype(args[0]) == 0) code = getival(args[0]);
    platformexit(code);
    return makenone();
}

object *builtinsleep(object **args, int argc, environment *env) {
    double seconds = 1.0;
    if (argc > 0) {
        if (otype(args[0]) == 0) seconds = getival(args[0]);
        else if (otype(args[0]) == 1) seconds = args[0]->fval;
    }
    platformsleep(seconds);
    return makenone();
}

object *builtintime(object **args, int argc, environment *env) {
    return makefloat(platformtime());
}

object *builtinreadfile(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("readfile needs path");
    char *content = platformreadfile(args[0]->sval);
    if (!content) return makenone();
    return makestring(content);
}

object *builtinwritefile(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("writefile needs path and content");
    int result = platformwritefile(args[0]->sval, args[1]->sval);
    return makebool(result);
}

object *builtinopen(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("open needs path");
    char *path = args[0]->sval;
    char *mode = "r";
    if (argc > 1) mode = args[1]->sval;
    const char *fmode = "r";
    if (strcmp(mode, "r") == 0) fmode = "r";
    else if (strcmp(mode, "w") == 0) fmode = "w";
    else if (strcmp(mode, "a") == 0) fmode = "a";
    else if (strcmp(mode, "rb") == 0) fmode = "rb";
    else if (strcmp(mode, "wb") == 0) fmode = "wb";
    else fmode = "r";
    FILE *f = fopen(path, fmode);
    if (!f) {
        throwexception("cannot open file");
        return NULL;
    }
    return makefile(f, fileno(f), strcmp(mode, "r") == 0 ? 0 : 1);
}

object *builtinclose(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("close needs file");
    if (otype(args[0]) == 23 && args[0]->fileobj.file) {
        fclose(args[0]->fileobj.file);
        args[0]->fileobj.file = NULL;
    }
    return makenone();
}

object *builtinread(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("read needs file");
    if (otype(args[0]) != 23) throwexception("not a file");
    FILE *f = args[0]->fileobj.file;
    if (!f) throwexception("file is closed");
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = 0;
    return makestring(buf);
}

object *builtinwrite(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("write needs file and data");
    if (otype(args[0]) != 23) throwexception("not a file");
    if (otype(args[1]) != 2) throwexception("write expects string");
    FILE *f = args[0]->fileobj.file;
    if (!f) throwexception("file is closed");
    fprintf(f, "%s", args[1]->sval);
    fflush(f);
    return makeint(strlen(args[1]->sval));
}

object *builtinimport(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("import needs module name");
    char *name = args[0]->sval;
    char path[512];
    snprintf(path, sizeof(path), "%s.ax", name);
    char *source = platformreadfile(path);
    if (!source) {
        snprintf(path, sizeof(path), "lib/%s.ax", name);
        source = platformreadfile(path);
    }
    if (!source) {
        throwexception("module not found");
        return NULL;
    }
    token *toks = tokenize(source);
    stmt *prog = parsetokens(toks, tcount);
    environment *modenv = envnew(globalenv);
    evalprogram(prog, modenv);
    object *mod = makemodule(name, NULL);
    mod->module.exports = modenv;
    free(source);
    envset(env, name, mod, 0);
    return mod;
}

object *builtindir(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("dir needs object");
    object *obj = args[0];
    object *result = makelist();
    gcaddroot(result);
    if (otype(obj) == 9 && obj->klass.attrs) {
        environment *e = (environment*)obj->klass.attrs;
        for (int i = 0; i < e->cap; i++) {
            if (e->slots[i].used) listappend(result, makestring(e->slots[i].name));
        }
    }
    if (otype(obj) == 10 && obj->instance.attrs) {
        environment *e = (environment*)obj->instance.attrs;
        for (int i = 0; i < e->cap; i++) {
            if (e->slots[i].used) listappend(result, makestring(e->slots[i].name));
        }
    }
    gcremoveroot(result);
    return result;
}

object *builtinhasattr(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("hasattr needs object and name");
    if (otype(args[1]) != 2) throwexception("name must be string");
    return makebool(hasattr(args[0], args[1]->sval));
}

object *builtingetattr(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("getattr needs object and name");
    if (otype(args[1]) != 2) throwexception("name must be string");
    object *attr = getattr(args[0], args[1]->sval);
    if (!attr && argc > 2) return args[2];
    if (!attr) throwexception("attribute not found");
    return attr;
}

object *builtinsetattr(object **args, int argc, environment *env) {
    if (argc < 3) throwexception("setattr needs object, name, value");
    if (otype(args[1]) != 2) throwexception("name must be string");
    setattr(args[0], args[1]->sval, args[2]);
    return makenone();
}

object *builtinsuper(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("super needs instance");
    if (otype(args[0]) != 10) throwexception("super needs instance");
    object *cls = args[0]->instance.klass;
    if (otype(cls) != 9) throwexception("invalid class");
    object *supercls = cls->klass.bases;
    if (!supercls) supercls = envget(globalenv, "object");
    return supercls;
}

object *builtinchr(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("chr needs integer");
    if (otype(args[0]) != 0) throwexception("chr expects integer");
    char buf[2] = {(char)getival(args[0]), 0};
    return makestring(buf);
}

object *builtinord(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("ord needs character");
    if (otype(args[0]) != 2) throwexception("ord expects string");
    if (strlen(args[0]->sval) != 1) throwexception("ord expects single character");
    return makeint((unsigned char)args[0]->sval[0]);
}

object *builtinhex(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("hex needs integer");
    if (otype(args[0]) != 0) throwexception("hex expects integer");
    char buf[32];
    snprintf(buf, sizeof(buf), "0x%lx", getival(args[0]));
    return makestring(buf);
}

object *builtinoct(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("oct needs integer");
    if (otype(args[0]) != 0) throwexception("oct expects integer");
    char buf[32];
    snprintf(buf, sizeof(buf), "0o%lo", getival(args[0]));
    return makestring(buf);
}

object *builtinbin(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("bin needs integer");
    if (otype(args[0]) != 0) throwexception("bin expects integer");
    char buf[64];
    char *ptr = buf;
    long val = getival(args[0]);
    *ptr++ = '0';
    *ptr++ = 'b';
    for (int i = 63; i >= 0; i--) {
        if ((val >> i) & 1) *ptr++ = '1';
        else if (ptr > buf + 2) *ptr++ = '0';
    }
    if (ptr == buf + 2) *ptr++ = '0';
    *ptr = 0;
    return makestring(buf);
}

object *builtineval(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("eval needs expression");
    if (otype(args[0]) != 2) throwexception("eval expects string");
    token *toks = tokenize(args[0]->sval);
    stmt *prog = parsetokens(toks, tcount);
    return evalprogram(prog, env);
}

object *builtinexec(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("exec needs code");
    if (otype(args[0]) != 2) throwexception("exec expects string");
    token *toks = tokenize(args[0]->sval);
    stmt *prog = parsetokens(toks, tcount);
    evalprogram(prog, env);
    return makenone();
}

object *builtincompile(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("compile needs source");
    if (otype(args[0]) != 2) throwexception("compile expects string");
    token *toks = tokenize(args[0]->sval);
    stmt *prog = parsetokens(toks, tcount);
    return makenative(prog, NULL);
}

object *builtincallable(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("callable needs object");
    return makebool(otype(args[0]) == 7 || otype(args[0]) == 8 || otype(args[0]) == 9);
}

object *builtinisinstance(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("isinstance needs object and class");
    if (otype(args[0]) != 10) return makebool(0);
    if (otype(args[1]) != 9) throwexception("second argument must be class");
    object *cls = args[0]->instance.klass;
    while (cls) {
        if (cls == args[1]) return makebool(1);
        cls = cls->klass.bases;
    }
    return makebool(0);
}

object *builtinissubclass(object **args, int argc, environment *env) {
    if (argc < 2) throwexception("issubclass needs class and class");
    if (otype(args[0]) != 9 || otype(args[1]) != 9) throwexception("both arguments must be classes");
    object *cls = args[0];
    while (cls) {
        if (cls == args[1]) return makebool(1);
        cls = cls->klass.bases;
    }
    return makebool(0);
}

object *builtinid(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("id needs object");
    return makeint((long)args[0]);
}

object *builtinhash(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("hash needs object");
    if (otype(args[0]) == 0) return makeint(getival(args[0]));
    if (otype(args[0]) == 2) {
        unsigned long h = 0;
        for (char *p = args[0]->sval; *p; p++) h = h * 31 + *p;
        return makeint(h);
    }
    if (otype(args[0]) == 3) return makeint(getbval(args[0]) ? 1 : 0);
    throwexception("unhashable type");
    return NULL;
}

object *builtinrepr(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("repr needs object");
    return builtinstr(args, argc, env);
}

object *builtinascii(object **args, int argc, environment *env) {
    if (argc < 1) throwexception("ascii needs object");
    object *s = builtinstr(args, argc, env);
    char *result = malloc(strlen(s->sval) * 4 + 1);
    int ridx = 0;
    for (int i = 0; s->sval[i]; i++) {
        if ((unsigned char)s->sval[i] < 128) result[ridx++] = s->sval[i];
        else {
            ridx += snprintf(result + ridx, 16, "\\x%02x", (unsigned char)s->sval[i]);
        }
    }
    result[ridx] = 0;
    return makestring(result);
}

void registerstdlib(environment *env) {
    envset(env, "print", makebuiltin(builtinprint), 0);
    envset(env, "len", makebuiltin(builtinlen), 0);
    envset(env, "str", makebuiltin(builtinstr), 0);
    envset(env, "int", makebuiltin(builtinint), 0);
    envset(env, "float", makebuiltin(builtinfloat), 0);
    envset(env, "bool", makebuiltin(builtinbool), 0);
    envset(env, "input", makebuiltin(builtininput), 0);
    envset(env, "range", makebuiltin(builtinrange), 0);
    envset(env, "type", makebuiltin(builtintype), 0);
    envset(env, "abs", makebuiltin(builtinabs), 0);
    envset(env, "max", makebuiltin(builtinmax), 0);
    envset(env, "min", makebuiltin(builtinmin), 0);
    envset(env, "sum", makebuiltin(builtinsum), 0);
    envset(env, "any", makebuiltin(builtinany), 0);
    envset(env, "all", makebuiltin(builtinall), 0);
    envset(env, "sorted", makebuiltin(builtinsorted), 0);
    envset(env, "enumerate", makebuiltin(builtinenumerate), 0);
    envset(env, "zip", makebuiltin(builtinzip), 0);
    envset(env, "map", makebuiltin(builtinmap), 0);
    envset(env, "filter", makebuiltin(builtinfilter), 0);
    envset(env, "reduce", makebuiltin(builtinreduce), 0);
    envset(env, "exit", makebuiltin(builtinexit), 0);
    envset(env, "sleep", makebuiltin(builtinsleep), 0);
    envset(env, "time", makebuiltin(builtintime), 0);
    envset(env, "readfile", makebuiltin(builtinreadfile), 0);
    envset(env, "writefile", makebuiltin(builtinwritefile), 0);
    envset(env, "open", makebuiltin(builtinopen), 0);
    envset(env, "close", makebuiltin(builtinclose), 0);
    envset(env, "read", makebuiltin(builtinread), 0);
    envset(env, "write", makebuiltin(builtinwrite), 0);
    envset(env, "import", makebuiltin(builtinimport), 0);
    envset(env, "dir", makebuiltin(builtindir), 0);
    envset(env, "hasattr", makebuiltin(builtinhasattr), 0);
    envset(env, "getattr", makebuiltin(builtingetattr), 0);
    envset(env, "setattr", makebuiltin(builtinsetattr), 0);
    envset(env, "super", makebuiltin(builtinsuper), 0);
    envset(env, "chr", makebuiltin(builtinchr), 0);
    envset(env, "ord", makebuiltin(builtinord), 0);
    envset(env, "hex", makebuiltin(builtinhex), 0);
    envset(env, "oct", makebuiltin(builtinoct), 0);
    envset(env, "bin", makebuiltin(builtinbin), 0);
    envset(env, "eval", makebuiltin(builtineval), 0);
    envset(env, "exec", makebuiltin(builtinexec), 0);
    envset(env, "compile", makebuiltin(builtincompile), 0);
    envset(env, "callable", makebuiltin(builtincallable), 0);
    envset(env, "isinstance", makebuiltin(builtinisinstance), 0);
    envset(env, "issubclass", makebuiltin(builtinissubclass), 0);
    envset(env, "id", makebuiltin(builtinid), 0);
    envset(env, "hash", makebuiltin(builtinhash), 0);
    envset(env, "repr", makebuiltin(builtinrepr), 0);
    envset(env, "ascii", makebuiltin(builtinascii), 0);
    registeralllibs(env);
}
