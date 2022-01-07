#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "wrap.h"

#define PORT 8001
int main()
{
    // 创建绑定套接字
    int lfd = tcp4bind(PORT, NULL);
    // 监听
    Listen(lfd, 128);
    // 创建树
    int epfd = epoll_create(1);
    // 将lfd上树
    struct epoll_event ev, evs[1024];
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    // while监听
    while(1)
    {
        int nready = epoll_wait(epfd, evs, 1024, -1);
        if(nready < 0){
            perror("");
            break;
        }
        else if(nready == 0){
            continue;
        }
        else{ // 有描述符变化
            for(int i = 0; i < nready; i++){
                // 判断lfd变化，并且是读事件变化
                if(evs[i].data.fd == lfd && evs[i].events & EPOLLIN){
                    // 获取新连接
                    struct sockaddr_in cliaddr;
                    char ip[16] = "";
                    socklen_t len = sizeof(cliaddr);
                    int cfd = Accept(lfd, (struct sockaddr *)&cliaddr, &len);
                    printf("new client ip=%s, port=%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, len), ntohs(cliaddr.sin_port));
                    // 设置cfd为非阻塞
                    int flags = fcntl(cfd, F_GETFL); // 获得cfd的标志位
                    flags |= O_NONBLOCK;
                    fcntl(cfd, F_SETFL, flags);
                    // 将cfd上树
                    ev.data.fd = cfd;
                    // 设置边沿触发
                    ev.events = EPOLLIN | EPOLLET;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                }
                // cfd变化，而且是读事件变化
                else if(evs[i].events & EPOLLIN){
                    // 设置循环读，既可以符合边沿触发，也不用设置较大缓冲区
                    while(1)
                    {
                        char buf[4]="";
                        // 如果读一个空缓冲区，如果是带阻塞，就阻塞等待
                        // 是非阻塞，返回值等于-1，且errno值设置为EAGAIN
                        int n = read(evs[i].data.fd, buf, sizeof(buf));
                        if(n < 0){ // 出错 或者 缓存区无数据
                            // 缓冲区读干净了，应该挑出循环，继续监听
                            if(errno = EAGAIN){
                                break;
                            }
                            // 普通错误 下树
                            perror("");
                            close(evs[i].data.fd);
                            epoll_ctl(epfd, EPOLL_CTL_DEL, evs[i].data.fd, &evs[i]);
                            break;
                        }
                        else if(n == 0){ // 客户端关闭
                            printf("client close\n");
                            close(evs[i].data.fd); // 将cfd关闭
                            epoll_ctl(epfd, EPOLL_CTL_DEL, evs[i].data.fd, &evs[i]);
                            break;
                        }
                        else{
                            //printf("%s", buf);
                            write(STDOUT_FILENO, buf, 4);
                            write(evs[i].data.fd, buf, n);
                        }
                    }
                }
            }
        }
    }
    return 0;
}