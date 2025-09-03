#include "UMeshRenderer.h"
#include "Mesh.h" // cpp 파일에서 실제 UMesh의 정의를 포함
#include "RenderContext.h"

UMeshRenderer::~UMeshRenderer()
{
	if (Mesh)
	{
		Mesh->Release();
		Mesh = nullptr;
	}
}

void UMeshRenderer::DrawMesh(URenderContext* RenderContext)
{
	// 실제 메시를 그리는 로직 구현
	if (Mesh)
	{
		RenderContext->Draw(Mesh);
	}
}