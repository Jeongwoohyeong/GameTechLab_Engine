#include<d3d11.h>
#include "UMesh.h"


UMesh::UMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	:Device(device), DeviceContext(deviceContext)
{
}

bool UMesh::Initialize(const void* vertices, const void* indices, const UINT vertexByteWidth, const UINT indexByteWidth, const UINT vertexCount)
{
	VertexByteWidth = vertexByteWidth;
	IndexByteWidth = indexByteWidth;

	if (!this->CreateVertexBuffer(vertices))
	{
		return false;
	}

	if (!this->CreateIndexBuffer(indices))
	{
		return false;
	}

	Transform = FTransform();

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

void UMesh::PrepareMesh(UINT vertexStride, UINT indicesCount, DXGI_FORMAT format)
{	
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &vertexStride, &offset);
	DeviceContext->IASetIndexBuffer(IndexBuffer, format, 0);
	DeviceContext->DrawIndexed(indicesCount, 0, 0);
}

bool UMesh::CreateVertexBuffer(const void* vertices)
{
	HRESULT result;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = VertexByteWidth;
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

bool UMesh::CreateIndexBuffer(const void* indices)
{
	HRESULT hr;

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = IndexByteWidth;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferSRD = {};
	indexBufferSRD.pSysMem = indices;

	hr = Device->CreateBuffer(&indexBufferDesc, &indexBufferSRD, &IndexBuffer);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"indexbuffer create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}
