#pragma once
#include "Types.h"
#include "Containers.h"
#include "RTTIMacros.h"

class UClass;

class UObject
{
	RTTI_DECLARE()

public:
	UObject();
	virtual ~UObject();

	bool IsA(const UClass*) const;

	void* operator new(size_t size);
	void operator delete(void* ptr);
public:
	uint32 UUID;
	uint32 InternalIndex;
	static uint64 TotalAllocationBytes;
	static uint64 TotalAllocationCount;
	static TMap<void*, size_t> AllocatedBytesMap;
};

extern TArray<UObject*> GUObjectArray;