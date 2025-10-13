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
}
