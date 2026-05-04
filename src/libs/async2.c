#include "async2.h"
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <fcntl.h>

static int epollfd = -1;
static asynctask *taskqueue = NULL;
static asynctask *runningtasks = NULL;
static int eventlooprunning = 0;
static struct epoll_event events[256];
static object *tasklist = NULL;

static void setnonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static void addevent(int fd, int events, asynctask *task) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = task;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void modi
