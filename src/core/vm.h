#ifndef vm_h
#define vm_h

#include "axton.h"
#include "bytecode.h"

object *executebytecode(bytecode *bc, environment *env);

#endif
