#ifndef _RDB_H_
#define _RDB_H_

#include <cstdint>
//
void rdbClear();
void rdbLabel(
    int32_t group, const char* label);
void rdbPoint(
    float x, float y, float z, 
    float r = 1.0f, float g = 1.0f, float b = 1.0f,
    int32_t group = 0);
void rdbLine(
    int32_t group,
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    float r0 = 1.0f, float g0 = 1.0f, float b0 = 1.0f,
    float r1 = 1.0f, float g1 = 1.0f, float b1 = 1.0f );

#if defined(RDB_IMPLIMATATION)
//
#include <WinSock2.h>
#include <concurrent_queue.h>
#pragma comment(lib, "Ws2_32.lib")
//
#include <thread>
#include <mutex>
#include <unordered_map>
#include <array>
//
enum class RdbTaskType : int32_t
{
    LABEL,
    POINT,
    LINE,
    TRIANGLE,
};
//
struct RdbTask
{
    RdbTaskType type;
    union
    {
        struct
        {
            const char* label;
            int32_t group;
        }rdbLabel;
        struct
        {
            float x;
            float y;
            float z;
            float r;
            float g;
            float b;
            int32_t group;
        }rdbPoint;
        struct
        {
            float x0;
            float y0;
            float z0;
            float x1;
            float y1;
            float z1;
            float r0;
            float g0;
            float b0;
            float r1;
            float g1;
            float b1;
            int32_t group;
        }rdbLine;
        struct
        {
            float x0;
            float y0;
            float z0;
            float x1;
            float y1;
            float z1;
            float x2;
            float y2;
            float z2;
            float r;
            float g;
            float b;
            int32_t group;
        }rdbTriangle;
    };
};
Concurrency::concurrent_queue<RdbTask> rdbTasks;

constexpr int32_t RDB_BUFFER_SIZE = 1024 * 4;

struct RdbContext
{
public:
    std::once_flag initialized;
    size_t bytesToSend = 0;
    SOCKET fd;

}rdbContext;

std::thread rdbMainThread;

//
void rdbInitCheck();
void rdb_printf(const char* fmt, ...);
void rdbMain();
void rdbExit();

//
void rdb_printf(const char* fmt, ...)
{
    //
    rdbInitCheck();
    //
    char buffer[RDB_BUFFER_SIZE];
    va_list argp;
    va_start(argp, fmt);
    const int32_t bytesToSend = vsnprintf(buffer, RDB_BUFFER_SIZE, fmt, argp);
    va_end(argp);
    //
    size_t sended = ::send(rdbContext.fd, buffer, bytesToSend, 0);
    // TODO: -1が帰ったらもう何も送らない状態にする
    //
    //printf("%s,[%d]", buffer, sended);
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
            switch (task.type)
            {
            case RdbTaskType::LABEL:
            {
                const auto& t = task.rdbLabel;
                rdb_printf("E %d,%s\n", t.group, t.label);
            }
            break;
            case RdbTaskType::POINT:
            {
                const auto& t = task.rdbPoint;
                rdb_printf("P %f,%f,%f,%f,%f,%f,%d\n",
                    t.x, t.y, t.z, t.r, t.g, t.b, t.group);
            }
            break;
            case RdbTaskType::LINE:
            {
                const auto& t = task.rdbLine;
                rdb_printf("L %f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d\n",
                    t.x0, t.y0, t.z0,
                    t.x1, t.y1, t.z1,
                    t.r0, t.g0, t.b0,
                    t.r1, t.g1, t.b1,
                    t.group);
            }
            break;
            case RdbTaskType::TRIANGLE:
            {
                const auto& t = task.rdbTriangle;
                rdb_printf("T %f,%f,%f,%f,%f,%f,%f,%f,%f,%d\n",
                    t.x0, t.y0, t.z0,
                    t.x1, t.y1, t.z1,
                    t.x2, t.y2, t.z2,
                    t.r, t.g, t.b,
                    t.group);
            }
            break;
            }
        }
    }
}
//
void rdbExit()
{
    rdbMainThread.join();
    ::closesocket(rdbContext.fd);
}
//
void rdbInitCheck()
{
    std::call_once(rdbContext.initialized, []()
        {
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
        });
}

//
void rdbLabel(int32_t group, const char* label)
{
    rdbInitCheck();
    RdbTask task;
    task.rdbLabel.label = label;
    task.rdbLabel.group = group;
    task.type = RdbTaskType::LABEL;
    rdbTasks.push(task);
}
//
void rdbPoint(
    float x, float y, float z,
    float r, float g, float b,
    int32_t group)
{
    rdbInitCheck();
    RdbTask task;
    task.rdbPoint.x = x;
    task.rdbPoint.y = y;
    task.rdbPoint.z = z;
    task.rdbPoint.r = r;
    task.rdbPoint.g = g;
    task.rdbPoint.b = b;
    task.rdbPoint.group = group;
    task.type = RdbTaskType::POINT;
    rdbTasks.push(task);
}
//
void rdbLine(
    int32_t group,
    float x0, float y0, float z0, 
    float x1, float y1, float z1,
    float r0, float g0, float b0,
    float r1, float g1, float b1 )
{
    rdbInitCheck();
    RdbTask task;
    auto& t = task.rdbLine;
    t.x0 = x0; t.y0 = y0; t.z0 = z0;
    t.x1 = x1; t.y1 = y1; t.z1 = z1;
    t.r0 = r0; t.g0 = g0; t.b0 = b0;
    t.r1 = r1; t.g1 = g1; t.b1 = b1;
    t.group = group;
    task.type = RdbTaskType::LINE;
    rdbTasks.push(task);
}
//
void rdbTriangle(
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    float r, float g, float b,
    int32_t group)
{
    rdbInitCheck();
    RdbTask task;
    auto& t = task.rdbTriangle;
    t.x0 = x0;
    t.y0 = y0;
    t.z0 = z0;
    t.x1 = x1;
    t.y1 = y1;
    t.z1 = z1;
    t.x2 = x2;
    t.y2 = y2;
    t.z2 = z2;
    t.r = r;
    t.g = g;
    t.b = b;
    t.group = group;
    task.type = RdbTaskType::TRIANGLE;
    rdbTasks.push(task);
}

#endif
#endif
