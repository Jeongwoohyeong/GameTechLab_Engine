#pragma once
#include "MovementComponent.h"

class UProjectileMovementComponent : public UMovementComponent
{
public:
	DECLARE_CLASS(UProjectileMovementComponent, UMovementComponent);

	UProjectileMovementComponent();
	~UProjectileMovementComponent() override = default;

	FVector GetLaunchDirection() { return LaunchDirection; }
	void SetLaunchDirection(const FVector& InRotationAngle) { LaunchDirection = InRotationAngle; }

	void Move(float DeltaSeconds) override;
	void TickComponent(float DeltaSeconds) override;

	UObject* Duplicate() override;
	void DuplicateSubObjects() override;

	void Serialize(FObjectData* Data) override;
	void DeSerialize(FObjectData* Data) override;

private:
	FVector LaunchDirection{ 0.0f, 0.0f, 10.0f };
};