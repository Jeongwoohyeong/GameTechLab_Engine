#pragma once

#include "PrimitiveComponent.h"
#include "OBoundingBoxComponent.h"
#include "StaticMeshComponent.h"

enum class EFadeTypes : uint8
{
    FadeIn, 
    FadeOut,
    FadeLoop
};

struct FFadeProperty
{
    EFadeTypes Type;
    FVector4 AlphaProperties;
    bool bIsFadeEnabled;
    bool bIsFadeStart;
    bool bIsLoop;
    FFadeProperty()
    : Type(EFadeTypes::FadeIn), AlphaProperties{}, bIsFadeEnabled(false), bIsFadeStart(false), bIsLoop(false) {};
    FFadeProperty(EFadeTypes InType, FVector4 InAlphaProperties, bool InbIsFadeEnabled, bool InbIsFadeStart, bool InbIsLoop)
    : Type(InType), AlphaProperties{InAlphaProperties}, bIsFadeEnabled(InbIsFadeEnabled),
        bIsFadeStart(InbIsFadeStart), bIsLoop(InbIsLoop) {};
    FFadeProperty(const FFadeProperty& InOther)
    {
        Type = InOther.Type;
        AlphaProperties = InOther.AlphaProperties;
        bIsFadeEnabled = InOther.bIsFadeEnabled;
        bIsFadeStart = InOther.bIsFadeStart;
        bIsLoop = InOther.bIsLoop;
    }
};

class UDecalComponent : public UPrimitiveComponent
{
public:
    DECLARE_CLASS(UDecalComponent, UPrimitiveComponent);
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

    const FString& GetTexturePath() const { return TexturePath; }
    FFadeProperty& GetFadeProperties() { return CurrentFadeProperty;}
    void ResetFadeProperties();

    // Projection mode
    void SetUsePerspectiveProjection(bool bUsePerspective) { bUsePerspectiveProjection = bUsePerspective; }
    bool GetUsePerspectiveProjection() const { return bUsePerspectiveProjection; }

    void SetProjectionFOV(float InFOV) { ProjectionFOV = InFOV; }
    float GetProjectionFOV() const { return ProjectionFOV; }

    // Projection matrix getters
    static FMatrix GetDecalOrthoProjection();
    static FMatrix GetDecalPerspectiveProjection(float FovYDegrees);

    UOBoundingBoxComponent* GetOBBComponent() { return &OBB; }

    virtual UObject* Duplicate() override;
    virtual void DuplicateSubObjects() override;

    void Serialize(FObjectData* Data) override;
    void DeSerialize(FObjectData* Data) override;
    void TickComponent(float DeltaTime) override;
    
    void UpdateFade(float DeltaTime);

protected:
    ~UDecalComponent() override;

private:
    void RenderFrustumLines(URenderer* Renderer);

private:
    UOBoundingBoxComponent OBB;
    FString TexturePath = "Editor/Decal/doro.dds";
    FString MaterialPath = "Decal.hlsl";
    // Duration, min, max, Alpha
    FFadeProperty DefaultFadeProperty = {
        EFadeTypes::FadeIn, FVector4({5.0f, 0.0f, 1.0f, 1.0f}),
        false, false, false};
    FFadeProperty CurrentFadeProperty = DefaultFadeProperty;

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
