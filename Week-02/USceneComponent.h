#pragma once
#include "UObject.h"
#include "FTransform.h"

class USceneComponent : public UObject
{
public:
	USceneComponent() {};
	virtual ~USceneComponent() {};

	FVector RelativeLocation;
	FVector RelativeRotation;
	FVector RelativeScale3D;
};