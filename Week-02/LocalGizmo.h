#pragma once
#include <span>
#include "Gizmo.h"

constexpr FVector COLOR_R{ 1.0f, 0.0f, 0.0f };
constexpr FVector COLOR_G{ 0.0f, 1.0f, 0.0f };
constexpr FVector COLOR_B{ 0.0f, 0.0f, 1.0f };

class LocalGizmo : public Gizmo
{
public:
    
    std::span<const FVertexSimple> coneVerts;
    std::span<const unsigned int>  coneIdx;
    std::span<const FVertexSimple> cylinderVerts;
    std::span<const unsigned int>  cylinderIdx;

    ID3D11Buffer* gizmoConeVerticesBuffer = nullptr;
    ID3D11Buffer* gizmoConeIndicesBuffer = nullptr;
    ID3D11Buffer* gizmoCylinderVerticesBuffer = nullptr;
    ID3D11Buffer* gizmoCylinderIndicesBuffer = nullptr;

    void Initialize(URenderer* renderer);
    void Render(URenderer* renderer);
    void Release();
};