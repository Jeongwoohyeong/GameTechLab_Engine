#include "Object.h"
#include "UMeshRenderer.h"
#include "App.h"

UObject::~UObject()
{
	if (renderer)
	{
		delete renderer;
		renderer = nullptr;
	}
}

FVector3 UObject::GetLocation()
{
	return location;
}

void UObject::SetLocation(const FVector3& newLocation)
{
	location = newLocation;
}

void UObject::Draw()
{
	if (renderer)
	{
		renderer->DrawMesh(GetLocation());
	}
}
