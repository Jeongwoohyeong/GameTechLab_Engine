#pragma once
#include "Actor.h"
#include "DecalComponent.h"

class UAABoundingBoxComponent;

/**
 * Decal Actor
 * 현재: Decal 볼륨 박스를 시각화하는 액터
 * 향후: 실제 Decal 투영 기능으로 확장 예정
 */
class ADecalActor : public AActor
{
public:
	DECLARE_CLASS(ADecalActor, AActor)

	ADecalActor();
	virtual void Tick(float DeltaTime) override;

	// UI Category
	//virtual FString GetDisplayCategory() const override { return "Decal"; }

protected:
	~ADecalActor() override;

public:
	virtual bool DeleteComponent(USceneComponent* ComponentToDelete) override;

	UDecalComponent* GetDecalComponent() const { return DecalComponent; }
	void SetDecalComponent(UDecalComponent* InDecalComponent);	

	// PIE 복제
	UObject* Duplicate() override;
	void DuplicateSubObjects() override;

protected:
	// [PIE] Duplicate 복사
	UDecalComponent* DecalComponent = nullptr;
	UAABoundingBoxComponent* CollisionComponent = nullptr;
};