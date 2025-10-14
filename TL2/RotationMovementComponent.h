#pragma once
#include "MovementComponent.h"

class URotationMovementComponent : public UMovementComponent
{
public:
	DECLARE_CLASS(URotationMovementComponent, UMovementComponent);

	URotationMovementComponent();
	~URotationMovementComponent() override = default;

	FVector GetRotationAngle() { return RotationAngle; }
	void SetRotationAngle(const FVector& InRotationAngle) { RotationAngle = InRotationAngle; }

	void Move(float DeltaSeconds) override;
	void TickComponent(float DeltaSeconds) override;

	UObject* Duplicate()
	{
		URotationMovementComponent* DuplicatedComponent = NewObject<URotationMovementComponent>(*this);

		// 복제된 컴포넌트는 초기화되지 않은 상태로 시작
		DuplicatedComponent->bIsInitialized = false;
		DuplicatedComponent->RotationAngle = RotationAngle;

		DuplicatedComponent->DuplicateSubObjects();
		return DuplicatedComponent;
	}

	void DuplicateSubObjects()
	{
		Super_t::DuplicateSubObjects();
	}

private:
	FVector RotationAngle{0.0f, 0.0f, 10.0f};
};