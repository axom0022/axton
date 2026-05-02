#include "axton.h"

object *builtin_print(object **args, int argc, environment *env) {
    for (int i = 0; i < argc; i++) {
        if (args[i]->type == 2) platform_log(args[i]->sval);
        else if (args[i]->type == 0) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%ld", args[i]->ival);
            platform_log(buf);
        }
        else if (args[i]->type == 1) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%g", args[i]->fval);
            platform_log(buf);
        }
        else if (args[i]->type == 3) platform_log(args[i]->bval ? "true" : "false");
        else platform_log("none");
        if (i < argc - 1) platform_log(" ");
    }
    platform_log("\n");
    return make_none();
}

object *builtin_len(object **args, int argc, environment *env) {
    if (argc != 1) throw_exception("len needs 1 argument");
    if (args[0]->type == 2) return make_int(strlen(args[0]->sval));
    if (args[0]->type == 5) return make_int(args[0]->list.count);
    if (args[0]->type == 6) return make_int(args[0]->dict.count);
    throw_exception("object has no length");
    return NULL;
}

object *builtin_str(object **args, int argc, environment *env) {
    if (argc != 1) throw_exception("str needs 1 argument");
    char buf[256];
    switch (args[0]->type) {
        case 0: snprintf(buf, sizeof(buf), "%ld", args[0]->ival); break;
        case 1: snprintf(buf, sizeof(buf), "%g", args[0]->fval); break;
        case 2: return make_string(args[0]->sval);
        case 3: return make_string(args[0]->bval ? "true" : "false");
        case 4: return make_string("none");
        default: return make_string("object");
    }
    return make_string(buf);
}

object *builtin_int(object **args, int argc, environment *env) {
    if (argc != 1) throw_exception("int needs 1 argument");
    if (args[0]->type == 0) return make_int(args[0]->ival);
    if (args[0]->type == 1) return make_int((long)args[0]->fval);
    if (args[0]->type == 2) return make_int(atol(args[0]->sval));
    throw_exception("cannot convert to int");
    return NULL;
}

object *builtin_float(object **args, int argc, environment *env) {
    if (argc != 1) throw_exception("float needs 1 argument");
    if (args[0]->type == 1) return make_float(args[0]->fval);
    if (args[0]->type == 0) return make_float((double)args[0]->ival);
    if (args[0]->type == 2) return make_float(atof(args[0]->sval));
    throw_exception("cannot convert to float");
    return NULL;
}

object *builtin_input(object **args, int argc, environment *env) {
    if (argc > 0) builtin_print(args, argc, env);
    char buf[1024];
    if (!fgets(buf, sizeof(buf), stdin)) return make_string("");
    buf[strcspn(buf, "\n")] = 0;
    return make_string(buf);
}

object *builtin_range(object **args, int argc, environment *env) {
    long start = 0, stop = 0, step = 1;
    if (argc == 1) {
        if (args[0]->type != 0) throw_exception("range needs integer");
        stop = args[0]->ival;
    } else if (argc == 2) {
        if (args[0]->type != 0 || args[1]->type != 0) throw_exception("range needs integer");
        start = args[0]->ival;
        stop = args[1]->ival;
    } else if (argc == 3) {
        if (args[0]->type != 0 || args[1]->type != 0 || args[2]->type != 0)
            throw_exception("range needs integer");
        start = args[0]->ival;
        stop = args[1]->ival;
        step = args[2]->ival;
    } else {
        throw_exception("range needs 1-3 arguments");
    }
    return make_range(start, stop, step);
}

object *builtin_type(object **args, int argc, environment *env) {
    if (argc != 1) throw_exception("type needs 1 argument");
    switch (args[0]->type) {
        case 0: return make_string("int");
        case 1: return make_string("float");
        case 2: return make_string("str");
        case 3: return make_string("bool");
        case 4: return make_string("none");
        case 5: return make_string("list");
        case 6: return make_string("dict");
        case 7: return make_string("function");
        case 9: return make_string("class");
        default: return make_string("object");
    }
}

object *builtin_exit(object **args, int argc, environment *env) {
    exit(0);
    return make_none();
}

object *builtin_sleep(object **args, int argc, environment *env) {
    double seconds = 1.0;
    if (argc > 0) {
        if (args[0]->type == 0) seconds = args[0]->ival;
        else if (args[0]->type == 1) seconds = args[0]->fval;
    }
    platform_sleep(seconds);
    return make_none();
}

object *builtin_time(object **args, int argc, environment *env) {
    return make_float(platform_time());
}

object *builtin_readfile(object **args, int argc, environment *env) {
    if (argc < 1) throw_exception("readfile needs path");
    char *content = platform_read_file(args[0]->sval);
    if (!content) return make_none();
    return make_string(content);
}

object *builtin_writefile(object **args, int argc, environment *env) {
    if (argc < 2) throw_exception("writefile needs path and content");
    int result = platform_write_file(args[0]->sval, args[1]->sval);
    return make_bool(result);
}

void register_stdlib(environment *env) {
    env_set(env, "print", make_builtin(builtin_print), 0);
    env_set(env, "len", make_builtin(builtin_len), 0);
    env_set(env, "str", make_builtin(builtin_str), 0);
    env_set(env, "int", make_builtin(builtin_int), 0);
    env_set(env, "float", make_builtin(builtin_float), 0);
    env_set(env, "input", make_builtin(builtin_input), 0);
    env_set(env, "range", make_builtin(builtin_range), 0);
    env_set(env, "type", make_builtin(builtin_type), 0);
    env_set(env, "exit", make_builtin(builtin_exit), 0);
    env_set(env, "sleep", make_builtin(builtin_sleep), 0);
    env_set(env, "time", make_builtin(builtin_time), 0);
    env_set(env, "readfile", make_builtin(builtin_readfile), 0);
    env_set(env, "writefile", make_builtin(builtin_writefile), 0);
}
