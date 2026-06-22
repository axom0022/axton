#include "../core/axton.h"

object *memalloc(object **args, int argc, void *env) {
    if (argc < 1) throwexception("alloc needs size");
    int size = args[0]->ival;
    void *ptr = platformallocate(size);
    if (!ptr) throwexception("allocation failed");
    object *block = makememoryblock(size);
    block->memblock.memory = ptr;
    block->memblock.size = size;
    block->memblock.used = 0;
    return block;
}

object *memfree(object **args, int argc, void *env) {
    if (argc < 1) throwexception("free needs memory block");
    object *block = args[0];
    if (block->type != 78) throwexception("not a memory block");
    platformdeallocate(block->memblock.memory);
    block->memblock.memory = NULL;
    block->memblock.size = 0;
    return makenone();
}

object *memread(object **args, int argc, void *env) {
    if (argc < 3) throwexception("read needs block offset size");
    object *block = args[0];
    if (block->type != 78) throwexception("not a memory block");
    int offset = args[1]->ival;
    int size = args[2]->ival;
    if (offset + size > block->memblock.size) throwexception("out of bounds");
    char *buf = malloc(size + 1);
    memcpy(buf, (char*)block->memblock.memory + offset, size);
    buf[size] = 0;
    object *result = makestring(buf);
    free(buf);
    return result;
}

object *memwrite(object **args, int argc, void *env) {
    if (argc < 3) throwexception("write needs block offset data");
    object *block = args[0];
    if (block->type != 78) throwexception("not a memory block");
    int offset = args[1]->ival;
    char *data = args[2]->sval;
    int size = strlen(data);
    if (offset + size > block->memblock.size) throwexception("out of bounds");
    memcpy((char*)block->memblock.memory + offset, data, size);
    return makenone();
}

object *memgetptr(object **args, int argc, void *env) {
    if (argc < 1) throwexception("getptr needs block");
    object *block = args[0];
    if (block->type != 78) throwexception("not a memory block");
    return makeint((long)block->memblock.memory);
}

object *memgetsize(object **args, int argc, void *env) {
    if (argc < 1) throwexception("getsize needs block");
    object *block = args[0];
    if (block->type != 78) throwexception("not a memory block");
    return makeint(block->memblock.size);
}

object *memrealloc(object **args, int argc, void *env) {
    if (argc < 2) throwexception("realloc needs block and newsize");
    object *block = args[0];
    if (block->type != 78) throwexception("not a memory block");
    int newsize = args[1]->ival;
    void *ptr = platformreallocate(block->memblock.memory, newsize);
    if (!ptr) throwexception("reallocation failed");
    block->memblock.memory = ptr;
    block->memblock.size = newsize;
    return block;
}

void registermemorylib(environment *env) {
    object *mod = makemodule("mem", NULL);
    envset(mod->module.exports, "alloc", makebuiltin(memalloc), 0);
    envset(mod->module.exports, "free", makebuiltin(memfree), 0);
    envset(mod->module.exports, "read", makebuiltin(memread), 0);
    envset(mod->module.exports, "write", makebuiltin(memwrite), 0);
    envset(mod->module.exports, "getptr", makebuiltin(memgetptr), 0);
    envset(mod->module.exports, "getsize", makebuiltin(memgetsize), 0);
    envset(mod->module.exports, "realloc", makebuiltin(memrealloc), 0);
    envset(env, "mem", mod, 0);
}
