#pragma once

#include "PrimitiveComponent.h"
#include "Color.h"

class UHeightFogComponent : public UPrimitiveComponent
{
public:
    DECLARE_CLASS(UHeightFogComponent, UPrimitiveComponent)
    UHeightFogComponent(); 
    virtual void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;

private:
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;

    FLinearColor FogInscatteringColor;
};
