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
    while (true)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);

        if (false)
        {
            const float tb = theta * 0.05f;
            const float r = (std::sinf(tb + 0.0f) + 1.0f) * 0.5f;
            const float g = (std::sinf(tb + 3.14f / 3.0f) + 1.0f) * 0.5f;
            const float b = (std::sinf(tb + 3.14f * 2.0f / 3.0f) + 1.0f) * 0.5f;

            const float c = 100.0f;
            const float d = c * std::cosf(theta * 0.01f);
            rdbPoint(
                std::sinf(theta) * d + c, std::cosf(theta) * d + c, 0.0f,
                r, g, b,
                0.0f);
            theta += 0.01f;
            tmp += 1.0f;
        }
        if (true)
        {
            const float c = 100.0f;
            const float x0 = c * std::cosf(theta) + c;
            const float y0 = c * std::sinf(theta) + c;
            const float dt = 0.1f;
            const float x1 = c * std::cosf(theta + dt) + c;
            const float y1 = c * std::sinf(theta + dt) + c;
            rdbLine(x0, y0, 0.0f, x1, y1, 0.0f);
            theta += 0.1f;
        }

    }
}

