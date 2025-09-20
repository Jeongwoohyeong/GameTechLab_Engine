#include "pch.h"
#include "Mesh/StaticMesh.h"
#include "Render/Renderer/Renderer.h"
#include "Math/AABB.h"
IMPLEMENT_CLASS(UStaticMesh, UObject)

UStaticMesh::UStaticMesh()
{
}

UStaticMesh::UStaticMesh(FStaticMesh* InStaticMeshAsset)
{
	SetStaticMeshAsset(InStaticMeshAsset);
}

UStaticMesh::~UStaticMesh()
{
	if (VertexBuffer)
	{
		VertexBuffer->Release();
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
	}
}

void UStaticMesh::SetStaticMeshAsset(FStaticMesh* InStaticMeshAsset)
{
	URenderer& Renderer = URenderer::GetInstance();
	if (!InStaticMeshAsset)
	{
		UE_LOG("MeshAsset이 유효하지 않아 SetStaticMeshAsset이 반환되었습니다");
		return;
	}
	if (VertexBuffer)
	{
		VertexBuffer->Release();
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
	}

	StaticMeshAsset = InStaticMeshAsset;
	VertexBuffer = Renderer.CreateVertexBuffer(InStaticMeshAsset->Vertices);
	IndexBuffer = Renderer.CreateIndexBuffer(InStaticMeshAsset->Indices);
	IndexNum = InStaticMeshAsset->IndexNum;
}

FAABB UStaticMesh::CalculateAABB() const
{
	if (!StaticMeshAsset)
		return FAABB();
	const TArray<FNormalVertex>& Vertices = StaticMeshAsset->Vertices;

	FAABB AABB = FAABB();
	for (int Index = 0; Index < Vertices.Num();Index++)
	{
		AABB.AddPoint(Vertices[Index].Position);
	}

	return AABB;
}

FStaticMesh* UStaticMesh::GetStaticMeshAsset()
{
	return StaticMeshAsset;
}
