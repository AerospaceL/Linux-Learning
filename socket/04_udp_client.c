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
    myaddr.sin_port = htons(9000);
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    int ret = bind(fd, (struct sockaddr*)&myaddr, sizeof(myaddr));
    if(ret < 0){
        perror("");
        return 0;
    }

    char buf[1500];
    struct sockaddr_in dstaddr;
    dstaddr.sin_family = AF_INET;
    dstaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    dstaddr.sin_port = htons(8080);
    socklen_t len = sizeof(dstaddr);

    int n = 0;
    while(1)
    {
        n = read(STDIN_FILENO, buf, sizeof(buf));
        sendto(fd, buf, n, 0, (struct sockaddr*)&dstaddr, len);
        memset(buf, 0, sizeof(buf));
        recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
        if(n < 0){
            perror("");
            break;
        }
        else{
            printf("%s\n", buf);
            //sendto(fd, buf, n, 0, (struct sockaddr*)&dstaddr, len);
        }
    }
    close(fd);
    return 0;
}