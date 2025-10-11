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

    // Projection mode
    void SetUsePerspectiveProjection(bool bUsePerspective) { bUsePerspectiveProjection = bUsePerspective; }
    bool GetUsePerspectiveProjection() const { return bUsePerspectiveProjection; }

    void SetProjectionFOV(float InFOV) { ProjectionFOV = InFOV; }
    float GetProjectionFOV() const { return ProjectionFOV; }

    // Projection matrix getters
    static FMatrix GetDecalOrthoProjection();
    static FMatrix GetDecalPerspectiveProjection(float FovYDegrees);

    virtual UObject* Duplicate() override;
    virtual void DuplicateSubObjects() override;

    void Serialize(FObjectData* Data) override;
    void DeSerialize(FObjectData* Data) override;
    void TickComponent(float DeltaTime) override;

    void SetFadeEnabled(bool bIsEnable);

protected:
    ~UDecalComponent() override;

private:
    void UpdateFade(float DeltaTime);
    void RenderFrustumLines(URenderer* Renderer);

private:
    UOBoundingBoxComponent OBB;
    FString TexturePath = "Editor/Decal/doro.dds";
    FString MaterialPath = "Decal.hlsl";
    // Duration, min, max, Alpha
    FVector4 DefaultFadeProperties = {5.0f, 0.0f, 1.0f, 1.0f};
    FVector4 FadeProperties = DefaultFadeProperties;
    bool bIsFadeEnabled = false;
    bool bIsFadeOut = true;
    float ElapsedTime = 0.0f;
    

    // Projection settings
    bool bUsePerspectiveProjection = false;  // false = Ortho, true = Perspective
    float ProjectionFOV = 45.0f;  // FOV in degrees (for perspective mode)
    
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


    inline static const FMatrix DecalViewAdjustMatrix =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 1.0f
    };
};