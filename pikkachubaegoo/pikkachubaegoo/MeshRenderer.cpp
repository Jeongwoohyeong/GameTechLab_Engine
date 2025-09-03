#include "MeshRenderer.h"
#include "App.h"
const UINT UMeshRenderer::PikkachuOrder = 100;
const UINT UMeshRenderer::BallOrder = 110;
const UINT UMeshRenderer::BallTrailOrder = 109;
const UINT UMeshRenderer::NetOrder = 99;
const UINT UMeshRenderer::PunchOrder = 98;

void UMeshRenderer::Init()
{
	D3DUtil::CreateConstantBuffer(&AtlasInfoCBuffer, sizeof(FVector4));
}


UMeshRenderer::~UMeshRenderer()
{
	if (Mesh)
	{
		//Mesh->Release();	게임 종료시 UApp에서 Release 호출해줘서 주석 처리
		Mesh = nullptr;
	}
	if (AtlasInfoCBuffer)
	{
		AtlasInfoCBuffer->Release();
	}
}
void UMeshRenderer::ChangeAtlasInfo(const FVector4& atlasInfo)
{
	AtlasInfo = atlasInfo;
	D3DUtil::UpdateConstantBuffer(UApp::Ins->GetContext(), AtlasInfoCBuffer, AtlasInfo);
}

void UMeshRenderer::DrawMesh(const FMatrix4x4& Transform)
{
	// 실제 메시를 그리는 로직 구현
	if (Mesh)
	{
		D3DUtil::UpdateConstantBuffer(UApp::Ins->GetContext(), UApp::Ins->GetTransformCBuffer(), Transform);
		UApp::Ins->GetContext()->VSSetConstantBuffers(1, 1, &AtlasInfoCBuffer);
		Mesh->Draw();
	}
}