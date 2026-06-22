#ifndef platform_h
#define platform_h

void platforminit(void);
void platformlog(const char *msg);
void platformsleep(double seconds);
double platformtime(void);
char *platformreadfile(const char *path);
int platformwritefile(const char *path, const char *content);
void *platformloadlib(const char *path);
int platformsocket(void);
int platformbind(int fd, int port);
int platformlisten(int fd, int backlog);
int platformaccept(int fd);
int platformsend(int fd, const char *data, int len);
int platformrecv(int fd, char *buf, int len);
char *platformgetenv(const char *name);
int platformsetenv(const char *name, const char *value);
int platformkill(int pid, int sig);
int platformgetpid(void);
void *platformopendir(const char *path);
char *platformreaddir(void *dir);
void platformclosedir(void *dir);
int platformchmod(const char *path, int mode);
int platformchown(const char *path, int uid, int gid);
void *platformallocate(int size);
void platformdeallocate(void *ptr);
void *platformreallocate(void *ptr, int size);
int platformgetpagesize(void);
int platformpcapopen(const char *iface, char *err);
int platformpcapnext(int handle, unsigned char **data, int *len);
int platformpcapinject(int handle, unsigned char *packet, int len);
void platformpcapclose(int handle);
int platformbluetoothopen(void);
int platformbluetoothscan(int handle, object *devices);
void platformbluetoothclose(int handle);
int platformprocessopen(int pid, int flags);
int platformprocessread(int handle, long address, unsigned char *buf, int size);
int platformprocesswrite(int handle, long address, unsigned char *buf, int size);
void platformprocessclose(int handle);
int platformprocessfind(const char *name);

#endif
