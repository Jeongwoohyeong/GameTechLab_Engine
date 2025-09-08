#include <d3d11.h>
#include "URenderer.h"
#include "UD3dDevice.h"
#include "UShader.h"
#include "UCamera.h"
#include "UCubeComp.h"
#include "CScene.h"
#include "Cube.h"
#include "Sphere.h"
#include "WorldGizmo.h"

//ID3D11Buffer* URenderer::CubeVertexBuffer = nullptr;
//ID3D11Buffer* URenderer::CubeIndexBuffer = nullptr;

FMesh* URenderer::CubeMesh = nullptr;
FMesh* URenderer::SphereMesh = nullptr;

URenderer::URenderer()
{	
	
}

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

	/*if (!this->CreateCubeBuffers())
	{
		return false;
	}*/
	TArray<UPrimitiveComponent*>& Primitives = CScene::GetInstance().GetPrimitives();
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		Primitive->Initialize(this);
	}

	if (!this->CreateCubeMesh())
	{
		return false;
	}

	if (!this->CreateRasterizerState())
	{
		return false;
	}	

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
	
	// 씬의 프리미티브 렌더링
	TArray<UPrimitiveComponent*>& Primitives = CScene::GetInstance().GetPrimitives();
	for (UPrimitiveComponent* Primitive : Primitives)
	{
		RenderPrimitive(Primitive);
		Primitive->Render(this);
	}
	worldGizmo->Render(this);
	// 선택된 프리미티브의 컨트롤 UI 표시
	UI.ObjectControlUI(Primitives[0]->GetTransform());

	// worldGizmo->Render(this);

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

	ReleaseAllMesh();
	if(worldGizmo)
	{
		worldGizmo->Release();
		delete worldGizmo;
		worldGizmo = nullptr;
	}
	if (Shader)
	{
		Shader->Release();
		delete Shader;
		Shader = nullptr;
	}

	if (Device)
	{
		Device->Release();
		delete Device;
		Device = nullptr;
	}

}

bool URenderer::CreateRasterizerState()
{
	HRESULT result;
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	result = Device->GetDeivce()->CreateRasterizerState(&rasterizerDesc, &RasterizerState);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"rasterizerstate create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}

bool URenderer::CreateCubeMesh()
{
	if (CubeMesh == nullptr)
	{
		CubeMesh = new FMesh();
		CubeMesh->Vertices = GCubeVertices;
		CubeMesh->Indices = GCubeIndices;
		CubeMesh->VertexByteWidth = sizeof(GCubeVertices);
		CubeMesh->IndexByteWidth = sizeof(GCubeIndices);
		CubeMesh->IndexCount = CubeMesh->IndexByteWidth / sizeof(uint32);
		CubeMesh->Offset = 0;
		CubeMesh->Stride = sizeof(FVertexSimple);
		CubeMesh->bUseIndexBuffer = true;

		if (!this->CreateVertexBuffer(CubeMesh))
		{
			return false;
		}

		if (!this->CreateIndexBuffer(CubeMesh))
		{
			return false;
		}

		return true;
	}

	return false;
}

void URenderer::Resize(UINT width, UINT height)
{
	if (Device == nullptr)
	{
		return;
	}

	Device->Resize(width, height);
}

bool URenderer::CreateVertexBuffer(FMesh* Mesh)
{
	HRESULT result;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = Mesh->VertexByteWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferSRD = { Mesh->Vertices };

	result = Device->GetDeivce()->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, &Mesh->VertexBuffer);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"vertexbuffer create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}

bool URenderer::CreateIndexBuffer(FMesh* Mesh)
{
	HRESULT hr;

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = Mesh->IndexByteWidth;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferSRD = { Mesh->Indices };

	hr = Device->GetDeivce()->CreateBuffer(&indexBufferDesc, &indexBufferSRD, &Mesh->IndexBuffer);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"indexbuffer create fail,", L"error", MB_OK);
		return false;
	}
	return true;
}

bool URenderer::RenderPrimitive(UPrimitiveComponent* Primitive)
{
	if (!Primitive)
	{
		return false;
	}

	FMatrix World = FMatrix::Identity();
	World = World * Primitive->GetTransform()->GetTransformMatrix();
	Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(World));
	
	ID3D11DeviceContext* DeviceContext = Device->GetDeviceContext();

	// 드로우 콜
	FMesh* Mesh = Primitive->GetMesh();
	DeviceContext->IASetVertexBuffers(0, 1, &Mesh->VertexBuffer, &Mesh->Stride, &Mesh->Offset);
	DeviceContext->IASetIndexBuffer(Mesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DeviceContext->DrawIndexed(Mesh->IndexCount, 0, 0);

	return true;
}

#pragma region Gizmo 사용, 추후 수정

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
#pragma endregion
void URenderer::ReleaseAllMesh()
{
	if (CubeMesh)
	{
		if (CubeMesh->IndexBuffer)
		{
			CubeMesh->IndexBuffer->Release();
			CubeMesh->IndexBuffer = nullptr;
		}
		if (CubeMesh->VertexBuffer)
		{
			CubeMesh->VertexBuffer->Release();
		}
			CubeMesh->VertexBuffer = nullptr;
		CubeMesh = nullptr;
		delete CubeMesh;
	}
	if (SphereMesh)
	{
		if (SphereMesh->IndexBuffer)
		{
			SphereMesh->IndexBuffer->Release();
			SphereMesh->IndexBuffer = nullptr;
		}
		if (SphereMesh->VertexBuffer)
		{
			SphereMesh->VertexBuffer->Release();
			SphereMesh->VertexBuffer = nullptr;
		}
		delete SphereMesh;
		SphereMesh = nullptr;
	}
}