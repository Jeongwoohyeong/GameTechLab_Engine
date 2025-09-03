#include "UMeshRenderer.h"
#include "Mesh.h" // cpp 파일에서 실제 UMesh의 정의를 포함
#include "RenderContext.h"

UMeshRenderer::~UMeshRenderer()
{
	if (Mesh)
	{
		//Mesh->Release();	게임 종료시 UApp에서 Release 호출해줘서 주석 처리
		Mesh = nullptr;
	}
}

void UMeshRenderer::DrawMesh(URenderContext* RenderContext, const FVector3& Location)
{
	// 실제 메시를 그리는 로직 구현
	if (Mesh)
	{
		RenderContext->Draw(Mesh, Location);
	}
}