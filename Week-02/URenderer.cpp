#include <d3d11.h>
#include "URenderer.h"
#include "UD3dDevice.h"
#include "UShader.h"
#include "UCamera.h"
#include "UCubeComp.h"
#include "CScene.h"

#include "Cube.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Cone.h"
#include "Cylinder.h"
#include "RotationRing.h"
#include "GizmoCube.h"

#include "WorldGizmo.h"

#include "ShapeData.h"

FMesh* URenderer::CubeMesh = nullptr;
FMesh* URenderer::SphereMesh = nullptr;
FMesh* URenderer::TriangleMesh = nullptr;
FMesh* URenderer::ConeMesh = nullptr;
FMesh* URenderer::CylinderMesh = nullptr;
FMesh* URenderer::RingMesh = nullptr;
FMesh* URenderer::GizmoCubeMesh = nullptr;

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

	if (!this->CreateAllMesh())
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
	
	SelectedGizmo = ConeMesh;

	return true;
}

// 파이프라인 설정 -> 씬 -> 월드 기즈모 -> UI 순으로 렌더링
void URenderer::Render()
{
	Device->BeginScene(); // DeviceContext 설정
	Device->SetRSState(RasterizerState);
	Shader->PrepareShader();
	
	// 씬 렌더링
	RenderScene();

	worldGizmo->Render(this);

	// UI 렌더링
	RenderUI();

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
	rasterizerDesc.DepthClipEnable = TRUE;
	result = Device->GetDeivce()->CreateRasterizerState(&rasterizerDesc, &RasterizerState);
	if (FAILED(result))
	{
		MessageBox(nullptr, L"rasterizerstate create fail,", L"error", MB_OK);
		return false;
	}

	return true;
}
bool URenderer::CreateAllMesh()
{
	CreateMesh(CubeMesh, GCubeVertices, sizeof(GCubeVertices), GCubeIndices, sizeof(GCubeIndices));
	CreateMesh(TriangleMesh, GTriangleVertices, sizeof(GTriangleVertices));
	CreateMesh(SphereMesh, GSphereVertices, sizeof(GSphereVertices));
	CreateMesh(CylinderMesh, GCylinderVertices, sizeof(GCylinderVertices), GCylinderIndices, sizeof(GCylinderIndices));
	CreateMesh(ConeMesh, GConeVertices, sizeof(GConeVertices), GConeIndices, sizeof(GConeIndices));
	CreateMesh(RingMesh, GAxisRingVertices, sizeof(GAxisRingVertices), GAxisRingIndices, sizeof(GAxisRingIndices));
	CreateMesh(GizmoCubeMesh, GGizmoCubeVertices, sizeof(GGizmoCubeVertices), GGizmoCubeIndices, sizeof(GGizmoCubeVertices));

	return true;
}

// 함수 정의에서 기본 인수를 제거합니다.
bool URenderer::CreateMesh(FMesh*& mesh, FVertexSimple* vertices, int verticeSize, uint32* indices, int indiceSize)
{
	mesh = new FMesh();
	// 버텍스 데이터 초기화
	mesh->Vertices = vertices;
	mesh->VertexByteWidth = verticeSize;
	mesh->Stride = sizeof(FVertexSimple);
	mesh->Offset = 0;

	// 인덱스 데이터 초기화
	if (indices != nullptr)
	{
		mesh->Indices = indices;
		mesh->IndexByteWidth = indiceSize;
		mesh->IndexCount = indiceSize/sizeof(uint32);
		mesh->bUseIndexBuffer = true;
	}
	else
	{
		mesh->Indices = nullptr;
		mesh->bUseIndexBuffer = false;
		mesh->IndexByteWidth = 0;
		mesh->IndexCount = 0;
	}

	// 버퍼 생성
	if (!this->CreateVertexBuffer(mesh))
	{
		return false;
	}

	if (mesh->bUseIndexBuffer)
	{
		if (!this->CreateIndexBuffer(mesh))
		{
			return false;
		}
	}

	return true;
}   


void URenderer::Resize(UINT width, UINT height)
{
	if (Device == nullptr)
	{
		return;
	}

	Device->Resize(width, height);
	// TODO#4: 전체적으로 URenderer와 UCamera의 의존도가 낮은 상태임을 고려, 다른 곳으로 옮길 것
	UCamera::GetInstance().AspectRatio = (float)width / (float)height; 
}

