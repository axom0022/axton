#include "../core/axton.h"

void registermathlib(environment *env);
void registertensorlib(environment *env);
void registerguilib(environment *env);
void registerdblib(environment *env);
void registerasynclib(environment *env);
void registerpackagelib(environment *env);
void registerweblib(environment *env);
void registeroslib(environment *env);
void registersyslib(environment *env);
void registerjsonlib(environment *env);
void registerrelib(environment *env);
void registerthreadlib(environment *env);
void registercollectionslib(environment *env);
void registeritertools(environment *env);
void registerhttplib(environment *env);
void registergui2lib(environment *env);

void registeralllibs(environment *env) {
    registermathlib(env);
    registertensorlib(env);
    registerguilib(env);
    registerdblib(env);
    registerasynclib(env);
    registerpackagelib(env);
    registerweblib(env);
    registeroslib(env);
    registersyslib(env);
    registerjsonlib(env);
    registerrelib(env);
    registerthreadlib(env);
    registercollectionslib(env);
    registeritertools(env);
    registerhttplib(env);
    registergui2lib(env);
}
