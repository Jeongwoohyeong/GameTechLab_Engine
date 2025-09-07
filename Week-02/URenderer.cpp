#include "URenderer.h"
#include "UD3dDevice.h"
#include "UShader.h"
#include "UCamera.h"
#include "UMesh.h"
#include "Cube.h"

#include "ShapeData.h"
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
	
	Mesh = new UMesh(Device->GetDeivce(), Device->GetDeviceContext());
	if (!Mesh->Initialize(&GCubeVertices, &GCubeIndices,
		sizeof(GCubeVertices), 
		sizeof(GCubeIndices), 
		sizeof(GCubeVertices) / sizeof(FVertexSimple)))
	{		
		return false;
	}

	//Shape = new ShapeData();
	//Shape->Initialize(*Mesh);
	
	worldGizmo = new WorldGizmo();
	worldGizmo->Initialize(*Mesh);
	
	if (!this->CreateRasterizerState())
	{
		return false;
	}

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

	/*FMatrix worldMatrix = FMatrix::Identity();
	worldMatrix = worldMatrix * Mesh->GetTransform()->GetTransformMatrix();
	Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(worldMatrix));*/

	// UI.ObjectControlUI(Mesh->GetTransform());

	worldGizmo->Render(this);

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
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	result = Device->GetDeivce()->CreateRasterizerState(&rasterizerDesc, &RasterizerState);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"rasterizerstate create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}

void URenderer::SetTopologyR(bool isLine)
{
	Mesh->SetTopology(isLine);
}

void URenderer::UpdateConstantR(const FMatrix& mvp)
{
	Shader->UpdateConstant(mvp);
}

void URenderer::RenderMesh(ID3D11Buffer* VertexBuffer, unsigned int NumVertices, ID3D11Buffer* IndexBuffer, unsigned int IndexCount, unsigned int Stride)
{
	unsigned int offset = 0; // 버퍼 오프셋 초기화
	// 정점 버퍼 설정
	Device.
DeviceContext.IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &offset);

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