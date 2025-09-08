#pragma once
#include "USceneComponent.h"
#include "Types.h"
#include "Math.h"
#include "UPrimitiveTypes.h"
#include "IntersectionTest.h"

struct ID3D11Buffer;
struct ID3D11DeviceContext;

class UPrimitiveComponent :public USceneComponent
{
public:
	UPrimitiveComponent() {};
	virtual ~UPrimitiveComponent() {};

	virtual EPrimitiveType GetPrimitiveType() = 0;

	virtual void RenderPrimitive(ID3D11DeviceContext*);

	virtual void CreateAABB();

protected:
	const void* Vertices = nullptr;
	const void* Indices = nullptr;
	uint32 VertexByteWidth = 0;
	uint32 IndexByteWidth = 0;
	uint32 VertexStride = 0;
	uint32 IndexCount = 0;
	FAABB AABB;

	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
};