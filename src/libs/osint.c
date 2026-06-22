#include "../core/axton.h"
#include <curl/curl.h>
#include <regex.h>
#include <string.h>
#include <jpeglib.h>
#include <png.h>
#include <exif.h>
#include <unistd.h>

typedef struct osintctx {
    object *apikeys;
    object *useragent;
    int timeout;
    object *results;
} osintctx;

static size_t writecb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    object *list = (object*)userdata;
    size_t realsize = size * nmemb;
    char *data = malloc(realsize + 1);
    memcpy(data, ptr, realsize);
    data[realsize] = 0;
    listappend(list, makestring(data));
    free(data);
    return realsize;
}

static object *httpget(char *url, object *headers) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;
    object *data = makelist();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    if (headers) {
        struct curl_slist *slist = NULL;
        object *keys = dictkeys(headers);
        for (int i = 0; i < keys->list.count; i++) {
            char *key = keys->list.items[i]->sval;
            object *val = dictget(headers, keys->list.items[i]);
            char *header = malloc(strlen(key) + strlen(val->sval) + 3);
            sprintf(header, "%s: %s", key, val->sval);
            slist = curl_slist_append(slist, header);
            free(header);
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
    }
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return NULL;
    }
    curl_easy_cleanup(curl);
    return data;
}

object *osintcreate(object **args, int argc, void *env) {
    object *ctx = makeosint(NULL);
    osintctx *o = malloc(sizeof(osintctx));
    o->apikeys = makedict();
    o->useragent = makestring("Mozilla/5.0");
    o->timeout = 30;
    o->results = makelist();
    ctx->native.data = o;
    return ctx;
}

object *osintsetapikey(object **args, int argc, void *env) {
    if (argc < 3) throwexception("osint.setapikey needs ctx service key");
    object *ctx = args[0];
    char *service = args[1]->sval;
    char *key = args[2]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    dictset(o->apikeys, makestring(service), makestring(key));
    return makenone();
}

static object *extractprofiles(char *html, char *pattern) {
    object *profiles = makelist();
    regex_t re;
    regcomp(&re, pattern, REG_EXTENDED);
    regmatch_t matches[10];
    char *p = html;
    while (regexec(&re, p, 10, matches, 0) == 0) {
        int start = matches[1].rm_so;
        int end = matches[1].rm_eo;
        char *found = malloc(end - start + 1);
        memcpy(found, p + start, end - start);
        found[end - start] = 0;
        listappend(profiles, makestring(found));
        free(found);
        p += end;
    }
    regfree(&re);
    return profiles;
}

object *osintusername(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.username needs ctx and username");
    object *ctx = args[0];
    char *username = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    object *results = makelist();
    char *sites[] = {"github", "twitter", "instagram", "facebook", "reddit", "youtube", "tiktok", "linkedin", "pinterest", "twitch", "discord", "telegram", "snapchat", "tumblr", "flickr", "deviantart", "behance", "dribbble", "soundcloud", "spotify", "medium", "wordpress", "blogger", "tumblr", "quora", "stackoverflow", "hackernews", "producthunt", "gitlab", "bitbucket", NULL};
    for (int i = 0; sites[i]; i++) {
        char url[256];
        snprintf(url, sizeof(url), "https://%s.com/%s", sites[i], username);
        object *data = httpget(url, NULL);
        if (data && data->list.count > 0) {
            char *html = data->list.items[0]->sval;
            if (strstr(html, "404") == NULL && strstr(html, "not found") == NULL) {
                object *entry = makedict();
                dictset(entry, makestring("site"), makestring(sites[i]));
                dictset(entry, makestring("url"), makestring(url));
                listappend(results, entry);
            }
        }
    }
    o->results = results;
    return results;
}

object *osintemail(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.email needs ctx and email");
    object *ctx = args[0];
    char *email = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    object *results = makedict();
    char url[256];
    snprintf(url, sizeof(url), "https://haveibeenpwned.com/api/v3/breachedaccount/%s", email);
    object *data = httpget(url, NULL);
    if (data) {
        char *html = data->list.items[0]->sval;
        object *breaches = extractprofiles(html, "\"Name\":\"([^\"]+)\"");
        dictset(results, makestring("breaches"), breaches);
    }
    dictset(results, makestring("email"), makestring(email));
    return results;
}

