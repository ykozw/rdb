//
#include "rdb.hpp"
//
#include <thread>
//
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <concurrent_queue.h>
#pragma comment(lib, "Ws2_32.lib")
//
enum class RdbTaskType : int32_t
{
    POINT
};
//
struct RdbTask
{
    RdbTaskType type;
    union
    {
        struct
        {
            float r;
            float g;
            float b;
            int32_t group;
        }rdbPoint;
    };
};
Concurrency::concurrent_queue<RdbTask> rdbTasks;

constexpr int32_t RDB_BUFFER_SIZE = 1024 * 4;

struct RdbContext
{
public:
    bool initialized = false;
    size_t bytesToSend = 0;
    SOCKET fd;
    
}rdbContext;

std::thread rdbMainThread;

//
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void rdbInit();


void rdb_printf(const char* fmt, ...)
{
    puts("rdb_printf");
    //
    rdbInit();
    //
    char buffer[RDB_BUFFER_SIZE];
    va_list argp;
    va_start(argp, fmt);
    const int32_t bytesToSend = vsnprintf(buffer, RDB_BUFFER_SIZE, fmt, argp);
    va_end(argp);
    //
    size_t sended = ::send(rdbContext.fd, buffer, bytesToSend, 0);
    //
    puts(buffer);
}

//
void rdbMain()
{
    while (true)
    {
        if (rdbTasks.empty())
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(10ms);
        }
        //
        RdbTask task;
        while (rdbTasks.try_pop(task))
        {
            printf("popped! %d\n", rdbTasks.unsafe_size());
            switch (task.type)
            {
            case RdbTaskType::POINT:
            {
                const auto& p = task.rdbPoint;
                rdb_printf("%f,%f,%f", p.r, p.g, p.b);
            }
            break;
            }
        }
    }
}



void rdbExit()
{
    rdbMainThread.join();
    ::closesocket(rdbContext.fd);
}

void rdbInit()
{
    if (!rdbContext.initialized)
    {
        rdbContext.initialized = true;
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData); // TODO: エラー処理
        rdbContext.fd = socket(AF_INET, SOCK_STREAM, 0);
        //
        struct sockaddr_in serv_name;
        serv_name.sin_family = AF_INET;
        serv_name.sin_addr.s_addr = ::htonl(0x7F000001L);
        serv_name.sin_port = ::htons(10000);
        ::connect(rdbContext.fd, (struct sockaddr*) & serv_name, sizeof(serv_name));
        ::atexit(rdbExit); // TODO: 普通にグローバル解放で使うようにする

        //
        rdbMainThread = std::thread([]() {rdbMain(); });
    }
}

//
void rdb_point(float r, float g, float b, int32_t group)
{
    puts("rdb_point");
    rdbInit();
    RdbTask task;
    task.rdbPoint.r = r;
    task.rdbPoint.g = g;
    task.rdbPoint.b = b;
    task.rdbPoint.group = group;
    task.type = RdbTaskType::POINT;
    rdbTasks.push(task);
}


