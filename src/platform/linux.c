#include "platform.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#include <sys/ptrace.h>
#include <sys/user.h>
#include <pcap.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#endif

void platforminit(void) {}

void platformlog(const char *msg) {
    printf("%s", msg);
    fflush(stdout);
}

void platformsleep(double seconds) {
    usleep(seconds * 1000000);
}

double platformtime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

char *platformreadfile(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = 0;
    fclose(f);
    return buf;
}

int platformwritefile(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    fprintf(f, "%s", content);
    fclose(f);
    return 1;
}

void *platformloadlib(const char *path) {
    return dlopen(path, RTLD_LAZY);
}

int platformsocket(void) {
    return socket(AF_INET, SOCK_STREAM, 0);
}

int platformbind(int fd, int port) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    return bind(fd, (struct sockaddr*)&addr, sizeof(addr));
}

int platformlisten(int fd, int backlog) {
    return listen(fd, backlog);
}

int platformaccept(int fd) {
    return accept(fd, NULL, NULL);
}

int platformsend(int fd, const char *data, int len) {
    return send(fd, data, len, 0);
}

int platformrecv(int fd, char *buf, int len) {
    return recv(fd, buf, len, 0);
}

char *platformgetenv(const char *name) {
    return getenv(name);
}

int platformsetenv(const char *name, const char *value) {
    return setenv(name, value, 1);
}

int platformkill(int pid, int sig) {
    return kill(pid, sig);
}

int platformgetpid(void) {
    return getpid();
}

void *platformopendir(const char *path) {
    return (void*)opendir(path);
}

char *platformreaddir(void *dir) {
    struct dirent *entry = readdir((DIR*)dir);
    if (!entry) return NULL;
    return strdup(entry->d_name);
}

void platformclosedir(void *dir) {
    closedir((DIR*)dir);
}

int platformchmod(const char *path, int mode) {
    return chmod(path, mode);
}

int platformchown(const char *path, int uid, int gid) {
    return chown(path, uid, gid);
}

void *platformallocate(int size) {
    return malloc(size);
}

void platformdeallocate(void *ptr) {
    free(ptr);
}

void *platformreallocate(void *ptr, int size) {
    return realloc(ptr, size);
}

int platformgetpagesize(void) {
    return getpagesize();
}

#ifdef __linux__
int platformpcapopen(const char *iface, char *err) {
    char ebuf[PCAP_ERRBUF_SIZE];
    pcap_t *p = pcap_open_live(iface, 65536, 1, 1000, ebuf);
    if (!p) {
        if (err) strcpy(err, ebuf);
        return -1;
    }
    return (int)(long)p;
}

int platformpcapnext(int handle, unsigned char **data, int *len) {
    pcap_t *p = (pcap_t*)(long)handle;
    struct pcap_pkthdr *hdr;
    const unsigned char *packet;
    int res = pcap_next_ex(p, &hdr, &packet);
    if (res == 1) {
        *data = malloc(hdr->len);
        memcpy(*data, packet, hdr->len);
        *len = hdr->len;
        return 0;
    }
    return -1;
}

int platformpcapinject(int handle, unsigned char *packet, int len) {
    pcap_t *p = (pcap_t*)(long)handle;
    return pcap_inject(p, packet, len);
}

void platformpcapclose(int handle) {
    pcap_t *p = (pcap_t*)(long)handle;
    pcap_close(p);
}

int platformbluetoothopen(void) {
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0) return -1;
    int sock = hci_open_dev(dev_id);
    return sock;
}

int platformbluetoothscan(int handle, object *devices) {
    if (handle < 0) return -1;
    inquiry_info *info = NULL;
    int max_rsp = 255;
    int len = 8;
    info = malloc(max_rsp * sizeof(inquiry_info));
    int num = hci_inquiry(handle, len, max_rsp, NULL, &info, IREQ_CACHE_FLUSH);
    if (num < 0) { free(info); return -1; }
    for (int i = 0; i < num; i++) {
        char addr[18];
        ba2str(&info[i].bdaddr, addr);
        char name[248];
        if (hci_read_remote_name(handle, &info[i].bdaddr, sizeof(name), name, 0) == 0) {
            object *entry = makedict();
            dictset(entry, makestring("address"), makestring(addr));
            dictset(entry, makestring("name"), makestring(name));
            listappend(devices, entry);
        }
    }
    free(info);
    return 0;
}

void platformbluetoothclose(int handle) {
    if (handle >= 0) close(handle);
}

int platformprocessopen(int pid, int flags) {
    (void)flags;
#ifdef __linux__
    // attach with ptrace
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) return -1;
    waitpid(pid, NULL, 0);
    return pid;
#else
    return -1;
#endif
}

int platformprocessread(int handle, long address, unsigned char *buf, int size) {
#ifdef __linux__
    long word;
    for (int i = 0; i < size; i += sizeof(long)) {
        errno = 0;
        word = ptrace(PTRACE_PEEKDATA, handle, (void*)(address + i), NULL);
        if (errno != 0) return -1;
        int bytes = size - i;
        if (bytes > sizeof(long)) bytes = sizeof(long);
        memcpy(buf + i, &word, bytes);
    }
    return 0;
#else
    return -1;
#endif
}

int platformprocesswrite(int handle, long address, unsigned char *buf, int size) {
#ifdef __linux__
    for (int i = 0; i < size; i += sizeof(long)) {
        long word = 0;
        int bytes = size - i;
        if (bytes > sizeof(long)) bytes = sizeof(long);
        memcpy(&word, buf + i, bytes);
        errno = 0;
        if (ptrace(PTRACE_POKEDATA, handle, (void*)(address + i), word) == -1) {
            if (errno != 0) return -1;
        }
    }
    return 0;
#else
    return -1;
#endif
}

void platformprocessclose(int handle) {
#ifdef __linux__
    ptrace(PTRACE_DETACH, handle, NULL, NULL);
#endif
}

int platformprocessfind(const char *name) {
    DIR *d = opendir("/proc");
    if (!d) return -1;
    struct dirent *entry;
    while ((entry = readdir(d))) {
        if (entry->d_type != DT_DIR) continue;
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        char path[256];
        snprintf(path, sizeof(path), "/proc/%d/comm", pid);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        char line[64];
        if (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\n")] = 0;
            if (strcmp(line, name) == 0) {
                fclose(f);
                closedir(d);
                return pid;
            }
        }
        fclose(f);
    }
    closedir(d);
    return -1;
}
#else
int platformpcapopen(const char *iface, char *err) { return -1; }
int platformpcapnext(int handle, unsigned char **data, int *len) { return -1; }
int platformpcapinject(int handle, unsigned char *packet, int len) { return -1; }
void platformpcapclose(int handle) {}
int platformbluetoothopen(void) { return -1; }
int platformbluetoothscan(int handle, object *devices) { return -1; }
void platformbluetoothclose(int handle) {}
int platformprocessopen(int pid, int flags) { return -1; }
int platformprocessread(int handle, long address, unsigned char *buf, int size) { return -1; }
int platformprocesswrite(int handle, long address, unsigned char *buf, int size) { return -1; }
void platformprocessclose(int handle) {}
int platformprocessfind(const char *name) { return -1; }
#endif
