#include "../core/axton.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

static void *lspserverthread(void *arg) {
    int port = (int)(long)arg;
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(srv, (struct sockaddr*)&addr, sizeof(addr));
    listen(srv, 1);
    while (1) {
        int client = accept(srv, NULL, NULL);
        if (client < 0) continue;
        char buf[4096];
        int n = recv(client, buf, sizeof(buf)-1, 0);
        if (n > 0) {
            buf[n] = 0;
            const char *resp = "Content-Length: 58\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":null,\"id\":null}";
            send(client, resp, strlen(resp), 0);
        }
        close(client);
    }
    close(srv);
    return NULL;
}

object *lspstart(object **args, int argc, void *env) {
    int port = argc > 0 ? args[0]->ival : 8080;
    pthread_t thread;
    pthread_create(&thread, NULL, lspserverthread, (void*)(long)port);
    pthread_detach(thread);
    return makelsp(NULL, port, 1);
}

void registerlsplib(environment *env) {
    object *mod = makemodule("lsp", NULL);
    envset(mod->module.exports, "start", makebuiltin(lspstart), 0);
    envset(env, "lsp", mod, 0);
}