object *osintphone(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.phone needs ctx and phone");
    object *ctx = args[0];
    char *phone = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    object *results = makedict();
    char url[256];
    snprintf(url, sizeof(url), "https://api.phonevalidation.com/validate?number=%s&key=APIKEY", phone);
    object *data = httpget(url, NULL);
    if (data) {
        char *html = data->list.items[0]->sval;
        regex_t re;
        regcomp(&re, "\"country\":\"([^\"]+)\"", REG_EXTENDED);
        regmatch_t m[2];
        if (regexec(&re, html, 2, m, 0) == 0) {
            char *country = malloc(m[1].rm_eo - m[1].rm_so + 1);
            memcpy(country, html + m[1].rm_so, m[1].rm_eo - m[1].rm_so);
            country[m[1].rm_eo - m[1].rm_so] = 0;
            dictset(results, makestring("country"), makestring(country));
            free(country);
        }
        regfree(&re);
    }
    dictset(results, makestring("phone"), makestring(phone));
    return results;
}

object *osintdomain(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.domain needs ctx and domain");
    object *ctx = args[0];
    char *domain = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    object *results = makedict();
    char url[256];
    snprintf(url, sizeof(url), "https://whois.icann.org/api/domain/%s", domain);
    object *data = httpget(url, NULL);
    if (data) {
        char *html = data->list.items[0]->sval;
        regex_t re;
        regcomp(&re, "\"registrant\":\"([^\"]+)\"", REG_EXTENDED);
        regmatch_t m[2];
        if (regexec(&re, html, 2, m, 0) == 0) {
            char *registrant = malloc(m[1].rm_eo - m[1].rm_so + 1);
            memcpy(registrant, html + m[1].rm_so, m[1].rm_eo - m[1].rm_so);
            registrant[m[1].rm_eo - m[1].rm_so] = 0;
            dictset(results, makestring("registrant"), makestring(registrant));
            free(registrant);
        }
        regfree(&re);
    }
    dictset(results, makestring("domain"), makestring(domain));
    return results;
}

object *osintip(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.ip needs ctx and ip");
    object *ctx = args[0];
    char *ip = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    object *results = makedict();
    char url[256];
    snprintf(url, sizeof(url), "http://ip-api.com/json/%s", ip);
    object *data = httpget(url, NULL);
    if (data) {
        char *html = data->list.items[0]->sval;
        regex_t re;
        regcomp(&re, "\"city\":\"([^\"]+)\"", REG_EXTENDED);
        regmatch_t m[2];
        if (regexec(&re, html, 2, m, 0) == 0) {
            char *city = malloc(m[1].rm_eo - m[1].rm_so + 1);
            memcpy(city, html + m[1].rm_so, m[1].rm_eo - m[1].rm_so);
            city[m[1].rm_eo - m[1].rm_so] = 0;
            dictset(results, makestring("city"), makestring(city));
            free(city);
        }
        regcomp(&re, "\"country\":\"([^\"]+)\"", REG_EXTENDED);
        if (regexec(&re, html, 2, m, 0) == 0) {
            char *country = malloc(m[1].rm_eo - m[1].rm_so + 1);
            memcpy(country, html + m[1].rm_so, m[1].rm_eo - m[1].rm_so);
            country[m[1].rm_eo - m[1].rm_so] = 0;
            dictset(results, makestring("country"), makestring(country));
            free(country);
        }
        regfree(&re);
    }
    dictset(results, makestring("ip"), makestring(ip));
    return results;
}

object *osintimage(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.image needs ctx and imagepath");
    object *ctx = args[0];
    char *path = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    object *results = makelist();
    char url[512];
    snprintf(url, sizeof(url), "https://www.google.com/searchbyimage?image_url=file://%s", path);
    object *data = httpget(url, NULL);
    if (data) {
        char *html = data->list.items[0]->sval;
        object *matches = extractprofiles(html, "<a[^>]*href=\"([^\"]+)\"[^>]*>");
        for (int i = 0; i < matches->list.count; i++) {
            listappend(results, matches->list.items[i]);
        }
    }
    return results;
}

