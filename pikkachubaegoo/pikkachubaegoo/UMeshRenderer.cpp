#include "UMeshRenderer.h"
#include "Mesh.h" // cpp 파일에서 실제 UMesh의 정의를 포함

UMeshRenderer::~UMeshRenderer()
{
	// 필요 시 Mesh 메모리 해제 로직 추가
}

void UMeshRenderer::DrawMesh()
{
	// 실제 메시를 그리는 로직 구현
	if (Mesh)
	{
		// ...
	}
}