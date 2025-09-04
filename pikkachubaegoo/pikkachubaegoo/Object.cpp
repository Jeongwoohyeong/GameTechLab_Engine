#include "Object.h"
#include "MeshRenderer.h"
#include "App.h"
#include "Transform.h"

UObject::~UObject()
{
	if (renderer)
	{
		delete renderer;
		renderer = nullptr;
	}
}

Transform* UObject::GetTransform()
{
	return &transform;
}

void UObject::Reset()
{
	SetVelocity(FVector3());
}

void UObject::Draw()
{
	if (renderer)
	{
		renderer->DrawMesh(GetTransform()->GetTransformMatrix());
	}
}
