#pragma once
#include "Types.h"
#include "Containers.h"

class UObject
{
public:
	uint32 UUID;
	uint32 InternalIndex;
};


TArray<UObject*> GUObjectArray;