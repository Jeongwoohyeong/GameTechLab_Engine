#include "UObject.h"
#include <cstdlib>
#include <new>

TArray<UObject*> GUObjectArray;

UObject::UObject()
{

}

void* UObject::operator new(size_t size)
{
	void* ptr = malloc(size);
	if(ptr==nullptr)
	{
		throw::std::bad_alloc();
	}
	return ptr;
}

void UObject::operator delete(void* ptr)
{
	free(ptr);
}