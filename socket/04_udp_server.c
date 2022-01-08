#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    // 创建
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    // 绑定
    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(8080);
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    int ret = bind(fd, (struct sockaddr*)&myaddr, sizeof(myaddr));
    if(ret < 0){
        perror("");
        return 0;
    }
    char buf[1500];
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    while(1)
    {
        memset(buf, 0, sizeof(buf));
        int n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&cliaddr, &len);
        if(n < 0){
            perror("");
            break;
        }
        else{
            printf("%s\n", buf);
            sendto(fd, buf, n, 0, (struct sockaddr*)&cliaddr, len);
        }
    }
    close(fd);
    return 0;
}