bool URenderer::CreateVertexBuffer(FMesh* Mesh)
{
	HRESULT result;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = Mesh->VertexByteWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferSRD = {};
	vertexBufferSRD.pSysMem = Mesh->Vertices;

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

	D3D11_SUBRESOURCE_DATA indexBufferSRD = {};
	indexBufferSRD.pSysMem = Mesh->Indices;

	hr = Device->GetDeivce()->CreateBuffer(&indexBufferDesc, &indexBufferSRD, &Mesh->IndexBuffer);
	if (FAILED(hr))
	{
		MessageBox(nullptr, L"indexbuffer create fail,", L"error", MB_OK);
		return false;
	}
	return true;
}

void URenderer::RenderScene()
{
	// 씬의 프리미티브 렌더링
	TMap<uint32, UPrimitiveComponent*>& Primitives = CScene::GetInstance().GetPrimitives();
	for (const auto& Pair : Primitives)
	{
		RenderPrimitive(Pair.second);
	}
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
	if(Primitive == CScene::GetInstance().GetSelectedPrimitive())
	{
		Render(Mesh, DeviceContext, &World);
	}
	else
	{
		Render(Mesh, DeviceContext, nullptr);
	}
	
	// 선택된 프리미티브면 로컬 기즈모 렌더링
	if (CScene::GetInstance().GetSelectedPrimitive() == Primitive)
	{
		if (RenderLocalGizmo(Primitive) == false)
		{
			return false;
		}
	}
	
	return true;
}

bool URenderer::RenderLocalGizmo(UPrimitiveComponent* Primitive)
{
	ID3D11DeviceContext* DeviceContext = Device->GetDeviceContext();

	// color 추가  렌더링
	//FTransform* gizmoTrans = Primitive->GetGizmoTransforms();

	//for (int i = 0; i < 3; i++)
	//{
	//	FMatrix World = FMatrix::Identity();
	//	gizmoTrans[i].SetScale(0.1f, 0.8f, 0.1f); // location gizmo
	//	World = World * gizmoTrans[i].GetTransformMatrix();
	//	Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(World), GAxisColors[i]);
	//	Render(ConeMesh, DeviceContext, nullptr);		

	//	World = FMatrix::Identity();
	//	gizmoTrans[i].SetScale(0.03f, 0.8f, 0.03f);
	//	World = World * gizmoTrans[i].GetTransformMatrix();
	//	Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(World), GAxisColors[i]);
	//	Render(CylinderMesh, DeviceContext, nullptr);		
	//}

	//// 테스트용	
	FTransform* gizmoTrans = Primitive->GetGizmoTransforms();
	constexpr int gizmoCount = 2;
	if (CInputManager::GetInstance().IsKeyPressed(VK_SPACE))
	{
		UE_LOG("space pressed");
		GizmoSwitch = (GizmoSwitch + 1) % gizmoCount;
		switch (GizmoSwitch)
		{
		case 0:
			UE_LOG("gizmo swith %d", GizmoSwitch);
			SelectedGizmo = ConeMesh;
			break;
		case 1:
			UE_LOG("gizmo swith %d", GizmoSwitch);
			SelectedGizmo = GizmoCubeMesh;
			break;
		case 2:
			break;
		default:
			break;
		}
		Primitive->SwitchGizmo(GizmoSwitch);
	}

	for (int i = 0; i < 3; i++)
	{
		FMatrix World = FMatrix::Identity();
		gizmoTrans[i].SetScale(0.1f, 0.8f, 0.1f); // location gizmo
		World = World * gizmoTrans[i].GetTransformMatrix();
		Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(World), GAxisColors[i]);
		Render(SelectedGizmo, DeviceContext, nullptr);

		World = FMatrix::Identity();
		gizmoTrans[i].SetScale(0.03f, 0.8f, 0.03f);
		World = World * gizmoTrans[i].GetTransformMatrix();
		Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(World), GAxisColors[i]);
		Render(CylinderMesh, DeviceContext, nullptr);		
	}

	////

	return true;
}

