#pragma once
#include "Vector.h"

enum FObjectType
{
	Player,
	Ball,
	Wall,
};

class UObject
{
public:
	virtual ~UObject() = default;

	virtual FObjectType GetType() = 0;

	virtual FVector GetLocation() = 0;
	virtual void SetLocation(const FVector& newLocation) = 0;

	virtual FVector GetVelocity() = 0;
	virtual void SetVelocity(const FVector& newVelocity) = 0;

	virtual float GetRadius() = 0;
	virtual float GetMass() = 0;

	virtual void Update() = 0;
};
