#pragma once
#include "UPrimitiveComponent.h"
#include "USphereComp.h"
#include "UCubeComp.h"
#include "UTriangleComp.h"

class FObjectFactory
{
public:
	FObjectFactory() {};
	~FObjectFactory() {};

	static UPrimitiveComponent* ConstructObject(UClass*);
};