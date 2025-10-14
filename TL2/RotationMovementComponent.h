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

	UObject* Duplicate() override;
	void DuplicateSubObjects() override;

	void Serialize(FObjectData* Data) override;
	void DeSerialize(FObjectData* Data) override;

private:
	FVector RotationAngle{0.0f, 0.0f, 10.0f};
};