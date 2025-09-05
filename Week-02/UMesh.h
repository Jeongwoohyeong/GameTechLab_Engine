#pragma once

class ID3D11Device;
class ID3D11DeviceContext;
class ID3D11Buffer;

//******************************************/
//
//	IndexBuffer 미완성
//
//
//******************************************/

class UMesh
{
public:
	UMesh(ID3D11Device*, ID3D11DeviceContext*);
	~UMesh() {};

	bool Initialize(const void* vertices, const UINT stride, const UINT vertexCount);
	void Release();
	void PrepareMesh();


private:
	bool CreateVertexBuffer(const void* vertices);
	bool CreateIndexBuffer();

private:
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	UINT Stride = 0;
	UINT offset = 0;
	UINT VertexCount = 0;
};