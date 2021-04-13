#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int fd;
int new_fd;
int flag = 0;
char write_buffer[100];
char read_buffer[100];

// inc SocketBindInit()
// =====================================
// Socket 파일 디스크립터 생성, socket()
// IP 주소와 Port 바인딩, bind()
int SocketBindInit()
{
    int ret = 0;

    // int socket(int domain, int type, int protocol)
    // ================================================================================================
    // int domain : 어떤 영역에서 통신할 것인지 결정. AF_UNIX(프로토콜 내부), AF_INET(IPv4)
    // int type : 어떤 타입의 프로토콜을 사용할 것인지 결정. SOCK_STREAM(TCP), SOCK_DGRAM(UDP)
    // int protocol : 어떤 프로토콜의 값을 결정하는 것. 0, IPPROTO_TCP(TCP일 때), IPPROTO_UDP(UDP일 때)
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        printf("socket error\n");
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;  // IPv4

    // u_short htons(u_short hostshort)
    // ====================================================================================
    // 호스트 바이트 정렬 방식의 2바이트 데이터(short)를 네트워크 바이트 정렬 방식으로 변환
    // Big-Endian or Little-Endian -> Big-Endian
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = 0;

    // int bind(int sockfd, struct sockaddr* myaddr, int addrlen)
    // =======================================================================================================
    // 주소 정보 구조체 변수 생성 및 초기화에 이은 소켓에 주소 정보를 할당하는 함수.
    // int sockfd : 주소를 할당하고자 하는 소켓의 파일 디스크립터
    // struct sockaddr* myaddr : 할당하고자 하는 주소 정보를 지니고 있는 sockaddr 구조체 변수의 포인터 인자 값
    // int addrlen : 인자로 전달된 주소 정보 구조체의 길이
    ret = bind(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr));
    if (ret == -1) {
        printf("bind error\n");
        printf("Error code: %d\n", errno);
        return ret;
    }
}

// inc ListenInit()
// =============================
// Client의 연결을 기다리는 함수
int ListenInit()
{
    int ret = 0;

    // int listen(SOCKET s, int backlog)
    // ======================================================================
    // 특정 Socket Client의 접속을 받을 수 있도록 만들어주는 함수
    // s: bind된 Socket
    // backlog: 연결 큐에 최대 몇개의 연결 처리할 데이터를 담을지를 담은 변수
    ret = listen(fd, 2);
    if (ret == -1) {
        printf("listen error\n");
        return ret;
    }
    printf("LISTEN...\n");
    return ret;
}

// void bye()
// ==============================================
// ServerRead()에서 EOF를 읽은 경우 호출되는 함수
void bye()
{
    printf("\nbye\n");

    shutdown(new_fd, SHUT_WR);  // 0(EOF, End Of File)을 전달한다.

    close(new_fd);
    close(fd);
    exit(0);
}

// void* ServerRead()
// ==============================================
// Client로 부터 읽어서 Read에 저장한다.
// read_length에는 길이가 저장된다.
// EOF를 읽은 경우 flag를 1로 변경한 후 함수 종료
void* ServerRead()
{
    while (1) {
        memset(read_buffer, 0, 100);  // 초기화 해주면 시작하자마자 꺼지는 것 막을 수 있음
        int read_length = read(new_fd, read_buffer, sizeof(read_buffer));
        if (read_length == 0) {  // EOF
            printf("client exit\n");
            flag = 1;  // ServerWrite()를 종료해주기 위한 flag
            return;
        }
        printf("%s\n", read_buffer);
        sleep(1);
    }
}

// void* ServerWrite()
// ==============================================
// 입력받은 write_buffer를 Client로 전달하는 함수
// ServerRead()가 종료되었다면 ServerWrite() 종료
void* ServerWrite()
{
    while (1) {
        gets(write_buffer);
        if (flag == 1) return;
        write(new_fd, write_buffer, sizeof(write_buffer));
    }
}

int main()
{
    flag = 0;
    signal(SIGINT, bye);  // CTRL + C 입력시 bye() 호출

    int ret = SocketBindInit();
    if (ret == -1) {
        printf("SocketBindInit ERROR\n");
    }

    while (1) {  // Client가 다시 켜지는 경우에 통신을 위한 while(1)
        ret = ListenInit();
        if (ret == -1) {
            printf("ListenInit ERROR\n");
            break;
        }

        struct sockaddr new_addr = {0};
        int len;

        // int accept(int sockfd, struct sockaddr* addr, int *addrlen)
        // =====================================
        // Client로부터 연결 요청을 수락하여 데이터를 주고 받을 수 있는 상태
        // int sockfd : Server 소켓의 파일 디스크립터
        // struct sockaddr* addr : 연결 요청을 수락할 클라이언트의 주소 정보를 저장할 변수의 포인터
        // int addrlen : 인자로 전달된 주소 정보 구조체의 길이
        new_fd = accept(fd, &new_addr, &len);
        printf("START\n");

        pthread_t pt1, pt2;
        pthread_create(&pt1, NULL, ServerRead, NULL);
        pthread_create(&pt2, NULL, ServerWrite, NULL);

        pthread_join(pt1, NULL);
        pthread_join(pt2, NULL);

        printf("byebye\n");
        close(new_fd);
    }

    close(fd);

    return 0;
}