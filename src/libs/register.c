#include "../core/axton.h"

void registerstdlib(environment *env);
void registermathlib(environment *env);
void registervectorlib(environment *env);
void registergraphicslib(environment *env);
void registerengine3dlib(environment *env);
void registerdatetime(environment *env);
void registerhashlib(environment *env);
void registersubprocess(environment *env);
void registersocket(environment *env);
void registerre(environment *env);
void registerjson(environment *env);
void registercsv(environment *env);
void registertempfile(environment *env);
void registerlogging(environment *env);
void registerargparse(environment *env);
void registersecrets(environment *env);
void registerglob(environment *env);
void registerfnmatch(environment *env);
void registerpprint(environment *env);
void registercopy(environment *env);
void registeritertools(environment *env);
void registercollections(environment *env);
void registeros(environment *env);
void registersys(environment *env);
void registerthreading(environment *env);

void registeralllibs(environment *env) {
    registerstdlib(env);
    registermathlib(env);
    registervectorlib(env);
    registergraphicslib(env);
    registerengine3dlib(env);
    registerdatetime(env);
    registerhashlib(env);
    registersubprocess(env);
    registersocket(env);
    registerre(env);
    registerjson(env);
    registercsv(env);
    registertempfile(env);
    registerlogging(env);
    registerargparse(env);
    registersecrets(env);
    registerglob(env);
    registerfnmatch(env);
    registerpprint(env);
    registercopy(env);
    registeritertools(env);
    registercollections(env);
    registeros(env);
    registersys(env);
    registerthreading(env);
}
