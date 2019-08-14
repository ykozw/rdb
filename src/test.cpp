#define _USE_MATH_DEFINES
#define RDB_IMPLIMATATION
#include "rdb.hpp"
//
#include <cstdio>
#include <cstdint>
#include <random>
#include <cmath>

int main()
{
    // 20–Ê‘Ì
    for (int32_t f=0;f<20;++f)
    {
        const float d = 1.0f + std::sqrtf(5.0f) * 0.5f;
        const float vtx[] = {
            0.0f, -1.0f, -d,
            0.0f,+1.0f, -d,
            0.0f, -1.0f,+d,
            0.0f, 1.0f, +d,
            -d,   0.0f, -1.0f,
            -d,   0.0f, +1.0f,
            d,    0.0f, -1.0f,
            d,    0.0f, +1.0f,
            -1.0f, -d,  +0.0f,
            +1.0f,-d,   +0.0f,
            -1.0f, +d,  +0.0f,
            +1.0f, +d,  +0.0f,
        };
        int32_t index[] = {
            0, 1, 6,
            1, 0, 4,
            2, 3, 5,
            3, 2, 7,
            //
            4, 5, 10,
            5, 4, 8,
            6, 7, 9,
            7, 6, 11,
            //
            8, 9, 2,
            9, 8, 0,
            10, 11, 1,
            11, 10, 3,
            //
            0, 6, 9,
            0, 8, 4,
            1, 4, 10,
            1, 11, 6,
            //
            2, 5, 8,
            2, 9, 7,
            3, 7, 11,
            3, 10, 5
        };
        const int32_t i0 = index[f * 3 + 0];
        const int32_t i1 = index[f * 3 + 1];
        const int32_t i2 = index[f * 3 + 2];
        const float x0 = vtx[i0 * 3 + 0];
        const float y0 = vtx[i0 * 3 + 1];
        const float z0 = vtx[i0 * 3 + 2];
        const float x1 = vtx[i1 * 3 + 0];
        const float y1 = vtx[i1 * 3 + 1];
        const float z1 = vtx[i1 * 3 + 2];
        const float x2 = vtx[i2 * 3 + 0];
        const float y2 = vtx[i2 * 3 + 1];
        const float z2 = vtx[i2 * 3 + 2];
        const float r = ((x0 + x1 + x2) + 3.0f) * 0.3f;
        const float g = ((y0 + y1 + y2) + 3.0f) * 0.3f;
        const float b = ((z0 + z1 + z2) + 3.0f) * 0.3f;
        rdbTriangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, r, g, b, 0);
    }

    std::mt19937 mt(0x123);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for(int32_t sn=0;sn<1024;++sn)
    {
        const float z = dist(mt)*2.0f - 1.0f;
        const float phi = dist(mt) * 2.0f * float(M_PI);
        const float costheta = std::sqrtf(1.0f - z * z);
        const float x = costheta * std::sinf(phi);
        const float y = costheta * std::cosf(phi);
        rdbPoint(x, y, z, x, y, z);
    }
}
