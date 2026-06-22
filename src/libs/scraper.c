#include "../core/axton.h"
#include <curl/curl.h>

typedef struct scrapersession {
    CURL *curl;
    char *useragent;
    char *cookies;
    int timeout;
} scrapersession;

static size_t writefunc(void *ptr, size_t size, size_t nmemb, void *userdata) {
    object *obj = (object*)userdata;
    size_t realsize = size * nmemb;
    char *data = malloc(realsize + 1);
    memcpy(data, ptr, realsize);
    data[realsize] = 0;
    listappend(obj, makestring(data));
    free(data);
    return realsize;
}

object *scrcreate(object **args, int argc, void *env) {
    object *session = makescraper(NULL);
    scrapersession *s = malloc(sizeof(scrapersession));
    s->curl = curl_easy_init();
    s->useragent = strdup("Mozilla/5.0");
    s->cookies = NULL;
    s->timeout = 30;
    session->native.data = s;
    return session;
}

object *scrget(object **args, int argc, void *env) {
    if (argc < 2) throwexception("scraper.get needs session and url");
    object *session = args[0];
    char *url = args[1]->sval;
    if (session->type != 115) throwexception("not a scraper session");
    scrapersession *s = (scrapersession*)session->native.data;
    curl_easy_setopt(s->curl, CURLOPT_URL, url);
    curl_easy_setopt(s->curl, CURLOPT_USERAGENT, s->useragent);
    if (s->cookies) curl_easy_setopt(s->curl, CURLOPT_COOKIE, s->cookies);
    curl_easy_setopt(s->curl, CURLOPT_TIMEOUT, s->timeout);
    curl_easy_setopt(s->curl, CURLOPT_FOLLOWLOCATION, 1);
    object *data = makelist();
    curl_easy_setopt(s->curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(s->curl, CURLOPT_WRITEDATA, data);
    CURLcode res = curl_easy_perform(s->curl);
    if (res != CURLE_OK) throwexception(curl_easy_strerror(res));
    return data;
}

object *scrpost(object **args, int argc, void *env) {
    if (argc < 3) throwexception("scraper.post needs session, url, body");
    object *session = args[0];
    char *url = args[1]->sval;
    char *body = args[2]->sval;
    if (session->type != 115) throwexception("not a scraper session");
    scrapersession *s = (scrapersession*)session->native.data;
    curl_easy_setopt(s->curl, CURLOPT_URL, url);
    curl_easy_setopt(s->curl, CURLOPT_POST, 1);
    curl_easy_setopt(s->curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(s->curl, CURLOPT_USERAGENT, s->useragent);
    if (s->cookies) curl_easy_setopt(s->curl, CURLOPT_COOKIE, s->cookies);
    curl_easy_setopt(s->curl, CURLOPT_TIMEOUT, s->timeout);
    curl_easy_setopt(s->curl, CURLOPT_FOLLOWLOCATION, 1);
    object *data = makelist();
    curl_easy_setopt(s->curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(s->curl, CURLOPT_WRITEDATA, data);
    CURLcode res = curl_easy_perform(s->curl);
    if (res != CURLE_OK) throwexception(curl_easy_strerror(res));
    return data;
}

object *scrsetuseragent(object **args, int argc, void *env) {
    if (argc < 2) throwexception("scraper.setuseragent needs session and ua");
    object *session = args[0];
    char *ua = args[1]->sval;
    if (session->type != 115) throwexception("not a scraper session");
    scrapersession *s = (scrapersession*)session->native.data;
    free(s->useragent);
    s->useragent = strdup(ua);
    return makenone();
}

object *scrsetcookie(object **args, int argc, void *env) {
    if (argc < 2) throwexception("scraper.setcookie needs session and cookie");
    object *session = args[0];
    char *cookie = args[1]->sval;
    if (session->type != 115) throwexception("not a scraper session");
    scrapersession *s = (scrapersession*)session->native.data;
    free(s->cookies);
    s->cookies = strdup(cookie);
    return makenone();
}

void registerscriptlib(environment *env) {
    object *mod = makemodule("scraper", NULL);
    envset(mod->module.exports, "create", makebuiltin(scrcreate), 0);
    envset(mod->module.exports, "get", makebuiltin(scrget), 0);
    envset(mod->module.exports, "post", makebuiltin(scrpost), 0);
    envset(mod->module.exports, "setuseragent", makebuiltin(scrsetuseragent), 0);
    envset(mod->module.exports, "setcookie", makebuiltin(scrsetcookie), 0);
    envset(env, "scraper", mod, 0);
}
