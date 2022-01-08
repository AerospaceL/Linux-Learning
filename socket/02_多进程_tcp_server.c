// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>
#include "wrap.h"
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>

void free_process(int sig)
{
    pid_t pid;
    while(1)
    {
        pid = waitpid(-1, NULL, WNOHANG);
        if(pid <= 0) // 小于0 子进程全部退出了; 等于0 没有进程退出 
        {
            break;
        }
        else{
            printf("child pid=%d\n", pid);
        }
    }

}

int main()
{
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    sigprocmask(SIG_BLOCK, &set, NULL);
    // 创建套接字
    int lfd = tcp4bind(8000, NULL);
    // 绑定
    Listen(lfd, 128);
    // 监听
    // 回射
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    while(1)
    {
        char ip[16] = "";
        int cfd = Accept(lfd, (struct sockaddr *)&cliaddr, &len);
        printf("new client ip=%s, port=%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, 16), ntohs(cliaddr.sin_port));
        // fork创建子进程
        pid_t pid = fork();
        if(pid < 0){
            perror("");
            exit(0);
        }
        else if(pid == 0){ // 子进程
            close(lfd);
            while(1){
                char buf[1024] = "";
                int n = read(cfd, buf, sizeof(buf));
                if(n < 0){
                    perror("");
                    close(cfd);
                    break;
                }
                else if(n == 0){ // 对方关闭
                    printf("clienr close\n");
                    close(cfd);
                    break;
                }
                else{
                    printf("%s\n", buf);
                    write(cfd, buf, n);
                }
            }
        }
        else{ // 父进程
            close(cfd);
            // 回收子进程资源
            // 注册信号回调
            struct sigaction act;
            act.sa_flags = 0;
            act.sa_handler = free_process;
            sigemptyset(&act.sa_mask);
            sigaction(SIGCHLD,&act, NULL);
            sigprocmask(SIG_UNBLOCK, &set, NULL);
        }
    }
    // 关闭

}