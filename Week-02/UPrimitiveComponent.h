#pragma once
#include "USceneComponent.h"
#include "Types.h"
#include "Math.h"
#include "UPrimitiveTypes.h"
#include "FTransform.h"

struct ID3D11Buffer;
struct ID3D11DeviceContext;

class UPrimitiveComponent :public USceneComponent
{
public:
	UPrimitiveComponent();
	virtual ~UPrimitiveComponent() {};

	void Release();

	inline uint32 GetVertexByteWidth() const { return VertexByteWidth; }
	inline uint32 GetIndexByteWidth() const { return IndexByteWidth; }
	inline uint32 GetVertexStride() const { return VertexStide; }
	inline uint32 GetIndexCount() const { return IndexCount; }
	inline const void* GetVertices() const { return Vertices; }
	inline const void* GetIndices() const { return Indices; }
	inline ID3D11Buffer** GetVertexBufferAddr() { return &VertexBuffer; }
	inline ID3D11Buffer** GetIndexBufferAddr() { return &IndexBuffer; }
	inline FTransform* GetTransform() { return &Transform; }

	inline void SetVertexBuffer(ID3D11Buffer* vertexBuffer) { VertexBuffer = vertexBuffer; }
	inline void SetIndexBuffer(ID3D11Buffer* indexBuffer) { IndexBuffer = indexBuffer; }

	virtual EPrimitiveType GetPrimitiveType() = 0;
	virtual void RenderPrimitive(ID3D11DeviceContext*);

protected:
	const void* Vertices = nullptr;
	const void* Indices = nullptr;
	uint32 VertexByteWidth = 0;
	uint32 IndexByteWidth = 0;
	uint32 VertexStide = 0;
	uint32 IndexCount = 0;
	uint32 Offset = 0;
	uint32 Stride = 0;
	FTransform Transform;

	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
};