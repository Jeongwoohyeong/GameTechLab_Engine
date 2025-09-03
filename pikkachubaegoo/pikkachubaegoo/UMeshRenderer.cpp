#include "UMeshRenderer.h"
#include "App.h"

UMeshRenderer::~UMeshRenderer()
{
	if (Mesh)
	{
		//Mesh->Release();	게임 종료시 UApp에서 Release 호출해줘서 주석 처리
		Mesh = nullptr;
	}
}

void UMeshRenderer::DrawMesh(const FMatrix4x4& Transform)
{
	// 실제 메시를 그리는 로직 구현
	if (Mesh)
	{
		D3DUtil::UpdateConstantBuffer(UApp::Ins->GetContext(), UApp::Ins->GetTransformCBuffer(), Transform);
		Mesh->Draw();
	}
}