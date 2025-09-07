#pragma once
#include "Types.h"
#include "Containers.h"

class UObject
{
public:
	void* operator new(size_t size);

	void operator delete(void* ptr);
public:
	uint32 UUID;
	uint32 InternalIndex;
};

extern TArray<UObject*> GUObjectArray;