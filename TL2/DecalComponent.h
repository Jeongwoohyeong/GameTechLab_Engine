#pragma once

#include "PrimitiveComponent.h"
#include "OBoundingBoxComponent.h"
#include "StaticMeshComponent.h"

class UDecalComponent : public UPrimitiveComponent
{
public:
    DECLARE_CLASS(UDecalComponent, UPrimitiveComponent)
    UDecalComponent();

    virtual void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;

    void ProjectDecal
    (
        URenderer* Renderer,
        UStaticMeshComponent* StaticMeshComponent,
        const FMatrix& View,
        const FMatrix& Proj
    );
    void SetTexture(const FString& TexturePath);
    void SetFadeProperties(const FVector4& FadeProperties);
    
    const FString& GetTexturePath() const { return TexturePath; }
    FVector4 GetFadeProperties() const { return FadeProperties;}
    void ResetFadeProperties();

protected:
    ~UDecalComponent() override;

private:
    UOBoundingBoxComponent OBB;
    FString TexturePath = "Editor/Decal/PointLight_64x.dds";
    // Duration, min, max, Alpha
    FVector4 DefaultFadeProperties = {5.0f, 0.0f, 1.0f, 1.0f};
    FVector4 FadeProperties = DefaultFadeProperties;
    
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

    inline static const FMatrix DecalProjection =
    {
        2.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 2.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    inline static const FMatrix DecalViewRotation =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
};