#include <cstdlib>
#include <new>
#include "UObject.h"
#include "UEngineStatics.h"

TArray<UObject*> GUObjectArray;
uint64 UObject::TotalAllocationBytes = 0;
uint64 UObject::TotalAllocationCount = 0;
TMap<void*, size_t> UObject::AllocatedBytesMap = {};

UObject::UObject()
	: UUID(UEngineStatics::GenUUID()), InternalIndex(0)
{
	GUObjectArray.push_back(this);
}

void* UObject::operator new(size_t size)
{
	void* ptr = malloc(size);
	if (ptr == nullptr)
	{
		throw::std::bad_alloc();
	}
	
	TotalAllocationBytes = TotalAllocationBytes + size;
	TotalAllocationCount++;

	AllocatedBytesMap[ptr] = size;

	return ptr;
}

void UObject::operator delete(void* ptr)
{
	if (ptr == nullptr)
	{
		return;
	}

	TotalAllocationBytes -= AllocatedBytesMap[ptr];
	TotalAllocationCount--;

	AllocatedBytesMap.erase(ptr);
	
	free(ptr);
}