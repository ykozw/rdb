#ifndef _RDB_H_
#define _RDB_H_

#include <cstdint>
//
void rdb_max_buffer(int32_t maxBuffeInBytes);
void rdbLabel(const char* label, int32_t group = 0);
void rdbColor(float r, float g, float b, int32_t group = 0);
void rdbPoint(float x, float y, float z, int32_t group = 0);
void rdbLine(
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    int32_t group = 0);
void rdbNormal(
    float x, float y, float z,
    float nx, float ny, float nz,
    int32_t group = 0);
void rdbTriangle(
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    int32_t group = 0);


#if defined(RDB_IMPLIMATATION)
//
#include <WinSock2.h>
#include <concurrent_queue.h>
#pragma comment(lib, "Ws2_32.lib")
//
#include <thread>
#include <mutex>
//
enum class RdbTaskType : int32_t
{
    LABEL,
    COLOR,
    POINT,
    LINE,
    NORMAL,
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
            float r;
            float g;
            float b;
            int32_t group;
        }rdbColor;
        struct
        {
            float x;
            float y;
            float z;
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
            int32_t group;
        }rdbLine;
        struct
        {
            float x;
            float y;
            float z;
            float nx;
            float ny;
            float nz;
            int32_t group;
        }rdbNormal;
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
            switch (task.type)
            {
            case RdbTaskType::LABEL:
            {
                const auto& t = task.rdbLabel;
                rdb_printf("E %s", t.label, t.group);
            }
            break;
            case RdbTaskType::COLOR:
            {
                const auto& t = task.rdbColor;
                rdb_printf("C %f,%f,%f,%d", t.r, t.g, t.b, t.group);
            }
            break;
            case RdbTaskType::POINT:
            {
                const auto& t = task.rdbPoint;
                rdb_printf("P %f,%f,%f,%d", t.x, t.y, t.z, t.group);
            }
            break;
            case RdbTaskType::LINE:
            {
                const auto& t = task.rdbLine;
                rdb_printf("L %f,%f,%f,%f,%f,%f,%d",
                    t.x0, t.y0, t.z0,
                    t.x1, t.y1, t.z1,
                    t.group);
            }
            break;
            case RdbTaskType::NORMAL:
            {
                const auto& t = task.rdbNormal;
                rdb_printf("N %f,%f,%f,%f,%f,%f,%f,%f,%f,%d",
                    t.x, t.y, t.z,
                    t.nx, t.ny, t.nz,
                    t.group);
            }
            break;
            case RdbTaskType::TRIANGLE:
            {
                const auto& t = task.rdbTriangle;
                rdb_printf("T %f,%f,%f,%f,%f,%f,%f,%f,%f,%d",
                    t.x0, t.y0, t.z0,
                    t.x1, t.y1, t.z1,
                    t.x2, t.y2, t.z2,
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
void rdbLabel(const char* label, int32_t group)
{
    rdbInitCheck();
    RdbTask task;
    task.rdbLabel.label = label;
    task.rdbLabel.group = group;
    task.type = RdbTaskType::LABEL;
    rdbTasks.push(task);
}
//
void rdbColor(float r, float g, float b, int32_t group)
{
    rdbInitCheck();
    RdbTask task;
    task.rdbColor.r = r;
    task.rdbColor.g = g;
    task.rdbColor.b = b;
    task.rdbColor.group = group;
    task.type = RdbTaskType::COLOR;
    rdbTasks.push(task);
}
//
void rdbPoint(float x, float y, float z, int32_t group)
{
    rdbInitCheck();
    RdbTask task;
    task.rdbPoint.x = x;
    task.rdbPoint.y = y;
    task.rdbPoint.z = z;
    task.rdbPoint.group = group;
    task.type = RdbTaskType::POINT;
    rdbTasks.push(task);
}
//
void rdbLine(
    float x0, float y0, float z0, 
    float x1, float y1, float z1,
    int32_t group )
{
    rdbInitCheck();
    RdbTask task;
    auto& t = task.rdbLine;
    t.x0 = x0;
    t.y0 = y0;
    t.z0 = z0;
    t.x1 = x1;
    t.y1 = y1;
    t.z1 = z1;
    t.group = group;
    task.type = RdbTaskType::LINE;
    rdbTasks.push(task);
}
//
void rdbNormal(
    float x, float y, float z,
    float nx, float ny, float nz,
    int32_t group)
{
    rdbInitCheck();
    RdbTask task;
    auto& t = task.rdbNormal;
    t.x = x;
    t.y = y;
    t.z = z;
    t.nx = nx;
    t.ny = ny;
    t.nz = nz;
    t.group = group;
    task.type = RdbTaskType::NORMAL;
    rdbTasks.push(task);
}
//
void rdbTriangle(
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    float x2, float y2, float z2,
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
    t.group = group;
    task.type = RdbTaskType::TRIANGLE;
    rdbTasks.push(task);
}

#endif
#endif