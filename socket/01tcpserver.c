#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    // 创建套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    // 绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8000);
    inet_pton(AF_INET, "10.0.24.11", &addr.sin_addr.s_addr);
    // addr.sin_addr.s_addr = INADDR_ANY; // 绑定的是通配地址，即本机所有ip地址
    int ret = bind(lfd, (struct socket_addr *)&addr, sizeof(addr));
    if(ret < 0){
        perror("");
        exit(0);
    }
    // 监听
    listen(lfd, 128); 
    // 提取
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    int cfd = accept(lfd, (struct socket_addr *)&cliaddr, &len);
    char ip[16]="";
    printf("new client ip=%s, port=%d\n", inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, ip, 16), ntohs(cliaddr.sin_port));
    // 读写
    char buf[1024] = "";
    while(1)
    {
        bzero(buf, sizeof(buf)); // 清零
        int n = read(STDIN_FILENO, buf, sizeof(buf));
        write(cfd, buf, n);
        n = read(cfd, buf ,sizeof(buf));
        printf("%s\n", buf);
    }
    // 关闭
    close(lfd);
    close(cfd);
    return 0;
}