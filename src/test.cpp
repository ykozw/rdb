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
    float theta = 0.0f;

    float vertex[][3] = {
  { 0.0, 0.0, 0.0 },
  { 1.0, 0.0, 0.0 },
  { 1.0, 1.0, 0.0 },
  { 0.0, 1.0, 0.0 },
  { 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 1.0 },
  { 1.0, 1.0, 1.0 },
  { 0.0, 1.0, 1.0 }
    };

    int edge[][2] = {
      { 0, 1 },
      { 1, 2 },
      { 2, 3 },
      { 3, 0 },
      { 4, 5 },
      { 5, 6 },
      { 6, 7 },
      { 7, 4 },
      { 0, 4 },
      { 1, 5 },
      { 2, 6 },
      { 3, 7 }
    };
    rdbLabel(0, "Cube");
    for (int i = 0; i < 12; i++) {
        const float* v0 = vertex[edge[i][0]];
        const float* v1 = vertex[edge[i][1]];
        rdbLine(
            0,
            v0[0], v0[1], v0[2], 
            v1[0], v1[1], v1[2]);
    }
    //return 0;

    rdbLabel(1, "Circle");
    for(int32_t i=0;i<128;++i)
    {
        /*using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);*/

        if (true)
        {
            const float tb = theta * 0.05f;
            const float r = (std::sinf(tb + 0.0f) + 1.0f) * 0.5f;
            const float g = (std::sinf(tb + 3.14f / 3.0f) + 1.0f) * 0.5f;
            const float b = (std::sinf(tb + 3.14f * 2.0f / 3.0f) + 1.0f) * 0.5f;

            const float c = 1.0f;
            const float d = c * std::cosf(theta * 0.01f);
            rdbPoint(
                std::sinf(theta) * d + c, std::cosf(theta) * d + c, 0.0f,
                r, g, b,
                0.0f);
            theta += 0.01f;
            tmp += 1.0f;
        }
        if (false)
        {
            const float c = 1.0f;
            const float x0 = c * std::cosf(theta) + c;
            const float y0 = c * std::sinf(theta) + c;
            const float dt = 0.6f;
            const float x1 = c * std::cosf(theta + dt) + c;
            const float y1 = c * std::sinf(theta + dt) + c;

            const float tb = theta * 0.3f;
            const float r = (std::sinf(tb + 0.0f) + 1.0f) * 0.5f;
            const float g = (std::sinf(tb + 3.14f / 3.0f) + 1.0f) * 0.5f;
            const float b = (std::sinf(tb + 3.14f * 2.0f / 3.0f) + 1.0f) * 0.5f;

            rdbLine(1, x0, y0, 0.0f, x1, y1, 0.0f, 1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f);
            theta += 0.6f;
        }

    }
}

