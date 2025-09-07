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

private:
	bool CreateVertexBuffer(const void* vertices);
	bool CreateIndexBuffer(const void* indices);

private:
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	FTransform Transform = {};
	UINT VertexByteWidth = 0;
	UINT IndexByteWidth = 0;
	UINT offset = 0;
};