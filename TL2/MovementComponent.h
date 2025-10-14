#pragma once
#include "ActorComponent.h"

class UMovementComponent : public UActorComponent
{
public:
	DECLARE_CLASS(UMovementComponent, UActorComponent);

	UMovementComponent() = default;
	~UMovementComponent() override = default;

	virtual void Move(float DeltaSeconds) = 0;
};