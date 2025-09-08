#include <cstdlib>
#include <new>
#include <cassert>
#include "UObject.h"
#include "UEngineStatics.h"
#include "UUIManager.h"

TArray<UObject*> GUObjectArray;
uint64 UObject::TotalAllocationBytes = 0;
uint64 UObject::TotalAllocationCount = 0;
TMap<void*, size_t> UObject::AllocatedBytesMap = {};

UObject::UObject()
	: UUID(-1), InternalIndex(0)
{
	GUObjectArray.push_back(this);
	InternalIndex = static_cast<uint32>(GUObjectArray.size() - 1);
}

UObject::~UObject()
{
	bool bErased = false;

	// GUObjectArray에서 자신을 제거
	if (InternalIndex < GUObjectArray.size() && GUObjectArray[InternalIndex] == this)
	{
		GUObjectArray.erase(GUObjectArray.begin() + InternalIndex);
		bErased = true;
	}
	else
	{
		UE_LOG("Warning: UObject InternalIndex is invalid. Searching for object to remove.");

		// 만약 InternalIndex가 틀렸다면, 배열을 순회하며 자신을 찾아 제거
		for (size_t i = 0; i < GUObjectArray.size(); ++i)
		{
			if (GUObjectArray[i] == this)
			{
				GUObjectArray.erase(GUObjectArray.begin() + i);
				bErased = true;
				break;
			}
		}
	}

	if (!bErased)
	{
		UE_LOG("Error: UObject not erased from GUObjectArray.");
		assert(false); // 디버그 모드에서 중단
	}
	else
	{
		// 다른 객체들의 InternalIndex를 갱신
		for (size_t i = 0; i < GUObjectArray.size(); ++i)
		{
			GUObjectArray[i]->InternalIndex = static_cast<uint32>(i);
		}
	}
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