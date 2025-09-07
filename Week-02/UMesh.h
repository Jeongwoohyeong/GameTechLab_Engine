#pragma once
#include "FTransform.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

class UMesh
{
public:
	UMesh(ID3D11Device*, ID3D11DeviceContext*);
	~UMesh() {};

	bool Initialize(const void* vertices, const void* indices,
		const UINT vertexByteWidth, const UINT indexByteWidth);
	void Release();
	void RenderMesh(UINT vertexStride, UINT indicesCount, DXGI_FORMAT format);
	FTransform* GetTransform() { return &Transform; }

	// 임시로 추가, gizmo에서 사용
	void RenderMesh(ID3D11Buffer* VertexBuffer, UINT NumVertices, ID3D11Buffer* IndexBuffer, UINT IndexCount, UINT Stride);

	bool CreateVertexBuffer(ID3D11Buffer* verticesBuffer, const void* vertices, UINT byteWidth);
	bool CreateIndexBuffer(ID3D11Buffer* indicesBuffer, const void* indices, UINT byteWidth);
	void SetTopology(bool isLine);
private:
	bool CreateVertexBuffer(const void* vertices);
	bool CreateIndexBuffer(const void* indices);

private:
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;

	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;

	FTransform Transform = {};
	UINT VertexByteWidth = 0;
	UINT IndexByteWidth = 0;
	UINT offset = 0;
};