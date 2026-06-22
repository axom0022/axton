#include "../core/axton.h"

object *processtarget(object **args, int argc, void *env) {
    if (argc < 1) throwexception("target needs pid or name");
    object *arg = args[0];
    int pid = -1;
    if (arg->type == 0) {
        pid = arg->ival;
    } else if (arg->type == 2) {
        pid = platformprocessfind(arg->sval);
        if (pid < 0) throwexception("process not found");
    } else {
        throwexception("target must be int or string");
    }
    int handle = platformprocessopen(pid, 0);
    if (handle < 0) throwexception("cannot open process");
    object *p = makeprocesshandle(handle, pid);
    return p;
}

object *processread(object **args, int argc, void *env) {
    if (argc < 2) throwexception("read needs process and address");
    object *proc = args[0];
    if (proc->type != 90) throwexception("not a process handle");
    long addr = args[1]->ival;
    int size = 4;
    if (argc > 2) size = args[2]->ival;
    unsigned char *buf = malloc(size);
    int res = platformprocessread(proc->processhandle.handle, addr, buf, size);
    if (res < 0) { free(buf); throwexception("read failed"); }
    char *hex = malloc(size*2+1);
    for (int i=0;i<size;i++) snprintf(hex+i*2,3,"%02x",buf[i]);
    object *result = makestring(hex);
    free(buf); free(hex);
    return result;
}

object *processwrite(object **args, int argc, void *env) {
    if (argc < 3) throwexception("write needs process, address, data");
    object *proc = args[0];
    if (proc->type != 90) throwexception("not a process handle");
    long addr = args[1]->ival;
    char *data = args[2]->sval;
    int len = strlen(data);
    unsigned char *buf = malloc(len/2);
    for (int i=0;i<len/2;i++) {
        sscanf(data + i*2, "%2hhx", &buf[i]);
    }
    int res = platformprocesswrite(proc->processhandle.handle, addr, buf, len/2);
    free(buf);
    if (res < 0) throwexception("write failed");
    return makenone();
}

object *processclose(object **args, int argc, void *env) {
    if (argc < 1) throwexception("close needs process");
    object *proc = args[0];
    if (proc->type != 90) throwexception("not a process handle");
    platformprocessclose(proc->processhandle.handle);
    proc->processhandle.handle = -1;
    return makenone();
}

object *processpid(object **args, int argc, void *env) {
    if (argc < 1) throwexception("pid needs process");
    object *proc = args[0];
    if (proc->type != 90) throwexception("not a process handle");
    return makeint(proc->processhandle.pid);
}

object *processfind(object **args, int argc, void *env) {
    if (argc < 1) throwexception("find needs process name");
    char *name = args[0]->sval;
    int pid = platformprocessfind(name);
    if (pid < 0) return makenone();
    return makeint(pid);
}

void registerprocesslib(environment *env) {
    object *mod = makemodule("process", NULL);
    envset(mod->module.exports, "target", makebuiltin(processtarget), 0);
    envset(mod->module.exports, "read", makebuiltin(processread), 0);
    envset(mod->module.exports, "write", makebuiltin(processwrite), 0);
    envset(mod->module.exports, "close", makebuiltin(processclose), 0);
    envset(mod->module.exports, "pid", makebuiltin(processpid), 0);
    envset(mod->module.exports, "find", makebuiltin(processfind), 0);
    envset(env, "process", mod, 0);
}
