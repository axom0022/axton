#include "../core/axton.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifndef _WIN32
#define httpsend(fd, data, len) send(fd, data, len, MSG_NOSIGNAL)
#else
#define httpsend(fd, data, len) send(fd, data, len, 0)
#endif

#define httpreqmax 65536
#define httproutemax 128

typedef struct {
    char method[16];
    char path[1024];
    object *fn;
} httproute;

static httproute httproutes[httproutemax];
static int httproutecount = 0;
static object *httpmainhandler = NULL;
static environment *httpenv = NULL;

static const char *httpstatustext(int status) {
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 304: return "Not Modified";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        case 502: return "Bad Gateway";
        default: return "OK";
    }
}

static void httplowercase(char *s) {
    for (; *s; s++) {
        if (*s >= 'A' && *s <= 'Z') *s += 32;
    }
}

static char *httptrim(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    char *end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n')) {
        *--end = 0;
    }
    return s;
}

static object *httpmakereq(char *method, char *path, char *query, object *headers, char *body) {
    object *req = makedict();
    dictset(req, makestring("method"), makestring(method));
    dictset(req, makestring("path"), makestring(path));
    dictset(req, makestring("query"), makestring(query));
    dictset(req, makestring("headers"), headers);
    dictset(req, makestring("body"), makestring(body));
    return req;
}

static void httpsendresponse(int fd, int status, const char *ctype, const char *body, object *extraheaders, const char *location) {
    int bodylen = body ? (int)strlen(body) : 0;
    char head[2048];
    int headlen = snprintf(head, sizeof(head),
        "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: close\r\n",
        status, httpstatustext(status), ctype ? ctype : "text/html; charset=utf-8", bodylen);
    if (location) {
        headlen += snprintf(head + headlen, sizeof(head) - headlen, "Location: %s\r\n", location);
    }
    if (extraheaders && extraheaders->type == 6) {
        for (int i = 0; i < extraheaders->dict.count; i++) {
            object *k = extraheaders->dict.keyvals[i];
            object *v = extraheaders->dict.vals[i];
            if (k && k->type == 2 && v && v->type == 2) {
                headlen += snprintf(head + headlen, sizeof(head) - headlen, "%s: %s\r\n", k->sval, v->sval);
            }
        }
    }
    snprintf(head + headlen, sizeof(head) - headlen, "\r\n");
    httpsend(fd, head, strlen(head));
    if (bodylen > 0) httpsend(fd, body, bodylen);
}

static void httprespond(int fd, object *res) {
    if (!res || res->type == 4) {
        httpsendresponse(fd, 204, "text/plain", "", NULL, NULL);
        return;
    }
    if (res->type == 2) {
        httpsendresponse(fd, 200, "text/html; charset=utf-8", res->sval, NULL, NULL);
        return;
    }
    if (res->type == 0) {
        char num[16];
        snprintf(num, sizeof(num), "%ld", res->ival);
        httpsendresponse(fd, (int)res->ival, "text/plain", num, NULL, NULL);
        return;
    }
    if (res->type == 6) {
        int status = 200;
        const char *ctype = "text/html; charset=utf-8";
        const char *body = "";
        const char *location = NULL;
        object *extraheaders = NULL;
        object *o;
        o = dictget(res, makestring("status"));
        if (o && o->type == 0) status = (int)o->ival;
        o = dictget(res, makestring("body"));
        if (o && o->type == 2) body = o->sval;
        o = dictget(res, makestring("contenttype"));
        if (o && o->type == 2) ctype = o->sval;
        o = dictget(res, makestring("location"));
        if (o && o->type == 2) location = o->sval;
        o = dictget(res, makestring("headers"));
        if (o && o->type == 6) extraheaders = o;
        httpsendresponse(fd, status, ctype, body, extraheaders, location);
        return;
    }
    httpsendresponse(fd, 200, "text/plain", "response", NULL, NULL);
}

static object *httpcallhandler(object *handler, object *req) {
    object *args[1];
    args[0] = req;
    gcaddroot(req);
    object *res = callfunc(handler, args, 1, httpenv, NULL);
    gcremoveroot(req);
    object *ex = catchexception();
    if (ex) {
        if (res) gcremoveroot(res);
        char msg[512];
        snprintf(msg, sizeof(msg), "handler error: %s", ex->type == 2 ? ex->sval : "exception");
        res = makestring(msg);
        object *r = makedict();
        dictset(r, makestring("status"), makeint(500));
        dictset(r, makestring("body"), res);
        dictset(r, makestring("contenttype"), makestring("text/plain; charset=utf-8"));
        return r;
    }
    if (!res) res = makenone();
    gcaddroot(res);
    return res;
}

