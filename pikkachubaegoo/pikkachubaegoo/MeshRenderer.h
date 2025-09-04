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

	//가리키는것 생성,파괴 x
	ID3D11ShaderResourceView* TextureSRV;
	ID3D11PixelShader* PS;
	ID3D11VertexShader* VS;
	ID3D11InputLayout* InputLayout;
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
	UMeshRenderer(UMesh* InMesh, UINT InDrawOrder, ID3D11ShaderResourceView* textureSRV);
	~UMeshRenderer();

	void SetTextureSRV(ID3D11ShaderResourceView* textureSRV)
	{
		TextureSRV = textureSRV;
	}
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
	void SetShader(ID3D11InputLayout* inputLayout, ID3D11VertexShader* vs, ID3D11PixelShader* ps);
};