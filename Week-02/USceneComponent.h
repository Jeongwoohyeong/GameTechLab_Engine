#pragma once
#include "UObject.h"
#include "FTransform.h"

class USceneComponent : public UObject
{
public:
	USceneComponent()
		:Transform(FTransform())
	{};
	virtual ~USceneComponent() {};

	FTransform Transform;
};