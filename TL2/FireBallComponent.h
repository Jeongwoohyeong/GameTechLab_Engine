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

	void BeginPlay() override;
	void TickComponent(float DeltaTime) override;
	// Tick전용
	void StartExplosion();

private:
	float Intensity;
	float Radius;
	float RadiusFallOff;
	FLinearColor Color;

	// Tick 전용
	bool bIsExploding = false;
	float EffectTimer = 0.0f;         // 효과의 전체 시간을 추적하는 타이머
	float ExplosionDuration = 1.0f; // 폭발이 지속되는 시간 (초)
	float CooldownDuration = 2.0f;  // 폭발 후 다음 폭발까지 대기하는 시간 (초)
	float OriginalIntensity;
	float OriginalRadius;
	FLinearColor OriginalColor;
};

