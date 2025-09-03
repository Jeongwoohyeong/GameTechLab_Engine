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

void UObject::Draw()
{
	if (renderer)
	{
		renderer->DrawMesh(GetLocation());
	}
}
