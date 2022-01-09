#include <stdio.h>
#include "wrap.h"
#include <event.h>

#define _MAX_CLIENT_ 1024

typedef struct FdEventMap{
    int fd;
    struct event *ev;  // 对应事件
}FdEvent;

FdEvent mFdEvents[_MAX_CLIENT_];

void initEventArray()
{
    int i;
    for(i = 0; i < _MAX_CLIENT_; i++){
        mFdEvents[i].fd = -1;
        mFdEvents[i].ev = NULL;
    }
}

int addEvent(int fd, struct event *ev)
{
    int i;
    for(i = 0; i < _MAX_CLIENT_; i++){
        if(mFdEvents[i].fd < 0){
            break;
        }
    }
    if(i == _MAX_CLIENT_){
        printf("too many clients...\n");
        return -1;
    }
    mFdEvents[i].fd = fd;
    mFdEvents[i].ev = ev;
    return 0;
}

struct event* getEventByFd(int fd)
{
    int i;
    for(i = 0; i < _MAX_CLIENT_; i++){
        if(mFdEvents[i].fd == fd){
            // 匹配
            return mFdEvents[i].ev;
        }
    }
    return NULL;
}

void cfdcb(int cfd, short event, void *arg)
{
    char buf[1500] = "";
    int n = Read(cfd, buf, sizeof(buf));
    if(n <= 0){
        close(cfd);
        event_del(getEventByFd(cfd)); // 下树
    }
    else{
        printf("%s\n", buf);
        Write(cfd, buf, n);
    }
}

void lfdcb(int lfd, short event, void*arg)
{
    struct event_base *base = (struct event_base *)arg;
    // 提取新的cfd
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    int cfd = Accept(lfd, (struct sockaddr *)&cliaddr, &len);
    // 将cfd上树
    struct event *ev = event_new(base, cfd, EV_READ | EV_PERSIST, cfdcb, base);
    event_add(ev, NULL);
    // 加入数组
    addEvent(cfd, ev);
}

int main()
{
    int lfd = tcp4bind(8082, NULL);
    Listen(lfd, 128);
    initEventArray();
    // 创建根节点
    struct event_base * base = event_base_new();
    struct event *ev = event_new(base, lfd, EV_READ | EV_PERSIST, lfdcb, base);
    // 上树
    event_add(ev, NULL);
    // 循环监听
    event_base_dispatch(base); // 阻塞
    // 收尾
    event_free(ev);
    event_base_free(base);
    return 0;
}