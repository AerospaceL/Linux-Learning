#include "wrap.h"
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <pthread.h>

typedef struct c_info
{
    int cfd;
    struct sockaddr_in cliaddr;
}CINFO;

void* client_fun(void *arg);

int main(int agrc, char *argv[])
{
    if(agrc < 2){
        printf("argc < 2???   \n ./a.out 8000 \n");
        return 0;
    }
    // 设置线程结束，系统自动回收资源
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    short port = atoi(argv[1]);
    int lfd = tcp4bind(port, NULL);
    Listen(lfd, 128);

    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    CINFO *info;
    while(1)
    {
        int cfd = Accept(lfd, (struct sockaddr *)&cliaddr, &len);
        char ip[16] = "";
        //printf("new client ip=%s, port=%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, 16),ntohs(cliaddr.sin_port));

        pthread_t pthid;
        info = malloc(sizeof(CINFO)); // 使用堆区，防止覆盖
        info->cfd = cfd;
        info->cliaddr = cliaddr;
        pthread_create(&pthid, &attr, client_fun, info);
    }

}

void* client_fun(void *arg)
{
    CINFO *info = (CINFO *)arg;
    char ip[16] = "";
    printf("new client ip=%s, port=%d\n", inet_ntop(AF_INET, &(info->cliaddr.sin_addr.s_addr), ip, 16), ntohs(info->cliaddr.sin_port));

    while(1)
    {
        char buf[1024] = "";
        int count = 0;
        count = read(info->cfd, buf, sizeof(buf));
        if(count < 0){
            perror("");
            break;
        }
        else if(count == 0){
            printf("client close port=%d\n", ntohs(info->cliaddr.sin_port));
            break;
        }
        else{
            printf("%s\n", buf);
            write(info->cfd, buf, count);
        }
    }
    close(info->cfd);
    free(info);
}