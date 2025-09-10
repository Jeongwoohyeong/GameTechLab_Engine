#include "UPrimitiveComponent.h"
#include "d3d11.h"
#include "LocalGizmo.h"

RTTI_IMPL(UPrimitiveComponent, USceneComponent)

UPrimitiveComponent::UPrimitiveComponent()
{
}

void UPrimitiveComponent::Release()
{
	
}

void UPrimitiveComponent::CreateAABB()
{
	FVector Min(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	FMesh* Mesh = GetMesh();

	if (Mesh == nullptr || Mesh->Vertices == nullptr || Mesh->VertexByteWidth == 0 || Mesh->Stride == 0)
	{
		UE_LOG("Warning: Cannot create AABB because Mesh or its vertex data is invalid.");
		AABB.Min = Min;
		AABB.Max = Max;
		return;
	}

	const uint32 VertexCount = Mesh->VertexByteWidth / Mesh->Stride;
	for (uint32 Offset = 0; Offset < Mesh->VertexByteWidth; Offset += Mesh->Stride)
	{
		//Mesh->VertexBuffer[i][0];
		//Mesh->VertexBuffer[i][1];
		//Mesh->VertexBuffer[i][2];

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

FAABB UPrimitiveComponent::GetAABB()
{
	if (!bIsAABBCreated)
	{
		CreateAABB();
		bIsAABBCreated = true;
	}
	return AABB;
}

//void UPrimitiveComponent::SwapGizmo()
//{
//	constexpr int gizmoCount = 3;
//	int flag = (GizmoSwitch + 1) % gizmoCount;
//
//	switch (flag)
//	{
//	case 1:
//		if (LocationGizmo.ParentTransform == nullptr)
//			LocationGizmo.Initialize(&Transform);
//		GizmoTransform = LocationGizmo.GetGizmoTransform();
//		break;
//	case 2:
//		if (ObjectScaleGizmo.ParentTransform == nullptr)
//			ObjectScaleGizmo.Initialize(&Transform);
//		GizmoTransform = ObjectScaleGizmo.GetGizmoTransform();
//		break;
//	case 3:
//		break;
//	default:
//		break;
//	}
//	
//}
