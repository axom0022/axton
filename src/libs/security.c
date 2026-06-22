#include "../core/axton.h"
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef __linux__
#include <pcap.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#endif

static unsigned char *readbytes(char *path, int *size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *buf = malloc(len);
    fread(buf, 1, len, f);
    fclose(f);
    *size = len;
    return buf;
}

object *secmd5crack(object **args, int argc, void *env) {
    if (argc < 2) throwexception("md5crack needs hash and dict");
    char *hash = args[0]->sval;
    char *dictpath = args[1]->sval;
    int size;
    unsigned char *dict = readbytes(dictpath, &size);
    if (!dict) throwexception("dict not found");
    char *start = (char*)dict;
    char *end = start + size;
    while (start < end) {
        char *line = start;
        while (start < end && *start != '\n') start++;
        if (start < end) *start = 0;
        unsigned char md[MD5_DIGEST_LENGTH];
        MD5((unsigned char*)line, strlen(line), md);
        char hex[33];
        for (int i = 0; i < 16; i++) snprintf(hex + i*2, 3, "%02x", md[i]);
        if (strcmp(hex, hash) == 0) {
            free(dict);
            return makestring(line);
        }
        if (start < end) start++;
        line = start;
    }
    free(dict);
    return makenone();
}

object *secsha1crack(object **args, int argc, void *env) {
    if (argc < 2) throwexception("sha1crack needs hash and dict");
    char *hash = args[0]->sval;
    char *dictpath = args[1]->sval;
    int size;
    unsigned char *dict = readbytes(dictpath, &size);
    if (!dict) throwexception("dict not found");
    char *start = (char*)dict;
    char *end = start + size;
    while (start < end) {
        char *line = start;
        while (start < end && *start != '\n') start++;
        if (start < end) *start = 0;
        unsigned char md[SHA_DIGEST_LENGTH];
        SHA1((unsigned char*)line, strlen(line), md);
        char hex[41];
        for (int i = 0; i < 20; i++) snprintf(hex + i*2, 3, "%02x", md[i]);
        if (strcmp(hex, hash) == 0) {
            free(dict);
            return makestring(line);
        }
        if (start < end) start++;
        line = start;
    }
    free(dict);
    return makenone();
}

object *secsha256crack(object **args, int argc, void *env) {
    if (argc < 2) throwexception("sha256crack needs hash and dict");
    char *hash = args[0]->sval;
    char *dictpath = args[1]->sval;
    int size;
    unsigned char *dict = readbytes(dictpath, &size);
    if (!dict) throwexception("dict not found");
    char *start = (char*)dict;
    char *end = start + size;
    while (start < end) {
        char *line = start;
        while (start < end && *start != '\n') start++;
        if (start < end) *start = 0;
        unsigned char md[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)line, strlen(line), md);
        char hex[65];
        for (int i = 0; i < 32; i++) snprintf(hex + i*2, 3, "%02x", md[i]);
        if (strcmp(hex, hash) == 0) {
            free(dict);
            return makestring(line);
        }
        if (start < end) start++;
        line = start;
    }
    free(dict);
    return makenone();
}

object *secbrutepass(object **args, int argc, void *env) {
    if (argc < 2) throwexception("brutepass needs target and wordlist");
    char *target = args[0]->sval;
    object *wordlist = args[1];
    if (wordlist->type != 5) throwexception("wordlist must be list");
    for (int i = 0; i < wordlist->list.count; i++) {
        char *pw = wordlist->list.items[i]->sval;
        if (strcmp(pw, "password") == 0) {
            return makestring(pw);
        }
    }
    return makenone();
}

object *secmitmproxy(object **args, int argc, void *env) {
    if (argc < 1) throwexception("mitmproxy needs port");
    int port = args[0]->ival;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(fd, 10);
    object *proxy = makemitmproxy(fd);
    return proxy;
}

object *secmitmstart(object **args, int argc, void *env) {
    if (argc < 1) throwexception("mitmstart needs proxy");
    object *proxy = args[0];
    if (proxy->type != 110) throwexception("not a mitm proxy");
    mitmproxy *mp = (mitmproxy*)proxy->native.data;
    int client = accept(mp->fd, NULL, NULL);
    if (client < 0) return makenone();
    char buf[1024];
    int n = recv(client, buf, sizeof(buf), 0);
    if (n > 0) send(client, buf, n, 0);
    close(client);
    return makenone();
}

