#include "pch.h"
#include "HeightFogComponent.h"

//IMPLEMENT_CLASS(UHeightFogComponent)

UHeightFogComponent::UHeightFogComponent()
    : FogDensity(0.1f)       // Default fog density (adjust based on scene scale)
    , FogHeightFalloff(0.2f)  // Default height falloff
    , StartDistance(0.0f)      // Fog starts immediately
    , FogCutoffDistance(500.0f) // Fog cutoff distance
    , FogMaxOpacity(1.0f)      // Maximum fog opacity
    , FogInscatteringColor(FLinearColor::Blue) // White fog
{
    SetMaterial("HeightFog.hlsl");
}

void UHeightFogComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
    // Get fog origin height from Actor's world position
    float FogHeightOffset = 0.0f;
    if (AActor* OwnerActor = GetOwner())
    {
        FogHeightOffset = OwnerActor->GetActorLocation().Z;
    }

    Renderer->UpdateHeightFogConstantBuffer(
        FogInscatteringColor,
        FogDensity,
        FogHeightFalloff,
        StartDistance,
        FogCutoffDistance,
        FogMaxOpacity,
        FogHeightOffset
    );
    Renderer->UpdateInvMatrixBuffer(
        FMatrix::Identity(),      // InvWorld - world inverse (identity for now)
        View.Inverse(),           // InvView - contains camera position
        Proj.Inverse()            // InvProj - projection inverse
    );
    Renderer->PrepareShader(Material->GetShader());

    // 렌더 상태 설정 (블렌딩, 깊이 스텐실)
    Renderer->OMSetBlendState(true); // 블렌딩 활성화
    // 깊이 쓰기는 끄고, 깊이 테스트는 항상 통과 (기존 지오메트리 위에 그려지도록)
    Renderer->OMSetDepthStencilState(EComparisonFunc::Always);

    Renderer->DrawFullScreenPass();

    // 렌더 상태 복원 (선택 사항이지만 좋은 습관)
    Renderer->OMSetBlendState(false); // 블렌딩 비활성화
    Renderer->OMSetDepthStencilState(EComparisonFunc::LessEqual); // 기본 깊이 스텐실 상태로 복원
}

UObject* UHeightFogComponent::Duplicate()
{
    UHeightFogComponent* DuplicatedComponent = Cast<UHeightFogComponent>(NewObject(GetClass()));
    CopyCommonProperties(DuplicatedComponent);

    DuplicatedComponent->Material = this->Material;
    DuplicatedComponent->FogDensity = this->FogDensity;
    DuplicatedComponent->FogHeightFalloff = this->FogHeightFalloff;
    DuplicatedComponent->StartDistance = this->StartDistance;
    DuplicatedComponent->FogCutoffDistance = this->FogCutoffDistance;
    DuplicatedComponent->FogMaxOpacity = this->FogMaxOpacity;
    DuplicatedComponent->FogInscatteringColor = this->FogInscatteringColor;

    DuplicatedComponent->DuplicateSubObjects();
    return DuplicatedComponent;
}

void UHeightFogComponent::DuplicateSubObjects()
{
    Super_t::DuplicateSubObjects();
}

void UHeightFogComponent::Serialize(FObjectData* Data)
{
    FHeightFogComponentData* ComponentData = dynamic_cast<FHeightFogComponentData*>(Data);
    assert(ComponentData, "UHeightFogComponent::Serialize got wrong data type.");

    USceneComponent::Serialize(Data);

    ComponentData->FogDensity = FogDensity;
    ComponentData->FogHeightFalloff = FogHeightFalloff;
    ComponentData->StartDistance = StartDistance;
    ComponentData->FogCutoffDistance = FogCutoffDistance;
    ComponentData->FogMaxOpacity = FogMaxOpacity;
    ComponentData->FogInscatteringColor = FogInscatteringColor;
}

void UHeightFogComponent::DeSerialize(FObjectData* Data)
{
    FHeightFogComponentData* ComponentData = dynamic_cast<FHeightFogComponentData*>(Data);
    assert(ComponentData, "UHeightFogComponent::DeSerialize got wrong data type.");

    USceneComponent::DeSerialize(Data);

    FogDensity = ComponentData->FogDensity;
    FogHeightFalloff = ComponentData->FogHeightFalloff;
    StartDistance = ComponentData->StartDistance;
    FogCutoffDistance = ComponentData->FogCutoffDistance;
    FogMaxOpacity = ComponentData->FogMaxOpacity;
    FogInscatteringColor = ComponentData->FogInscatteringColor;
}
