#pragma once
#include "UObject.h"
#include "Math.h"

class USceneComponent : public UObject
{
public:
	FVector RelativeLocation;
	FVector RelativeRotation;
	FVector RelativeScale3D;
};