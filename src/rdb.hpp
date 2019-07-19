
#define DLLEXPORT extern "C" __declspec(dllexport)

#include <cstdint>
//enum class RDB_MODE : int32_t
//{
//
//};
//
//DLLEXPORT int rdb_mode(RDB_MODE mode, int32_t group = 0);
DLLEXPORT void rdb_max_buffer(int32_t maxBuffeInBytes);
DLLEXPORT void rdb_color(float r, float g, float b, int32_t group = 0);
DLLEXPORT void rdb_label(const char* label, int32_t group = 0);
DLLEXPORT void rdb_point(float x, float y, float z, int32_t group = 0);
DLLEXPORT int rdb_line(float x, float y, float z, int32_t group = 0);
DLLEXPORT int rdb_normal(float x, float y, float z, int32_t group = 0);
DLLEXPORT int rdb_triangle(
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    float x2, float y2, float z2, 
    int32_t group = 0);
