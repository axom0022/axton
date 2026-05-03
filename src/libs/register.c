#include "../core/axton.h"

void registermathlib(environment *env);
void registertensorlib(environment *env);
void registerguilib(environment *env);
void registerdblib(environment *env);
void registerasynclib(environment *env);
void registerpackagelib(environment *env);
void registerweblib(environment *env);

void registeralllibs(environment *env) {
    registermathlib(env);
    registertensorlib(env);
    registerguilib(env);
    registerdblib(env);
    registerasynclib(env);
    registerpackagelib(env);
    registerweblib(env);
}
aa
