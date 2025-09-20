#pragma once
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "Manager/ResourceManager.h"
#include "Math/AABB.h"

class ULevel;
class UPrimitiveComponent : public USceneComponent
{
	DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
	UPrimitiveComponent();

	const TArray<FVertex>* GetVerticesData() const;
	const TArray<FVertex>* GetReducedVerticesData() const;
	const TArray<uint32>* GetIndicesData() const;

	ID3D11Buffer* GetVertexBuffer() const { return VertexBuffer; }
	ID3D11Buffer* GetReducedVertexBuffer() const { return ReducedVertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() const { return IndexBuffer; }

	uint32 GetVertexNum() const { return VertexNum; }
	uint32 GetReducedVertexNum() const { return ReducedVertexNum; }
	uint32 GetIndexNum() const { return IndexNum; }

	FVector4 GetColor() const { return Color; }
	const FRenderState& GetRenderState() const { return RenderState; }
	D3D11_PRIMITIVE_TOPOLOGY GetTopology() const;


	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY InTopology);
	void SetVisibility(bool bVisibility) { bVisible = bVisibility; }
	void SetColor(const FVector4& InColor) { Color = InColor; }

	bool IsVisible() const { return bVisible; }
	virtual bool IsRayCollided(const FRay& ModelRay, const FMatrix& ModelMatrix, float* ShortestDistance) = 0;
	virtual FAABB GetWorldBounds() const;



	//StaticMesh가 구현되면 주석 해제(09/19 13:05)
	//자식 Component들이 본인의 타입에 맞게 알아서 RenderList에 저장하도록 하기 위해 가상함수 선언
	virtual void AddToRenderList(ULevel* Level) = 0;
	///////////////////////////////////////////////////


	

	
	//FAABB GetWorldBounds() const;

protected:
	const TArray<FVertex>* Vertices = nullptr;
	FVector4 Color = FVector4{ 0.f,0.f,0.f,0.f };
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* ReducedVertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	uint32 VertexNum = 0;
	uint32 ReducedVertexNum = 0;
	uint32 IndexNum = 0;
	D3D11_PRIMITIVE_TOPOLOGY Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	FRenderState RenderState = {};
	EPrimitiveType Type = EPrimitiveType::Cube;

	bool bVisible = true;

};
