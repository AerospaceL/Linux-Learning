#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "wrap.h"

#define PORT 8080

int main(int argc, char *argv)
{
    // 创建绑定套接字
    int lfd = tcp4bind(PORT, NULL);
    // 监听
    Listen(lfd, 128);

    int maxfd = lfd; // 最大的文件描述符
    fd_set oldset, rset;
    FD_ZERO(&oldset);
    FD_ZERO(&rset);
    // 将lfd添加到oldset中
    FD_SET(lfd, &oldset);
    while (1)
    {
        rset = oldset;
        int n = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if(n < 0){
            perror("");
            break;
        } 
        else if(n == 0){ // 无变化
            continue;
        }
        else{  // 监听到文件描述符变化
            // lfd变化，代表有新的连接到来
            if(FD_ISSET(lfd, &rset)){
                struct sockaddr_in cliaddr;
                socklen_t len = sizeof(cliaddr);
                char ip[16] = "";
                // 提取新的连接
                int cfd = Accept(lfd, (struct sockaddr*)&cliaddr, &len);
                printf("new client ip=%s, port=%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, 16), ntohs(cliaddr.sin_port));
                // 将cfd添加至oldset集合中，以下次监听
                FD_SET(cfd, &oldset);
                //maxfd = maxfd > cfd ? maxfd : cfd;
                if(maxfd < cfd)
                    maxfd = cfd;
                // 如果只有lfd变化，continue
                if(--n == 0)
                    continue;
            }
            // cfd 遍历lfd之后的文件描述符是否在rset集合中，如果在则cfd变化
            for(int i = lfd + 1; i < maxfd; i++){
                if(FD_ISSET(i, &rset)){
                    char buf[1500] = "";
                    int ret = Read(i, buf, sizeof(buf));
                    if(ret < 0){ // 出错，将cfd关闭，从oldset中删除cfd
                        perror("");
                        close(i);
                        FD_CLR(i, &oldset);
                        continue;
                    }
                    else if(ret == 0){
                        printf("client close\n");
                        close(i);
                        FD_CLR(i, &oldset);
                    }
                    else{
                        printf("%s\n", buf);
                        Write(i, buf, ret); // 回射
                    }
                }
            }
        }
    }
     
}