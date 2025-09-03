#pragma once
#include "Mesh.h"
#include "Matrix.h"

class UMeshRenderer
{
private:
	// 렌더링에 필요한 데이터
	UMesh* Mesh = nullptr;
	UINT DrawOrder = 0;
public:
	const static UINT PikkachuOrder;
	const static UINT BallOrder;
	const static UINT NetOrder;
public:
	// 생성자에서 UMesh를 받아 초기화하거나, 별도의 SetMesh 함수를 둘 수 있습니다.
	UMeshRenderer(UMesh* InMesh) : Mesh(InMesh) {}
	UMeshRenderer(UMesh* InMesh, UINT InDrawOrder) : Mesh(InMesh), DrawOrder(InDrawOrder) {}
	~UMeshRenderer();

	void DrawMesh(const FMatrix4x4& Transform);
	void SetDrawOrder(UINT inDrawOrder)
	{
		DrawOrder = inDrawOrder;
	}
	const UINT GetDrawOrder() const
	{
		return DrawOrder;
	}
};