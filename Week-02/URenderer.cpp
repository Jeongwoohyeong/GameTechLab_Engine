#include <d3d11.h>
#include "URenderer.h"
#include "UD3dDevice.h"
#include "UShader.h"
#include "UMesh.h"
#include "Sphere.h"

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
	
	Mesh = new UMesh(Device->GetDeivce(), Device->GetDeviceContext());
	if (!Mesh->Initialize(&sphere_vertices, sizeof(FVertexSimple), sizeof(sphere_vertices) / sizeof(FVertexSimple)))
	{		
		return false;
	}

	if (!this->CreateRasterizerState())
	{
		return false;
	}

	UI.Initialize(hWnd, Device->GetDeivce(), Device->GetDeviceContext(), Mesh->GetTransform());

	return true;
}

void URenderer::Render()
{
	Device->BeginScene();
	Device->SetRSState(RasterizerState);
	Shader->PrepareShader();
	Mesh->PrepareMesh();

	Shader->UpdateContant(Mesh->GetTransform()->GetLocation(), 1.0f);

	UI.RenderUI();

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

	if (Mesh)
	{
		Mesh->Release();
		delete Mesh;
		Mesh = nullptr;
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
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	result = Device->GetDeivce()->CreateRasterizerState(&rasterizerDesc, &RasterizerState);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"rasterizerstate create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}

void URenderer::MVP()
{
	FMatrix ModelMatrix = Mesh->GetTransform()->GetTransformMatrix() * MVPMatrix;
}
