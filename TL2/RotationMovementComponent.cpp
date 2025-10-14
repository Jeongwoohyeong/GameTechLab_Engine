#include "pch.h"
#include "RotationMovementComponent.h"

URotationMovementComponent::URotationMovementComponent() { SetTickEnabled(true); }

void URotationMovementComponent::Move(float DeltaSeconds)
{
	const FQuat Quat = FQuat::MakeFromEuler(RotationAngle * DeltaSeconds);
	GetOwner()->AddActorLocalRotation(Quat);
}

void URotationMovementComponent::TickComponent(float DeltaSeconds)
{
	Move(DeltaSeconds);
}