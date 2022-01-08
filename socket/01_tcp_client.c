#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    // 创建套接字
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    // 连接服务器
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "10.0.24.11", &addr.sin_addr.s_addr);
    connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
    // 读写数据
    char buf[1024] = "";
    while(1)
    {
        int n = read(STDIN_FILENO, buf, sizeof(buf));
        write(sock_fd, buf, n);

        n = read(sock_fd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, n);
    }
    close(sock_fd);
    return 0;
}