// TODO: 하이라이팅을 위해 임시로 World 매개변수를 추가. 리팩토링 필요함
void URenderer::Render(FMesh* mesh, ID3D11DeviceContext* DeviceContext, FMatrix* World)
{
	if (mesh->bUseIndexBuffer)
	{
		DeviceContext->IASetVertexBuffers(0, 1, &mesh->VertexBuffer, &mesh->Stride, &mesh->Offset);
		DeviceContext->IASetIndexBuffer(mesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		DeviceContext->DrawIndexed(mesh->IndexCount, 0, 0);
	}
	else
	{
		DeviceContext->IASetVertexBuffers(0, 1, &mesh->VertexBuffer, &mesh->Stride, &mesh->Offset);
		DeviceContext->Draw(mesh->VertexByteWidth / mesh->Stride, 0);
	}
    
#pragma region draw outline
	if(World == nullptr)
	{
		return; // 외곽선 그리기 패스 건너뜀
	}
	// --- Pass A: 스텐실 마킹 (컬러 미기록)
	DeviceContext->OMSetDepthStencilState(Device->GetDS_StencilMark(), 1);
	DeviceContext->OMSetBlendState(Device->GetBS_ColorOff(), nullptr, 0xffffffff);

	if (mesh->bUseIndexBuffer)
	{
		DeviceContext->IASetVertexBuffers(0, 1, &mesh->VertexBuffer, &mesh->Stride, &mesh->Offset);
		DeviceContext->IASetIndexBuffer(mesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		DeviceContext->DrawIndexed(mesh->IndexCount, 0, 0);
	}
	else
	{
		DeviceContext->IASetVertexBuffers(0, 1, &mesh->VertexBuffer, &mesh->Stride, &mesh->Offset);
		DeviceContext->Draw(mesh->VertexByteWidth / mesh->Stride, 0);
	}

	// --- Pass B: 외곽선(팽창 + Front면 컬링)
	DeviceContext->OMSetDepthStencilState(Device->GetDS_Outline(), 1);
	DeviceContext->OMSetBlendState(Device->GetBS_Alpha(), nullptr, 0xffffffff); // 불투명 원하면 nullptr
	DeviceContext->RSSetState(Device->GetRS_CullFront());

	const float k = 1.1f; // 두께(씬에 맞춰 1.01~1.1 튜닝)
	FMatrix Scale = FMatrix::MakeScale(FVector(k, k, k));
	FMatrix OutlineWorld = Scale * (*World);

	// 단색으로 그리도록 useUColor=1 경로 사용
	Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(OutlineWorld), FVector(1.0f, 0.3f, 0.1f));

	if (mesh->bUseIndexBuffer)
	{
		DeviceContext->IASetVertexBuffers(0, 1, &mesh->VertexBuffer, &mesh->Stride, &mesh->Offset);
		DeviceContext->IASetIndexBuffer(mesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		DeviceContext->DrawIndexed(mesh->IndexCount, 0, 0);
	}
	else
	{
		DeviceContext->IASetVertexBuffers(0, 1, &mesh->VertexBuffer, &mesh->Stride, &mesh->Offset);
		DeviceContext->Draw(mesh->VertexByteWidth / mesh->Stride, 0);
	}

	// --- 상태 원복 (다음 오브젝트에 영향 X)
	DeviceContext->OMSetDepthStencilState(Device->GetDepthStateOpaque(), 0);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	DeviceContext->RSSetState(nullptr);

	// (선택) 상수 버퍼도 원래 World로 되돌려둠
	Shader->UpdateConstant(UCamera::GetInstance().MakeMVP(*World));
	
#pragma endregion
}

void URenderer::RenderUI()
{
	UI.PrepareRender();

	UI.PropertyWindow(CScene::GetInstance().GetSelectedPrimitive());
	UI.ControlPanel();
	UI.ConsoleWindow(true);

	UI.Render();
}

void URenderer::ReleaseAllMesh()
{
	ReleaseMesh(CubeMesh);
	ReleaseMesh(SphereMesh);
	ReleaseMesh(TriangleMesh);
	ReleaseMesh(ConeMesh);
	ReleaseMesh(CylinderMesh);
}

void URenderer::ReleaseMesh(FMesh* mesh)
{
	if (mesh)
	{
		if (mesh->IndexBuffer)
		{
			mesh->IndexBuffer->Release();
			mesh->IndexBuffer = nullptr;
		}
		if (mesh->VertexBuffer)
		{
			mesh->VertexBuffer->Release();
			mesh->VertexBuffer = nullptr;
		}
		delete mesh;
		mesh = nullptr;
	}
}

#pragma region World Gizmo 사용, 추후 수정

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