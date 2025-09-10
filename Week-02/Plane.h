#pragma once
#include "FVertexStruct.h"
#include "Types.h"

// -------------------- 평면 정점 (Vertices) --------------------

// 1. 정점 배열 (Vertex Array)
// 사각형의 네 꼭짓점을 정의
static FVertexSimple GPlaneVertices[] =
{
    //     X,     Y,    Z,      R,   G,   B,   A
    { -0.5f,  0.5f, 0.0f,    1.0f, 0.0f, 0.0f, 1.0f }, // 0: 좌상단 (빨간색)
    {  0.5f,  0.5f, 0.0f,    0.0f, 1.0f, 0.0f, 1.0f }, // 1: 우상단 (녹색)
    {  0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f, 1.0f }, // 2: 우하단 (파란색)
    { -0.5f, -0.5f, 0.0f,    1.0f, 1.0f, 0.0f, 1.0f }  // 3: 좌하단 (노란색)
};

// 2. 인덱스 배열 (Index Array)
// 두 개의 삼각형으로 사각형을 만듭니다.
static uint32 GPlaneIndices[] =
{
    0, 1, 2,  // 첫 번째 삼각형
    0, 2, 3   // 두 번째 삼각형
};