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


	FVector4 GetColor() const { return Color; }

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
	EPrimitiveType Type = EPrimitiveType::Cube;

	bool bVisible = true;

};
