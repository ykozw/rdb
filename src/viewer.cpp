//
#include <chrono>
#include <thread>
//
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <concurrent_queue.h>
#pragma comment(lib, "Ws2_32.lib")

//#include <FL/Fl.H>
#define BUFFER_SIZE (64 * 1024)

//
void init()
{
    puts("0");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData)) 
    {
        exit(1);
    }
    puts("1");
    SOCKET sockd = ::socket(AF_INET, SOCK_STREAM, 0);
    //avoid address in use error that occur if we quit with a client connected
    int t = 1;
    int status = ::setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, (const char*)& t, sizeof(int));
    if (status == -1) 
    {
        exit(1);
    }
    puts("2");
    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = ::htons(10000);

    if (sockd == -1)
    {
        exit(1);
    }

    status = bind(sockd, (struct sockaddr*)&name, sizeof(name));
    puts("3");
    if (status == -1)
    {
        exit(1);
    }
    puts("4");
    status = ::listen(sockd, 5);
    if (status == -1)
    {
        exit(1);
    }
    puts("wait for connection");
    struct sockaddr_in peer_name;
    int32_t addrlen = sizeof(peer_name);
    SOCKET sock2 = ::accept(sockd, (struct sockaddr*)& peer_name, &addrlen);
    printf("6(%d)\n", sock2);
    //
    int32_t start = 0;
    int32_t end = 0;
    char data[BUFFER_SIZE];
    while (true)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);

        //
        if (start != 0)
        {
            size_t size = end - start;
            memmove(data, &data[start], size);
            start = 0;
            end = size;
            puts("7");
        }
        puts("9");
        int r = recv(sock2, &data[end], BUFFER_SIZE - end, 0);
        puts("10");
        if (r > 0) 
        {
            end += r;
            puts("8");

            // デバッグ出力
            data[16] = '\0';
            printf("%s", data);
            start = 0;
            end = 0;
        }
    }
}

//
int main()
{
    init();
    // TODO: ひたすら入力を受け付ける
    while (true)
    {
        // TODO: 
    }
}