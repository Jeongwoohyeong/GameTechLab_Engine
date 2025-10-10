#pragma once

#include "PrimitiveComponent.h"
#include "OBoundingBoxComponent.h"

class UDecalComponent : public UPrimitiveComponent
{
public:
    DECLARE_CLASS(UDecalComponent, UPrimitiveComponent)
    UDecalComponent();

    virtual void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;

protected:
    ~UDecalComponent() override;

private:
    UOBoundingBoxComponent OBB;
    
    // OBB 박스에 Vertex 정보를 전달하기 위한 배열
    // Local 공간에서 Decal Volume은 한 변의 길이가 1인 정육면체이다.
    inline static const TArray<FVector> DecalVolumeVertices =
    {
        {0.5f, 0.5f, 0.5f},
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, 0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},
        {-0.5f, -0.5f, 0.5f}
    };
};