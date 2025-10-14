#pragma once
#include "PrimitiveComponent.h"

class UFireBallComponent: public UPrimitiveComponent 
{
public:
	DECLARE_CLASS(UFireBallComponent, UPrimitiveComponent)
	UFireBallComponent();

	// ======= Getter ========
	float GetIntensity() const { return Intensity; }
	float GetRadius() const { return Radius; }
	float GetRadiusFallOff() const { return RadiusFallOff; }
	const FLinearColor& GetColor() const { return Color; }

	// ======= Setter =======
	void SetIntensity(float InIntensity) { Intensity = InIntensity; }
	void SetRadius(float InRadius) { Radius = InRadius; }
	void SetRadiusFallOff(float InRadiusFallOff) { RadiusFallOff = InRadiusFallOff; }
	void SetColor(FLinearColor InColor) { Color = InColor; }

	
	void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;
	
	UObject* Duplicate() override;
	void DuplicateSubObjects() override;
private:
	float Intensity;
	float Radius;
	float RadiusFallOff;
	FLinearColor Color;
};

