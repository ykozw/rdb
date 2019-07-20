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
    while (true)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1000ms);
        rdbPoint(tmp, tmp*2.0f, tmp*3.0f);
        tmp += 1.0f;
    }
}

