#include "pch.h"
#include "HeightFogComponent.h"

//IMPLEMENT_CLASS(UHeightFogComponent)

UHeightFogComponent::UHeightFogComponent()
    : FogDensity(0.0f)
    , FogHeightFalloff(0.0f)
    , StartDistance(0.0f)
    , FogCutoffDistance(0.0f)
    , FogMaxOpacity(1.0f)
    , FogInscatteringColor(FLinearColor::White)
{
    SetMaterial("HeightFog.hlsl");
}

void UHeightFogComponent::Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj)
{
    Renderer->UpdateHeightFogConstantBuffer(
        FogInscatteringColor,
        FogDensity,
        FogHeightFalloff,
        StartDistance,
        FogCutoffDistance,
        FogMaxOpacity
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
