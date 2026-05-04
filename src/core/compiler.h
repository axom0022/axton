#ifndef COMPILER_H
#define COMPILER_H

#include "axton.h"
#include "bytecode.h"

void compileprogram(stmt *prog, bytecode *bc);

#endif
