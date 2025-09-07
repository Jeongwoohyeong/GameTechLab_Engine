#pragma once
#include "Gizmo.h"
#include <vector>

// 전방 선언
class URenderer;
class UMesh;
struct ID3D11Buffer;

class WorldGizmo : public Gizmo
{
public:
    WorldGizmo() = default;
    ~WorldGizmo() = default;

    void Initialize(UMesh InMesh);
    void Render(URenderer* renderer);

private:
    void BuildGridXZwithYaxis(
        float Spacing,
        int   HalfCount,
        int   MajorEvery,
        float YLevel,
        std::vector<FVertexSimple>& outVertices,
        std::vector<unsigned int>& outIndices,
        const FVertexSimple& MinorColor,
        const FVertexSimple& MajorColor,
        const FVertexSimple& AxisXColor,
        const FVertexSimple& AxisZColor,
        const FVertexSimple& AxisYColor);

private:
    std::vector<FVertexSimple> xzGridVertices;
    std::vector<unsigned int>  xzGridIndices;

    UMesh mesh;
    ID3D11Buffer* gridVerticesBuffer = nullptr;
    ID3D11Buffer* gridIndicesBuffer = nullptr;

    // 색상 프리셋
    FVertexSimple Minor = { 0,0,0,  0.55f,0.55f,0.55f,1.0f };
    FVertexSimple Major = { 0,0,0,  0.30f,0.30f,0.30f,1.0f };
    FVertexSimple XCol = { 0,0,0,  1.00f,0.20f,0.20f,1.0f };
    FVertexSimple ZCol = { 0,0,0,  0.20f,0.40f,1.00f };
    FVertexSimple YCol = { 0,0,0,  0.40f,1.00f,0.40f,1.0f };
};
