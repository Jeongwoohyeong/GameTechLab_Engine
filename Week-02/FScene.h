#pragma once

#include "Types.h"
#include "Containers.h"
#include "UPrimitiveComponent.h"

class FScene
{
public:
	int32 Version;
	uint32 NextUUID;
	TArray<UPrimitiveComponent*> Primitives; // Key: UUID, Value: UPrimitiveComponent*
};
