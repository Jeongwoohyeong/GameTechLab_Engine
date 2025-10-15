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

	// 실제로 그리지 않고 FireBallBufferType만 수집합니다.
	void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;
	void AddDebugLine(URenderer* Renderer);
	
	void Serialize(FObjectData* Data) override;
	void DeSerialize(FObjectData* Data) override;

	UObject* Duplicate() override;
	void DuplicateSubObjects() override;
private:
	float Intensity;
	float Radius;
	float RadiusFallOff;
	FLinearColor Color;
};

