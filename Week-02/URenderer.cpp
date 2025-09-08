#include "URenderer.h"
#include "UD3dDevice.h"
#include "UShader.h"
#include "UCamera.h"

#include "ShapeData.h"
#include "LocalGizmo.h"
#include "WorldGizmo.h"

URenderer::URenderer() {}

bool URenderer::Initialize(HWND hWnd)
{
	Device = new UD3dDevice();
	if (!Device->Initialize(hWnd))
	{
		return false;
	}

	Shader = new UShader();
	if (!Shader->Initialize(Device->GetDeivce(), Device->GetDeviceContext()))
	{
		return false;
	}
	
	// TODO 
	//Shape = new ShapeData();
	//Shape->Initialize(*Mesh);
	
	// TODO 
	localCube = new LocalGizmo();
	localCube->Initialize(this);

	worldGizmo = new WorldGizmo();
	worldGizmo->Initialize(this);

	UCamera::GetInstance().Init();
		
	UI.Initialize(hWnd, Device->GetDeivce(), Device->GetDeviceContext());

	return true;
}

void URenderer::Render()
{
	Device->BeginScene();
	Device->SetRSState(RasterizerState);
	
	Shader->PrepareShader();

	// Mesh->PrepareMesh(sizeof(FVertexSimple), sizeof(GCubeIndices) / sizeof(UINT), DXGI_FORMAT_R32_UINT);

	FMatrix worldMatrix = FMatrix::Identity();
	worldMatrix = worldMatrix * localCube->Transform.GetTransformMatrix();
	
	Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(worldMatrix));
	localCube->Render(this);

	worldGizmo->Render(this);

	// TODO: 모든 Primitive 렌더링은 Scene의 렌더링 함수 내에서 수행

	UI.ObjectControlUI(&localCube->Transform);

	Device->EndScene();
}

void URenderer::Release()
{
	UI.ReleaseUI();

	if (RasterizerState)
	{
		RasterizerState->Release();
		RasterizerState = nullptr;
	}

	if (Shader)
	{
		Shader->Release();
		delete Shader;
		Shader = nullptr;
	}

	if (worldGizmo)
	{
		worldGizmo->Release();
		delete worldGizmo;
		worldGizmo = nullptr;
	}

	if (Device)
	{
		Device->Release();
		delete Device;
		Device = nullptr;
	}	
}

void URenderer::UpdateConstant(const FMatrix& mvp)
{
	Shader->UpdateConstant(mvp);
}

void URenderer::UpdateConstant(const FMatrix& mvp, const FVector& vec)
{
	Shader->UpdateConstant(mvp, vec);
}

void URenderer::RenderMesh(ID3D11Buffer* VertexBuffer, unsigned int NumVertices, ID3D11Buffer* IndexBuffer, unsigned int IndexCount, unsigned int Stride)
{
	unsigned int offset = 0; // 버퍼 오프셋 초기화
	// 정점 버퍼 설정
	Device->DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &offset);

	if (IndexBuffer)
	{
		// 인덱스 버퍼 설정
		Device->DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		// 드로우 콜 (인덱스 버퍼 사용)
		Device->DeviceContext->DrawIndexed(IndexCount, 0, 0); // 인덱스 수와 시작 인덱스 설정
	}
	else
	{
		// 인덱스 버퍼가 없을 때 드로우 콜
		Device->DeviceContext->Draw(NumVertices, 0); // 정점 수와 시작 인덱스 설정
	}
}

bool URenderer::CreateVertexBuffer(ID3D11Buffer** verticesBuffer, const void* vertices, unsigned int byteWidth)
{
	HRESULT result;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = byteWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferSRD = { vertices };

	result = Device->Device->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, verticesBuffer);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"vertexbuffer create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}

bool URenderer::CreateIndexBuffer(ID3D11Buffer** indicesBuffer, const void* indices, unsigned int byteWidth)
{
	HRESULT hr;

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = byteWidth;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferSRD = { indices };

	hr = Device->Device->CreateBuffer(&indexBufferDesc, &indexBufferSRD, indicesBuffer);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"indexbuffer create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}


void URenderer::SetTopology(bool isLine)
{
	if (isLine)
	{
		Device->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
	else
	{
		Device->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}