static void httphandle(int fd) {
    char *buf = malloc(httpreqmax);
    if (!buf) { close(fd); return; }
    int total = 0;
    char *headerend = NULL;
    while (total < httpreqmax - 1) {
        int n = recv(fd, buf + total, httpreqmax - 1 - total, 0);
        if (n <= 0) break;
        total += n;
        buf[total] = 0;
        headerend = strstr(buf, "\r\n\r\n");
        if (headerend) break;
    }
    if (total <= 0 || !headerend) {
        httpsendresponse(fd, 400, "text/plain", "bad request", NULL, NULL);
        free(buf);
        close(fd);
        return;
    }
    int contentlength = 0;
    char method[16] = "";
    char target[1100] = "";
    char version[16] = "";
    char *lineend = strstr(buf, "\r\n");
    if (lineend) {
        *lineend = 0;
        char *sp1 = strchr(buf, ' ');
        if (sp1) {
            *sp1 = 0;
            char *sp2 = strchr(sp1 + 1, ' ');
            if (sp2) {
                *sp2 = 0;
                snprintf(method, sizeof(method), "%s", buf);
                snprintf(target, sizeof(target), "%s", sp1 + 1);
                snprintf(version, sizeof(version), "%s", sp2 + 1);
            }
        }
    }
    object *headers = makedict();
    gcaddroot(headers);
    char *cursor = lineend ? lineend + 2 : buf;
    while (cursor < headerend) {
        char *nextline = strstr(cursor, "\r\n");
        if (!nextline || nextline >= headerend) break;
        *nextline = 0;
        char *colon = strchr(cursor, ':');
        if (colon) {
            *colon = 0;
            char *name = httptrim(cursor);
            char *value = httptrim(colon + 1);
            httplowercase(name);
            object *v = makestring(value);
            dictset(headers, makestring(name), v);
            if (strcmp(name, "content-length") == 0) contentlength = atoi(value);
        }
        cursor = nextline + 2;
    }
    int bodystart = (int)(headerend - buf) + 4;
    while (total < bodystart + contentlength && total < httpreqmax - 1) {
        int n = recv(fd, buf + total, httpreqmax - 1 - total, 0);
        if (n <= 0) break;
        total += n;
    }
    buf[total] = 0;
    char *body = buf + bodystart;
    int bodylen = total - bodystart;
    if (bodylen < 0) bodylen = 0;
    if (bodylen > contentlength) bodylen = contentlength;
    char *bodycopy = malloc(bodylen + 1);
    memcpy(bodycopy, body, bodylen);
    bodycopy[bodylen] = 0;
    char path[1024] = "";
    char query[1024] = "";
    char *qmark = strchr(target, '?');
    if (qmark) {
        *qmark = 0;
        snprintf(path, sizeof(path), "%s", target);
        snprintf(query, sizeof(query), "%s", qmark + 1);
    } else {
        snprintf(path, sizeof(path), "%s", target);
    }
    object *req = httpmakereq(method, path, query, headers, bodycopy);
    gcremoveroot(headers);
    free(bodycopy);
    object *handler = NULL;
    for (int i = 0; i < httproutecount; i++) {
        if (strcmp(httproutes[i].method, method) == 0 && strcmp(httproutes[i].path, path) == 0) {
            handler = httproutes[i].fn;
            break;
        }
    }
    if (!handler) handler = httpmainhandler;
    if (!handler) {
        httpsendresponse(fd, 404, "text/html; charset=utf-8", "<h1>404 not found</h1>", NULL, NULL);
        free(buf);
        close(fd);
        return;
    }
    object *res = httpcallhandler(handler, req);
    httprespond(fd, res);
    gcremoveroot(res);
    free(buf);
    close(fd);
}

object *httpserve(object **a, int c, environment *env) {
    if (c < 1 || a[0]->type != 0) throwexception("serve needs port");
    int port = (int)a[0]->ival;
    if (c >= 2) httpmainhandler = a[1];
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverfd < 0) throwexception("socket failed");
    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(serverfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(serverfd);
        throwexception("bind failed (port in use?)");
    }
    if (listen(serverfd, 100) < 0) {
        close(serverfd);
        throwexception("listen failed");
    }
    printf("axton http server listening on http://localhost:%d\n", port);
    fflush(stdout);
    for (;;) {
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        int clientfd = accept(serverfd, NULL, NULL);
        if (clientfd < 0) continue;
        setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        httphandle(clientfd);
    }
    return makenone();
}

object *httpget(object **a, int c, environment *env) {
    if (c < 2 || a[0]->type != 2 || a[1]->type != 7 && a[1]->type != 8)
        throwexception("get needs path and handler");
    if (httproutecount >= httproutemax) throwexception("too many routes");
    snprintf(httproutes[httproutecount].method, 16, "GET");
    snprintf(httproutes[httproutecount].path, 1024, "%s", a[0]->sval);
    httproutes[httproutecount].fn = a[1];
    httproutecount++;
    return makenone();
}

