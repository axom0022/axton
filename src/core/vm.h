#ifndef VM_H
#define VM_H

#include "axton.h"
#include "bytecode.h"

object *executebytecode(bytecode *bc, environment *env);

#endif
