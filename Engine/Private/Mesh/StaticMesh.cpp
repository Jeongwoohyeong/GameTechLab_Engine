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

FStaticMesh* UStaticMesh::GetStaticMeshAsset()
{
	return StaticMeshAsset;
}

FAABB UStaticMesh::GetLocalAABB()
{
	//Mesh에 로컬 바운드가 없으면 계산해서 리턴
	if (!AABB.IsValid())
	{
		CalculateLocalAABB();
	}
	return AABB;
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



void UStaticMesh::CalculateLocalAABB()
{
	if (!StaticMeshAsset)
		return;


	const TArray<FNormalVertex>& Vertices = StaticMeshAsset->Vertices;

	switch (PrimitiveType)
	{
	case EPrimitiveType::None:
	case EPrimitiveType::Triangle:
	case EPrimitiveType::Square:
		for (int Index = 0; Index < Vertices.Num();Index++)
		{
			AABB.AddPoint(Vertices[Index].Position);
		}break;
	case EPrimitiveType::Sphere:
	{
		float Radius = Vertices[0].Position.Length();
		AABB.Min = FVector(-Radius, -Radius, -Radius);
		AABB.Max = FVector(Radius, Radius, Radius);
	}
	}
	

}

