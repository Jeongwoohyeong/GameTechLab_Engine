#pragma once
#include "Types.h"
#include "Containers.h"

class UObject
{
public:
	UObject();
	virtual ~UObject() {};

	void* operator new(size_t size);
	void operator delete(void* ptr);
public:
	uint32 UUID;
	uint32 InternalIndex;
	uint32 TotalAllocationBytes;
	uint32 TatalAllocationCount;
};

extern TArray<UObject*> GUObjectArray;