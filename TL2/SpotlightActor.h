#pragma once
#include "Actor.h"
#include "BillboardComponent.h"

class USpotlightComponent;

/**
 * 레벨에 배치되어 스포트라이트 기능을 제공하는 액터입니다.
 * 실제 모든 로직은 SpotlightComponent에 의해 처리됩니다.
 */
class ASpotlightActor : public AActor
{
	
public:
	DECLARE_CLASS(ASpotlightActor, AActor);
	ASpotlightActor();

protected:
	~ASpotlightActor() override;

public: 
	virtual bool DeleteComponent(USceneComponent* ComponentToDelete) override;

	USpotlightComponent* GetSpotlightCompoonent() const { return SpotlightComponent; }
	void SetSpotlightComponent(USpotlightComponent* InSpotlightComponent);

	// PIE 복제
	UObject* Duplicate() override;
	void DuplicateSubObjects() override;

protected:
	// [PIE] Duplicate 복사
	USpotlightComponent* SpotlightComponent = nullptr;
	UBillboardComponent* BillboardComponent = nullptr;
};
