#pragma once

#include "D3DUtil.h"
#include "Vertex.h"
#include "MeshData.h"

class UMesh
{
public:
	UMesh(FMeshData& meshData);
	void Release();
	void Draw();
	~UMesh();
public:
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;

	int VertexCount = 0;
	int IndexCount = 0;
	unsigned int Stride = 0;

};