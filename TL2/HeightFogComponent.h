#pragma once

#include "PrimitiveComponent.h"
#include "Color.h"

class UHeightFogComponent : public UPrimitiveComponent
{
public:
    DECLARE_CLASS(UHeightFogComponent, UPrimitiveComponent)
    UHeightFogComponent();
    virtual void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;

    // Getters
    float GetFogDensity() const { return FogDensity; }
    float GetFogHeightFalloff() const { return FogHeightFalloff; }
    float GetStartDistance() const { return StartDistance; }
    float GetFogCutoffDistance() const { return FogCutoffDistance; }
    float GetFogMaxOpacity() const { return FogMaxOpacity; }
    FLinearColor GetFogInscatteringColor() const { return FogInscatteringColor; }

    // Setters
    void SetFogDensity(float Value) { FogDensity = Value; }
    void SetFogHeightFalloff(float Value) { FogHeightFalloff = Value; }
    void SetStartDistance(float Value) { StartDistance = Value; }
    void SetFogCutoffDistance(float Value) { FogCutoffDistance = Value; }
    void SetFogMaxOpacity(float Value) { FogMaxOpacity = Value; }
    void SetFogInscatteringColor(const FLinearColor& Value) { FogInscatteringColor = Value; }

    // Serialization & Duplication
    virtual UObject* Duplicate() override;
    virtual void DuplicateSubObjects() override;
    void Serialize(FObjectData* Data) override;
    void DeSerialize(FObjectData* Data) override;

private:
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;

    FLinearColor FogInscatteringColor;
};