object *osintsocial(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.social needs ctx and url");
    object *ctx = args[0];
    char *url = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    object *results = makedict();
    object *data = httpget(url, NULL);
    if (data) {
        char *html = data->list.items[0]->sval;
        regex_t re;
        regcomp(&re, "<title>([^<]*)</title>", REG_EXTENDED);
        regmatch_t m[2];
        if (regexec(&re, html, 2, m, 0) == 0) {
            char *title = malloc(m[1].rm_eo - m[1].rm_so + 1);
            memcpy(title, html + m[1].rm_so, m[1].rm_eo - m[1].rm_so);
            title[m[1].rm_eo - m[1].rm_so] = 0;
            dictset(results, makestring("title"), makestring(title));
            free(title);
        }
        regcomp(&re, "<meta[^>]*property=\"og:description\"[^>]*content=\"([^\"]*)\"", REG_EXTENDED);
        if (regexec(&re, html, 2, m, 0) == 0) {
            char *desc = malloc(m[1].rm_eo - m[1].rm_so + 1);
            memcpy(desc, html + m[1].rm_so, m[1].rm_eo - m[1].rm_so);
            desc[m[1].rm_eo - m[1].rm_so] = 0;
            dictset(results, makestring("description"), makestring(desc));
            free(desc);
        }
        regfree(&re);
        // extract followers count from social media meta tags
        regex_t re2;
        regcomp(&re2, "\"followers\":([0-9]+)", REG_EXTENDED);
        if (regexec(&re2, html, 2, m, 0) == 0) {
            char *followers = malloc(m[1].rm_eo - m[1].rm_so + 1);
            memcpy(followers, html + m[1].rm_so, m[1].rm_eo - m[1].rm_so);
            followers[m[1].rm_eo - m[1].rm_so] = 0;
            dictset(results, makestring("followers"), makeint(atoi(followers)));
            free(followers);
        }
        regfree(&re2);
    }
    dictset(results, makestring("url"), makestring(url));
    return results;
}

object *osinteximage(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.exif needs ctx and imagepath");
    object *ctx = args[0];
    char *path = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    object *exifdata = makedict();
    struct exif_data *ed = exif_data_new_from_file(path);
    if (ed) {
        exif_data_foreach_content(ed, NULL, NULL);
        exif_data_unref(ed);
    }
    return exifdata;
}

object *osintsearch(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.search needs ctx and query");
    object *ctx = args[0];
    char *query = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    char url[512];
    snprintf(url, sizeof(url), "https://www.google.com/search?q=%s", query);
    object *data = httpget(url, NULL);
    object *results = makelist();
    if (data) {
        char *html = data->list.items[0]->sval;
        object *links = extractprofiles(html, "<a[^>]*href=\"([^\"]*)\"[^>]*>");
        for (int i = 0; i < links->list.count; i++) {
            char *link = links->list.items[i]->sval;
            if (strstr(link, "http") == link) {
                listappend(results, makestring(link));
            }
        }
    }
    return results;
}

object *osintbreach(object **args, int argc, void *env) {
    if (argc < 2) throwexception("osint.breach needs ctx and email");
    object *ctx = args[0];
    char *email = args[1]->sval;
    if (ctx->type != 116) throwexception("not osint ctx");
    osintctx *o = (osintctx*)ctx->native.data;
    char url[256];
    snprintf(url, sizeof(url), "https://haveibeenpwned.com/api/v3/breachedaccount/%s", email);
    object *data = httpget(url, NULL);
    object *breaches = makelist();
    if (data) {
        char *html = data->list.items[0]->sval;
        object *names = extractprofiles(html, "\"Name\":\"([^\"]+)\"");
        for (int i = 0; i < names->list.count; i++) {
            listappend(breaches, names->list.items[i]);
        }
    }
    return breaches;
}

void registerosintlib(environment *env) {
    object *mod = makemodule("osint", NULL);
    envset(mod->module.exports, "create", makebuiltin(osintcreate), 0);
    envset(mod->module.exports, "setapikey", makebuiltin(osintsetapikey), 0);
    envset(mod->module.exports, "username", makebuiltin(osintusername), 0);
    envset(mod->module.exports, "email", makebuiltin(osintemail), 0);
    envset(mod->module.exports, "phone", makebuiltin(osintphone), 0);
    envset(mod->module.exports, "domain", makebuiltin(osintdomain), 0);
    envset(mod->module.exports, "ip", makebuiltin(osintip), 0);
    envset(mod->module.exports, "image", makebuiltin(osintimage), 0);
    envset(mod->module.exports, "social", makebuiltin(osintsocial), 0);
    envset(mod->module.exports, "exif", makebuiltin(osinteximage), 0);
    envset(mod->module.exports, "search", makebuiltin(osintsearch), 0);
    envset(mod->module.exports, "breach", makebuiltin(osintbreach), 0);
    envset(env, "osint", mod, 0);
}
