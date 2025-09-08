#pragma once

#include "Types.h"
#include <d3d11.h>
#include "FVertexStruct.h"

struct FMesh
{
	const FVertexSimple* Vertices = nullptr;
	const uint32* Indices = nullptr;
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	uint32 VertexByteWidth = 0;
	uint32 IndexByteWidth = 0;
	uint32 IndexCount = 0;
	uint32 Offset = 0;
	uint32 Stride = 0;
	bool bUseIndexBuffer = false;
};