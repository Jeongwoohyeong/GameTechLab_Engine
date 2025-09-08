#include "UPrimitiveComponent.h"
#include "d3d11.h"
#include "LocalGizmo.h"

UPrimitiveComponent::UPrimitiveComponent()
{}

void UPrimitiveComponent::Initialize(URenderer* renderer)
{
	Gizmo.Initialize(renderer, &Transform);
}

void UPrimitiveComponent::Render(URenderer* renderer)
{
	Gizmo.Render(renderer);
}

void UPrimitiveComponent::Release()
{
	Gizmo.Release();
}

void UPrimitiveComponent::CreateAABB()
{
	FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	FMesh* Mesh = GetMesh();

	const uint32 VertexCount = Mesh->VertexByteWidth / Mesh->Stride;
	for (uint32 Offset = 0; Offset < Mesh->VertexByteWidth; Offset += Mesh->Stride)
	{
		const char* Ptr = reinterpret_cast<const char*>(Mesh->Vertices) + Offset;
		float X = *reinterpret_cast<const float*>(Ptr);
		float Y = *reinterpret_cast<const float*>(Ptr + sizeof(float));
		float Z = *reinterpret_cast<const float*>(Ptr + 2 * sizeof(float));
		
		if (Min.X > X) Min.X = X;
		if (Min.Y > Y) Min.Y = Y;
		if (Min.Z > Z) Min.Z = Z;
		if (Max.X < X) Max.X = X;
		if (Max.Y < Y) Max.Y = Y;
		if (Max.Z < Z) Max.Z = Z;
	}

	AABB.Min = Min;
	AABB.Max = Max;
}
