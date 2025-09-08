#include <d3d11.h>
#include "URenderer.h"
#include "UD3dDevice.h"
#include "UShader.h"
#include "UCamera.h"
#include "UCubeComp.h"

ID3D11Buffer* URenderer::CubeVertexBuffer = nullptr;
ID3D11Buffer* URenderer::CubeIndexBuffer = nullptr;

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

	Primitives = new UCubeComp();
	if (!this->CreateCubeBuffers())
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
	FMatrix worldMatrix = FMatrix::Identity();
	worldMatrix = worldMatrix * Primitives->GetTransform()->GetTransformMatrix();
	
	Shader->PrepareShader();
	
	Primitives->RenderPrimitive(Device->GetDeviceContext());

	Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(worldMatrix));
	UI.ObjectControlUI(Primitives->GetTransform());

	// worldGizmo->Render(this);

	// TODO: 모든 Primitive 렌더링은 Scene의 렌더링 함수 내에서 수행

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

	if (Primitives)
	{
		Primitives->Release();
		delete Primitives;
		Primitives = nullptr;
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

bool URenderer::CreateCubeBuffers()
{
	if (!this->CreateVertexBuffer(&CubeVertexBuffer))
	{
		return false;
	}
	Primitives->SetVertexBuffer(CubeVertexBuffer);

	if (!this->CreateIndexBuffer(&CubeIndexBuffer))
	{
		return false;
	}
	Primitives->SetIndexBuffer(CubeIndexBuffer);

	return true;
}

bool URenderer::CreateVertexBuffer(ID3D11Buffer** vertexBuffer)
{
	HRESULT result;

	UINT byteWidth = Primitives->GetVertexByteWidth();
	const void* vertices = Primitives->GetVertices();
	//ID3D11Buffer** vertexBuffer = Primitives->GetVertexBufferAddr();

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = byteWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferSRD = { vertices };

	result = Device->GetDeivce()->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, vertexBuffer);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"vertexbuffer create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}
bool URenderer::CreateIndexBuffer(ID3D11Buffer** indexBuffer)
{
	HRESULT hr;

	UINT bytewidth = Primitives->GetIndexByteWidth();
	const void* indices = Primitives->GetIndices();
	//ID3D11Buffer** indexBuffer = Primitives->GetIndexBufferAddr();

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = bytewidth;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexBufferSRD = { indices };

	hr = Device->GetDeivce()->CreateBuffer(&indexBufferDesc, &indexBufferSRD, indexBuffer);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"indexbuffer create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}