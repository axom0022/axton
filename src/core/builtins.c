#include "axton.h"

object *builtinprint(object **args, int argc, environment *env) {
    for (int i = 0; i < argc; i++) {
        if (args[i]->type == 2) platformlog(args[i]->sval);
        else if (args[i]->type == 0) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%ld", args[i]->ival);
            platformlog(buf);
        }
        else if (args[i]->type == 1) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", args[i]->fval);
            platformlog(buf);
        }
        else if (args[i]->type == 3) platformlog(args[i]->bval ? "true" : "false");
        else platformlog("none");
        if (i < argc - 1) platformlog(" ");
    }
    platformlog("\n");
    return makenone();
}

object *builtinlen(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("len needs 1 argument");
    if (args[0]->type == 2) return makeint(strlen(args[0]->sval));
    if (args[0]->type == 5) return makeint(args[0]->list.count);
    if (args[0]->type == 6) return makeint(args[0]->dict.count);
    throwexception("object has no length");
    return NULL;
}

object *builtinstr(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("str needs 1 argument");
    char buf[256];
    switch (args[0]->type) {
        case 0: snprintf(buf, sizeof(buf), "%ld", args[0]->ival); break;
        case 1: snprintf(buf, sizeof(buf), "%g", args[0]->fval); break;
        case 2: return makestring(args[0]->sval);
        case 3: return makestring(args[0]->bval ? "true" : "false");
        case 4: return makestring("none");
        default: return makestring("object");
    }
    return makestring(buf);
}

object *builtinint(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("int needs 1 argument");
    if (args[0]->type == 0) return makeint(args[0]->ival);
    if (args[0]->type == 1) return makeint((long)args[0]->fval);
    if (args[0]->type == 2) return makeint(atol(args[0]->sval));
    throwexception("cannot convert to int");
    return NULL;
}

object *builtinfloat(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("float needs 1 argument");
    if (args[0]->type == 1) return makefloat(args[0]->fval);
    if (args[0]->type == 0) return makefloat((double)args[0]->ival);
    if (args[0]->type == 2) return makefloat(atof(args[0]->sval));
    throwexception("cannot convert to float");
    return NULL;
}

object *builtininput(object **args, int argc, environment *env) {
    if (argc > 0) builtinprint(args, argc, env);
    char buf[1024];
    if (!fgets(buf, sizeof(buf), stdin)) return makestring("");
    buf[strcspn(buf, "\n")] = 0;
    return makestring(buf);
}

object *builtinrange(object **args, int argc, environment *env) {
    long start = 0, stop = 0, step = 1;
    if (argc == 1) {
        if (args[0]->type != 0) throwexception("range needs integer");
        stop = args[0]->ival;
    } else if (argc == 2) {
        if (args[0]->type != 0 || args[1]->type != 0) throwexception("range needs integer");
        start = args[0]->ival;
        stop = args[1]->ival;
    } else if (argc == 3) {
        if (args[0]->type != 0 || args[1]->type != 0 || args[2]->type != 0)
            throwexception("range needs integer");
        start = args[0]->ival;
        stop = args[1]->ival;
        step = args[2]->ival;
    } else {
        throwexception("range needs 1-3 arguments");
    }
    return makerange(start, stop, step);
}

object *builtintype(object **args, int argc, environment *env) {
    if (argc != 1) throwexception("type needs 1 argument");
    switch (args[0]->type) {
        case 0: return makestring("int");
        case 1: return makestring("float");
        case 2: return makestring("str");
        case 3: return makestring("bool");
        case 4: return makestring("none");
        case 5: return makestring("list");
        case 6: return makestring("dict");
        case 7: return makestring("function");
        case 9: return makestring("class");
        case 20: return makestring("tensor");
        default: return makestring("object");
    }
}

object *builtinexit(object **args, int argc, environment *env) {
    exit(0);
    return makenone();
}

object *builtinsleep(object **args, int argc, environment *env) {
    double seconds = 1.0;
    if (argc > 0) {
        if (args[0]->type == 0) seconds = args[0]->ival;
        else if (args[0]->type == 1) seconds = args[0]->fval;
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

void registerstdlib(environment *env) {
    envset(env, "print", makebuiltin(builtinprint), 0);
    envset(env, "len", makebuiltin(builtinlen), 0);
    envset(env, "str", makebuiltin(builtinstr), 0);
    envset(env, "int", makebuiltin(builtinint), 0);
    envset(env, "float", makebuiltin(builtinfloat), 0);
    envset(env, "input", makebuiltin(builtininput), 0);
    envset(env, "range", makebuiltin(builtinrange), 0);
    envset(env, "type", makebuiltin(builtintype), 0);
    envset(env, "exit", makebuiltin(builtinexit), 0);
    envset(env, "sleep", makebuiltin(builtinsleep), 0);
    envset(env, "time", makebuiltin(builtintime), 0);
    envset(env, "readfile", makebuiltin(builtinreadfile), 0);
    envset(env, "writefile", makebuiltin(builtinwritefile), 0);
}
