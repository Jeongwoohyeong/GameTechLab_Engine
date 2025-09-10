#pragma once
#include <vector>
#include "FVertexStruct.h"
// 전방 선언
class URenderer;
class FTransform;

struct ID3D11Buffer;

class WorldGizmo
{
public:
    WorldGizmo() = default;
    ~WorldGizmo() = default;

    void Initialize(URenderer* renderer);
    void Render(URenderer* renderer);
	void Release(); 

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
    FTransform* ParentTransform;

    std::vector<FVertexSimple> xzGridVertices;
    std::vector<unsigned int>  xzGridIndices;

    ID3D11Buffer* gridVerticesBuffer = nullptr;
    ID3D11Buffer* gridIndicesBuffer = nullptr;

    // 색상 프리셋
    FVertexSimple Minor = { 0,0,0,  0.55f,0.55f,0.55f,1.0f };
    FVertexSimple Major = { 0,0,0,  0.30f,0.30f,0.30f,1.0f };
    FVertexSimple YColor = { 0,0,0,  0.4f,1.0f,0.4f,1.0f };
    FVertexSimple ZColor = { 0,0,0,  0.2f,0.4f,1.0f,1.0f };
    FVertexSimple XColor = { 0,0,0,  1.0f,0.2f,0.2f,1.0f };
};
