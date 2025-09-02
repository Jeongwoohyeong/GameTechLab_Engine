#include "Object.h"

UObject::~UObject()
{
	if (renderer)
	{
		delete renderer;
		renderer = nullptr;
	}
}