#include "UMeshRenderer.h"
#include "Mesh.h" // cpp 파일에서 실제 UMesh의 정의를 포함

UMeshRenderer::~UMeshRenderer()
{
	if (Mesh)
	{
		delete Mesh;
		Mesh = nullptr;
	}
}

void UMeshRenderer::DrawMesh()
{
	// 실제 메시를 그리는 로직 구현
	if (Mesh)
	{
		// ...
	}
}