#include "../core/axton.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

static int serverfd = -1;
static int serverport = 0;
static object *routes = NULL;

object *httpserve(object **a, int c, void *e) {
    if (c < 2) throwexception("serve needs port and handler");
    int port = a[0]->ival;
    object *handler = a[1];
    serverport = port;
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(serverfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(serverfd, 100);
    routes = handler;
    return makenone();
}

object *httpget(object **a, int c, void *e) {
    if (c < 2) throwexception("get needs path and handler");
    return makenone();
}

object *httppost(object **a, int c, void *e) {
    if (c < 2) throwexception("post needs path and handler");
    return makenone();
}

object *httpclientget(object **a, int c, void *e) {
    if (c < 1) throwexception("clientget needs url");
    return makestring("response");
}

object *httpclientpost(object **a, int c, void *e) {
    if (c < 2) throwexception("clientpost needs url and data");
    return makestring("response");
}

void registerhttplib(environment *env) {
    object *mod = makemodule("http", NULL);
    envset(mod->module.exports, "serve", makebuiltin(httpserve), 0);
    envset(mod->module.exports, "get", makebuiltin(httpget), 0);
    envset(mod->module.exports, "post", makebuiltin(httppost), 0);
    envset(mod->module.exports, "clientget", makebuiltin(httpclientget), 0);
    envset(mod->module.exports, "clientpost", makebuiltin(httpclientpost), 0);
    envset(env, "http", mod, 0);
}
