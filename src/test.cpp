#define RDB_IMPLIMATATION
#include "rdb.hpp"
//
#include <cstdio>
#include <cstdint>
#include <chrono>
#include <thread>

int main()
{
    float tmp = 0.0f;
    rdbLabel("TEST", 0);
    float theta = 0.0f;
    const float d = 100.0f;
    while (true)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);
        rdbPoint(std::sinf(theta) * d + d, std::cosf(theta) * d + d, tmp*0.0f);
        theta += 0.01f;
        //rdb_printf("ABC\n");
        tmp += 1.0f;
    }
}

