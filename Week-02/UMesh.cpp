#include<d3d11.h>
#include "UMesh.h"


UMesh::UMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	:Device(device), DeviceContext(deviceContext)
{
}

bool UMesh::Initialize(const void* vertices, const void* indices, const UINT vertexByteWidth, const UINT indexByteWidth)
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

void UMesh::RenderMesh(UINT vertexStride, UINT indicesCount, DXGI_FORMAT format)
{	
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &vertexStride, &offset);
	DeviceContext->IASetIndexBuffer(IndexBuffer, format, 0);
	DeviceContext->DrawIndexed(indicesCount, 0, 0);
}

void UMesh::RenderMesh(ID3D11Buffer* VertexBuffer, unsigned int NumVertices, ID3D11Buffer* IndexBuffer, unsigned int IndexCount, unsigned int Stride)
{
	unsigned int offset = 0; // 버퍼 오프셋 초기화
	// 정점 버퍼 설정
	DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &offset);

	if (IndexBuffer)
	{
		// 인덱스 버퍼 설정
		DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		// 드로우 콜 (인덱스 버퍼 사용)
		DeviceContext->DrawIndexed(IndexCount, 0, 0); // 인덱스 수와 시작 인덱스 설정
	}
	else
	{
		// 인덱스 버퍼가 없을 때 드로우 콜
		DeviceContext->Draw(NumVertices, 0); // 정점 수와 시작 인덱스 설정
	}
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

bool UMesh::CreateVertexBuffer(ID3D11Buffer* verticesBuffer, const void* vertices, unsigned int byteWidth)
{
	HRESULT result;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = byteWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferSRD = { vertices };

	result = Device->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, &verticesBuffer);
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


bool UMesh::CreateIndexBuffer(ID3D11Buffer* indicesBuffer, const void* indices, unsigned int byteWidth)
{
	HRESULT hr;

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = byteWidth;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferSRD = { indices };

	hr = Device->CreateBuffer(&indexBufferDesc, &indexBufferSRD, &indicesBuffer);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"indexbuffer create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}

void UMesh::SetTopology(bool isLine)
{
	if (isLine)
	{
		DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
	else
	{
		DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}
