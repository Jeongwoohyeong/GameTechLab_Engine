#include "MeshData.h"

FMeshData FMeshData::SpriteMeshData = FMeshData
(
    {
    { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f} }, // 왼쪽 아래
    { {0.5f, -0.5f, 0.0f}, {1.0f, 1.0f} }, // 오른쪽 아래
    { {0.5f, 0.5f, 0.0f}, {1.0f, 0.0f} }, // 오른쪽 위
    { {-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f} }, // 왼쪽 위
    },
    { 0,1,2,0,2,3 }
);
