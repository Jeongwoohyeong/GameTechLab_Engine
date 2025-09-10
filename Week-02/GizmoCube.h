#pragma once
#include "FVertexStruct.h"

// -------------------- 실린더 위에 놓일 큐브 데이터 --------------------

static FVertexSimple GGizmoCubeVertices[] =
{
    //         POSITION              COLOR
    { -0.5f,  1.8f, -0.5f,   1,1,1,1 }, // v0: 좌하단 뒤
    {  0.5f,  1.8f, -0.5f,   1,1,1,1 }, // v1: 우하단 뒤
    {  0.5f,  1.6f, -0.5f,   1,1,1,1 }, // v2: 우상단 뒤
    { -0.5f,  1.6f, -0.5f,   1,1,1,1 }, // v3: 좌상단 뒤
    { -0.5f,  1.8f,  0.5f,   1,1,1,1 }, // v4: 좌하단 앞
    {  0.5f,  1.8f,  0.5f,   1,1,1,1 }, // v5: 우하단 앞
    {  0.5f,  1.6f,  0.5f,   1,1,1,1 }, // v6: 우상단 앞
    { -0.5f,  1.6f,  0.5f,   1,1,1,1 }  // v7: 좌상단 앞
};

static uint32 GGizmoCubeIndices[] =
{
    // 앞면 (Front Face)
    4, 7, 6,   4, 6, 5,
    // 뒷면 (Back Face)
    3, 0, 1,   3, 1, 2,
    // 윗면 (Top Face)
    7, 3, 2,   7, 2, 6,
    // 아랫면 (Bottom Face)
    0, 4, 5,   0, 5, 1,
    // 왼쪽 면 (Left Face)
    0, 7, 4,   0, 3, 7,
    // 오른쪽 면 (Right Face)
    6, 2, 1,   6, 1, 5
};