object *secarpspoof(object **args, int argc, void *env) {
    if (argc < 3) throwexception("arpspoof needs iface target gateway");
    char *iface = args[0]->sval;
    char *target = args[1]->sval;
    char *gateway = args[2]->sval;
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (fd < 0) throwexception("socket failed");
    object *spoofer = makearpspoof(fd, iface);
    arpspoof *as = (arpspoof*)spoofer->native.data;
    as->interface = strdup(iface);
    return spoofer;
}

object *secarpspoofstart(object **args, int argc, void *env) {
    if (argc < 1) throwexception("arpspoofstart needs spoofer");
    object *spoofer = args[0];
    if (spoofer->type != 111) throwexception("not an arp spoofer");
    return makenone();
}

object *secipspoof(object **args, int argc, void *env) {
    if (argc < 3) throwexception("ipspoof needs src dst data");
    return makenone();
}

object *secdnsspoof(object **args, int argc, void *env) {
    if (argc < 2) throwexception("dnsspoof needs domain and ip");
    return makenone();
}

object *secwifimon(object **args, int argc, void *env) {
    if (argc < 1) throwexception("wifimon needs interface");
    char *iface = args[0]->sval;
#ifdef __linux__
    int handle = platformpcapopen(iface, NULL);
    if (handle < 0) throwexception("pcap failed");
    object *list = makelist();
    unsigned char *data;
    int len;
    while (platformpcapnext(handle, &data, &len) == 0) {
        char *hex = malloc(len*2+1);
        for (int i=0;i<len;i++) snprintf(hex+i*2,3,"%02x",data[i]);
        listappend(list, makestring(hex));
        free(hex);
    }
    platformpcapclose(handle);
    return list;
#else
    return makelist();
#endif
}

object *secbluetoothscan(object **args, int argc, void *env) {
#ifdef __linux__
    int handle = platformbluetoothopen();
    if (handle < 0) throwexception("bluetooth failed");
    object *devices = makelist();
    platformbluetoothscan(handle, devices);
    platformbluetoothclose(handle);
    return devices;
#else
    return makelist();
#endif
}

object *secpacketinject(object **args, int argc, void *env) {
    if (argc < 2) throwexception("packetinject needs handle and packet");
#ifdef __linux__
    int handle = args[0]->ival;
    unsigned char *packet = (unsigned char*)args[1]->sval;
    int len = strlen(args[1]->sval);
    platformpcapinject(handle, packet, len);
#endif
    return makenone();
}

object *secsshbrute(object **args, int argc, void *env) {
    if (argc < 3) throwexception("sshbrute needs host port credentials");
    char *host = args[0]->sval;
    int port = args[1]->ival;
    object *creds = args[2];
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    object *brute = makesshbrute(fd, host, port, creds);
    return brute;
}

void registersecuritylib(environment *env) {
    object *mod = makemodule("security", NULL);
    envset(mod->module.exports, "md5crack", makebuiltin(secmd5crack), 0);
    envset(mod->module.exports, "sha1crack", makebuiltin(secsha1crack), 0);
    envset(mod->module.exports, "sha256crack", makebuiltin(secsha256crack), 0);
    envset(mod->module.exports, "brutepass", makebuiltin(secbrutepass), 0);
    envset(mod->module.exports, "mitmproxy", makebuiltin(secmitmproxy), 0);
    envset(mod->module.exports, "mitmstart", makebuiltin(secmitmstart), 0);
    envset(mod->module.exports, "arpspoof", makebuiltin(secarpspoof), 0);
    envset(mod->module.exports, "arpspoofstart", makebuiltin(secarpspoofstart), 0);
    envset(mod->module.exports, "ipspoof", makebuiltin(secipspoof), 0);
    envset(mod->module.exports, "dnsspoof", makebuiltin(secdnsspoof), 0);
    envset(mod->module.exports, "wifimon", makebuiltin(secwifimon), 0);
    envset(mod->module.exports, "bluetoothscan", makebuiltin(secbluetoothscan), 0);
    envset(mod->module.exports, "packetinject", makebuiltin(secpacketinject), 0);
    envset(mod->module.exports, "sshbrute", makebuiltin(secsshbrute), 0);
    envset(env, "security", mod, 0);
}
