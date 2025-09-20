#include "pch.h"
#include "Mesh/StaticMesh.h"
#include "Render/Renderer/Renderer.h"
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

FStaticMesh* UStaticMesh::GetStaticMeshAsset()
{
	return StaticMeshAsset;
}
