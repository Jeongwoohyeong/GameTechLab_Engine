#include <d3d11.h>
#include "URenderer.h"
#include "UD3dDevice.h"
#include "UShader.h"
#include "UCamera.h"
#include "UCubeComp.h"
#include "CScene.h"
#include "Cube.h"
#include "Sphere.h"

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

	if (!this->CreateCubeMesh())
	{
		return false;
	}

	if (!this->CreateRasterizerState())
	{
		return false;
	}	

	/*worldGizmo = new WorldGizmo();
	worldGizmo->Initialize(this);*/

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
	}

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
			CubeMesh->VertexBuffer = nullptr;
		}
		delete CubeMesh;
		CubeMesh = nullptr;
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

//bool URenderer::CreateCubeBuffers()
//{
//	if (!this->CreateVertexBuffer(&CubeVertexBuffer))
//	{
//		return false;
//	}
//	/*Primitives->SetVertexBuffer(CubeVertexBuffer);*/
//
//	if (!this->CreateIndexBuffer(&CubeIndexBuffer))
//	{
//		return false;
//	}
//	/*Primitives->SetIndexBuffer(CubeIndexBuffer);*/
//
//	return true;
//}

//bool URenderer::CreateVertexBuffer(ID3D11Buffer** vertexBuffer)
//{
//	HRESULT result;
//
//	UINT byteWidth = Primitives->GetVertexByteWidth();
//	const void* vertices = Primitives->GetVertices();
//	//ID3D11Buffer** vertexBuffer = Primitives->GetVertexBufferAddr();
//
//	D3D11_BUFFER_DESC vertexBufferDesc = {};
//	vertexBufferDesc.ByteWidth = byteWidth;
//	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vertexBufferDesc.CPUAccessFlags = 0;
//
//	D3D11_SUBRESOURCE_DATA vertexBufferSRD = { vertices };
//
//	result = Device->GetDeivce()->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, vertexBuffer);
//	if (FAILED(result))
//	{
//		MessageBox(nullptr, L"vertexbuffer create fail,", L"error", MB_OK);
//		return false;
//	}
//
//	return true;
//}
//bool URenderer::CreateIndexBuffer(ID3D11Buffer** indexBuffer)
//{
//	HRESULT hr;
//
//	UINT bytewidth = Primitives->GetIndexByteWidth();
//	const void* indices = Primitives->GetIndices();
//	//ID3D11Buffer** indexBuffer = Primitives->GetIndexBufferAddr();
//
//	D3D11_BUFFER_DESC indexBufferDesc = {};
//	indexBufferDesc.ByteWidth = bytewidth;
//	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	indexBufferDesc.CPUAccessFlags = 0;
//
//	D3D11_SUBRESOURCE_DATA indexBufferSRD = { indices };
//
//	hr = Device->GetDeivce()->CreateBuffer(&indexBufferDesc, &indexBufferSRD, indexBuffer);
//	if (FAILED(hr))
//	{
//		MessageBox(nullptr, L"indexbuffer create fail,", L"error", MB_OK);
//		return false;
//	}
//
//	return true;
//}