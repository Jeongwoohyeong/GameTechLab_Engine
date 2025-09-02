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

	virtual FVector3 GetLocation() = 0;
	virtual void SetLocation(const FVector3& newLocation) = 0;

	virtual FVector3 GetVelocity() = 0;
	virtual void SetVelocity(const FVector3& newVelocity) = 0;

	virtual float GetRadius() = 0;
	virtual float GetMass() = 0;

	virtual void Update() = 0;
};