object *httppost(object **a, int c, environment *env) {
    if (c < 2 || a[0]->type != 2 || a[1]->type != 7 && a[1]->type != 8)
        throwexception("post needs path and handler");
    if (httproutecount >= httproutemax) throwexception("too many routes");
    snprintf(httproutes[httproutecount].method, 16, "POST");
    snprintf(httproutes[httproutecount].path, 1024, "%s", a[0]->sval);
    httproutes[httproutecount].fn = a[1];
    httproutecount++;
    return makenone();
}

object *httpresponse(object **a, int c, environment *env) {
    if (c < 2) throwexception("response needs status and body");
    object *r = makedict();
    dictset(r, makestring("status"), a[0]->type == 0 ? a[0] : makeint(200));
    dictset(r, makestring("body"), a[1]->type == 2 ? a[1] : makestring(""));
    dictset(r, makestring("contenttype"),
        c >= 3 && a[2]->type == 2 ? a[2] : makestring("text/html; charset=utf-8"));
    return r;
}

object *httphtml(object **a, int c, environment *env) {
    if (c < 1) throwexception("html needs body");
    object *r = makedict();
    dictset(r, makestring("status"), makeint(200));
    dictset(r, makestring("body"), a[0]->type == 2 ? a[0] : makestring(""));
    dictset(r, makestring("contenttype"), makestring("text/html; charset=utf-8"));
    return r;
}

object *httpredirect(object **a, int c, environment *env) {
    if (c < 1 || a[0]->type != 2) throwexception("redirect needs location");
    object *r = makedict();
    dictset(r, makestring("status"), makeint(302));
    dictset(r, makestring("body"), makestring(""));
    dictset(r, makestring("location"), a[0]);
    return r;
}

static char *httpreqclient(const char *method, const char *url, const char *data) {
    char host[256] = "";
    char path[1024] = "/";
    int port = 80;
    const char *u = url;
    if (strncmp(u, "http://", 7) == 0) u += 7;
    const char *slash = strchr(u, '/');
    const char *colon = strchr(u, ':');
    if (colon && (!slash || colon < slash)) {
        snprintf(host, sizeof(host), "%.*s", (int)(colon - u), u);
        port = atoi(colon + 1);
        const char *pstart = slash ? slash : u + strlen(u);
        snprintf(path, sizeof(path), "%s", pstart);
    } else {
        if (slash) {
            snprintf(host, sizeof(host), "%.*s", (int)(slash - u), u);
            snprintf(path, sizeof(path), "%s", slash);
        } else {
            snprintf(host, sizeof(host), "%s", u);
        }
    }
    struct hostent *he = gethostbyname(host);
    if (!he) throwexception("host not found");
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throwexception("socket failed");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        throwexception("connect failed");
    }
    char req[2048];
    if (data) {
        snprintf(req, sizeof(req),
            "%s %s HTTP/1.0\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s",
            method, path, host, (int)strlen(data), data);
    } else {
        snprintf(req, sizeof(req),
            "%s %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
            method, path, host);
    }
    httpsend(fd, req, strlen(req));
    int cap = 65536;
    int total = 0;
    char *buf = malloc(cap);
    for (;;) {
        if (total >= cap - 1) {
            cap *= 2;
            buf = realloc(buf, cap);
        }
        int n = recv(fd, buf + total, cap - 1 - total, 0);
        if (n <= 0) break;
        total += n;
    }
    close(fd);
    buf[total] = 0;
    char *headerend = strstr(buf, "\r\n\r\n");
    char *body = headerend ? headerend + 4 : buf;
    char *result = strdup(body);
    free(buf);
    return result;
}

object *httpclientget(object **a, int c, environment *env) {
    if (c < 1 || a[0]->type != 2) throwexception("clientget needs url");
    char *body = httpreqclient("GET", a[0]->sval, NULL);
    object *res = makestring(body);
    free(body);
    return res;
}

object *httpclientpost(object **a, int c, environment *env) {
    if (c < 2 || a[0]->type != 2 || a[1]->type != 2) throwexception("clientpost needs url and data");
    char *body = httpreqclient("POST", a[0]->sval, a[1]->sval);
    object *res = makestring(body);
    free(body);
    return res;
}

void registerhttplib(environment *env) {
    httpenv = env;
    object *mod = makemodule("http", NULL);
    envset(mod->module.exports, "serve", makebuiltin(httpserve), 0);
    envset(mod->module.exports, "get", makebuiltin(httpget), 0);
    envset(mod->module.exports, "post", makebuiltin(httppost), 0);
    envset(mod->module.exports, "response", makebuiltin(httpresponse), 0);
    envset(mod->module.exports, "html", makebuiltin(httphtml), 0);
    envset(mod->module.exports, "redirect", makebuiltin(httpredirect), 0);
    envset(mod->module.exports, "clientget", makebuiltin(httpclientget), 0);
    envset(mod->module.exports, "clientpost", makebuiltin(httpclientpost), 0);
    envset(env, "http", mod, 0);
}
