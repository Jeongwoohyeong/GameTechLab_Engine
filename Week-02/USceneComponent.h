#pragma once
#include "UObject.h"
#include "FTransform.h"
#include "RTTIMacros.h"


class USceneComponent : public UObject
{
	RTTI_DECLARE()
	
public:
	USceneComponent()
		:Transform(FTransform())
	{};
	virtual ~USceneComponent() {};

	FTransform Transform;
};