#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int fd;

// int Init()
// =====================================
// Socket 파일 디스크립터 생성, socket()
// IP 주소와 Port 바인딩, bind()
int Init()
{
    int ret = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);  // IPv4, 양방향, TCP
    if (fd == -1) return -1;

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;  // IPv4
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = inet_addr("15.164.48.55");  // 서버의 ip 주소

    // int connect(int sockfd, const struct sockaddr* serv_addr, socklen_t addrlen)
    // ============================================================================
    // int sockfd : 소켓 디스크립터
    // struct sockaddr* serv_addr : 서버 주소 정보에 대한 포인터
    // socklen_t addrlen : struct sockaddr *serv_addr 포인터가 가르키는 구조체의 크기
    ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) return ret;

    return ret;
}

// void bye()
// ==============================================
// ServerRead()에서 EOF를 읽은 경우 호출되는 함수
void bye()
{
    printf("\nBYE\n");

    shutdown(fd, SHUT_WR);

    close(fd);
    exit(1);
}

// void* ClientWrite()
// ==============================================
// 입력받은 write_buffer를 Server로 전달하는 함수
void* ClientWrite()
{
    char write_buffer[100];
    while (1) {
        gets(write_buffer);
        write(fd, write_buffer, sizeof(write_buffer));
    }

    return 0;
}

// void* ClientRead()
// ==============================================
// Server로 부터 읽어서 Read에 저장한다.
// read_length에는 길이가 저장된다.
// EOF를 읽은 경우 bye() 함수 실행
void* ClientRead()
{
    char read_buffer[100];
    while (1) {
        memset(read_buffer, 0, 100);
        int read_length = read(fd, read_buffer, sizeof(read_buffer));
        if (read_length == 0) bye();  // EOF을 Server로부터 받았을 때 bye() 함수를 실행
        printf("%s\n", read_buffer);
        sleep(1);
    }

    return 0;
}

int main()
{
    signal(SIGINT, bye);  // CTRL + C 입력시 bye() 호출

    int ret = Init();
    if (ret == -1) printf("INIT ERROR\n");

    pthread_t pt1, pt2;
    pthread_create(&pt1, NULL, ClientRead, NULL);
    pthread_create(&pt2, NULL, ClientWrite, NULL);

    pthread_join(pt1, NULL);
    pthread_join(pt2, NULL);

    return 0;
}
