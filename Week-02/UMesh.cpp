#include<d3d11.h>
#include "UMesh.h"


UMesh::UMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	:Device(device), DeviceContext(deviceContext)
{
}

bool UMesh::Initialize(const void* vertices, const UINT stride, const UINT vertexCount)
{
	Stride = stride;
	VertexCount = vertexCount;

	if (!this->CreateVertexBuffer(vertices))
	{
		return false;
	}

	return true;
}

void UMesh::Release()
{
	if (IndexBuffer)
	{
		IndexBuffer->Release();
	}

	if (VertexBuffer)
	{
		VertexBuffer->Release();
	}
}

void UMesh::PrepareMesh()
{	
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &offset);
	DeviceContext->Draw(VertexCount, 0);
}

bool UMesh::CreateVertexBuffer(const void* vertices)
{
	HRESULT result;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = Stride * VertexCount;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferSRD = { };
	vertexBufferSRD.pSysMem = vertices;

	result = Device->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, &VertexBuffer);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"vertexbuffer create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}

bool UMesh::CreateIndexBuffer()
{
	return true;
}
