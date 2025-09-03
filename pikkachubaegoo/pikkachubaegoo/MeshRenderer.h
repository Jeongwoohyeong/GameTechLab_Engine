#pragma once
#include "Mesh.h"
#include "Matrix.h"

//AtlasUVAnimation
//상태와 시간에 따른 스프라이트 변경
//스프라이트 변경은 AtlasUV값 변경으로 이어짐
//

class UMeshRenderer
{
private:
	// 렌더링에 필요한 데이터
	UMesh* Mesh = nullptr;
	UINT DrawOrder = 0;
	FVector4 AtlasInfo;
	ID3D11Buffer* AtlasInfoCBuffer;
public:
	const static UINT PikkachuOrder;
	const static UINT BallOrder;
	const static UINT BallTrailOrder;
	const static UINT NetOrder;
	const static UINT PunchOrder;
	const static UINT BGSkyOrder;
	const static UINT BGGroundOrder;
	const static UINT BGMountainOrder;

public:
	// 생성자에서 UMesh를 받아 초기화하거나, 별도의 SetMesh 함수를 둘 수 있습니다.
	UMeshRenderer(UMesh* InMesh) : Mesh(InMesh)
	{
		Init();
	}
	UMeshRenderer(UMesh* InMesh, UINT InDrawOrder) : Mesh(InMesh), DrawOrder(InDrawOrder) 
	{
		Init();
	}
	void Init();
	~UMeshRenderer();

	void ChangeAtlasInfo(const FVector4& atlasInfo);
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