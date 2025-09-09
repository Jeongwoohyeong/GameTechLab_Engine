#pragma once
#include <span>
#include "Gizmo.h"
#include "Math.h"

constexpr FVector COLOR_R{ 1.0f, 0.0f, 0.0f };
constexpr FVector COLOR_G{ 0.0f, 1.0f, 0.0f };
constexpr FVector COLOR_B{ 0.0f, 0.0f, 1.0f };

constexpr FVector ROTATE_Y{ 0.0f, 0.0f, 0.0f }; // y축, 초록색
constexpr FVector ROTATE_Z{ 0.0f, 0.0f, -90.0f }; // z축, 파란색
constexpr FVector ROTATE_X{ 90.0f, 0.0f, 0.0f }; // x축, 빨간색

constexpr FVector TRANSLATE_Y{ 0.0f, 1.0f, 0.0f }; // y축, 초록색
constexpr FVector TRANSLATE_X{ 0.0f, 0.0f, 1.0f }; // z축, 파란색
constexpr FVector TRANSLATE_Z{ 1.0f, 0.0f, 0.0f }; // x축, 빨간색

struct axis
{
    FVector color;
    FVector rotate;
	FVector translate;
};
struct ID3D11Buffer;
struct FVertexSimple;

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

    axis axisInfo[3] =
    {
        {COLOR_R, ROTATE_X, TRANSLATE_X}, // x
        {COLOR_B, ROTATE_Y, TRANSLATE_Y}, // y
        {COLOR_G, ROTATE_Z, TRANSLATE_Z} // z
    };

    void Initialize(class URenderer* renderer, FTransform* transform);
    virtual void Render(class URenderer* renderer) override;
	virtual void CreateAABB() override;
    virtual void Release() override;

    FTransform UpdateGizmoTranformFromParent(axis a); // 기즈모 Transform Getter
    void TranslatePrimitive(int axis, float offSet);
    FMatrix worldMatrix{
        1, 0 ,0,
        0, 1, 0,
        0, 0, 1
    };
};