#include "Mesh.h"
#include "App.h"

UMesh::UMesh(FMeshData& meshData)
{
	VertexCount = meshData.Vertices.size();
	IndexCount = meshData.Indices.size();
	Stride = sizeof(FVertex);
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = sizeof(FVertex) * VertexCount;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexBufferSRD = { meshData.Vertices.data()};
	
	HRESULT hResult = UApp::Ins->GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, &VertexBuffer);
	if (FAILED(hResult))
	{
		cout << "Create VertexBuffer in MeshCreate Failed" << endl;
		return;
	}
	
	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = sizeof(uint32_t) * IndexCount;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	
	D3D11_SUBRESOURCE_DATA indexBufferSRD = { meshData.Indices.data() };
	hResult = UApp::Ins->GetDevice()->CreateBuffer(&indexBufferDesc, &indexBufferSRD, &IndexBuffer);
	if (FAILED(hResult))
	{
		cout << "Create IndexBuffer in MeshCreate Failed" << endl;
		return;
	}
}
void UMesh::Draw()
{
	UINT offset = 0;
	UApp::Ins->GetContext()->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &offset);
	UApp::Ins->GetContext()->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, offset);
	UApp::Ins->GetContext()->DrawIndexed(IndexCount, 0, 0);
}

void UMesh::Release()
{
	if (VertexBuffer) 
	{
		VertexBuffer->Release();
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
	}
}

UMesh::~UMesh()
{
	Release();
}