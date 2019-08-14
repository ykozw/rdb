# What's this?

__rdb is a Ray-tracing debugger inspired by [vdb](https://github.com/zdevito/vdb).__

Feature
- Ultimately simple APIs
- Thread safety
- Single header only

# How to use

1. run "rdb.exe"
2. include "rdb.hpp" and call APIs
3. run.

# API

```c
void rdbPoint(
    float x, float y, float z,
    float r, float g, float b,
    int32_t group);
```

```c
void rdbLine(
    float x0, float y0, float z0, 
    float x1, float y1, float z1,
    float r0, float g0, float b0,
    float r1, float g1, float b1,
    int32_t group);
```

```c
void rdbTriangle(
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    float r, float g, float b,
    int32_t group)
```

That's all.