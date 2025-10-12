#pragma once

#include "PrimitiveComponent.h"

class UPrimitiveComponent;
class UDecalComponent;

/**
 * 가짜 스포트라이트(Decal-based)의 핵심 로직과 데이터를 담당하는 컴포넌트입니다.
 * 하위에 DecalComponent를 소유하고 제어합니다.
 */

class USpotlightComponent : public UPrimitiveComponent
{
public:
	DECLARE_CLASS(USpotlightComponent, UPrimitiveComponent)
	USpotlightComponent();

	// TODO : Only render in editor after implementing genuine spotlight logic
	virtual void Render(URenderer* Renderer, const FMatrix& View, const FMatrix& Proj) override;
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime) override;
	virtual UObject* Duplicate() override;

protected:
	~USpotlightComponent() override;
private:
	FVector CalculateScale(float InConeAngle, float InAttenuationRadius);

public:	
	// --- Spotlight Properties ---

	float GetConeAngle() const;
	void SetConeAngle(float InConeAngle);

	float GetAttenuationRadius() const;
	void SetAttenuationRadius(float InAttenuationRadius);

private:
	/** 스포트라이트 원뿔의 각도 (단위: 도) */
	float ConeAngle;

	/** 스포트라이트가 도달하는 최대 거리 */
	float AttenuationRadius;

	/** 라이트의 색상 */
	// TODO : not used yet
	FVector4 LightColor;

	/** 라이트의 강도 */
	// TODO : not used yet
	float Intensity;

private:
	// 이 컴포넌트가 제어할 Decal 컴포넌트입니다.
	UDecalComponent* DecalComponent;
};
