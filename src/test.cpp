#define RDB_IMPLIMATATION
#include "rdb.hpp"
//
#include <cstdio>
#include <cstdint>
#include <chrono>
#include <thread>
//
#pragma comment(lib,"rdb.lib")

int main()
{
    while (true)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1000ms);
        rdb_point(0.0f, 1.0f, 2.0f);
    }
}

