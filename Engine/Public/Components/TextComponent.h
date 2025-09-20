#pragma once
#include "Components/PrimitiveComponent.h"

class ULevel;

class UTextComponent : public UPrimitiveComponent
{
public:
	DECLARE_CLASS(UTextComponent, UPrimitiveComponent)
	UTextComponent();
	~UTextComponent();

	void SetInstanceData(const FWstring& Characters);
	void SetText(const FWstring& InText);

	TArray<FTextInstance>* GetInstanceData() { return &InstanceData; }
	FWstring GetText() const { return Text; }
	ID3D11Buffer* GetVertexBuffer() const { return VertexBuffer; }
	uint32 GetVertexNum() const { return VertexNum; }

	virtual void AddToRenderList(ULevel* Level) override;
	virtual bool IsRayCollided(const FRay& WorldRay, const FMatrix& ModelMatrix, float* Distance) override { return false; }
	virtual FAABB GetWorldBounds() override { return FAABB(); }

private:
	FWstring Text;
	TArray<FTextInstance> InstanceData;
	ID3D11Buffer* VertexBuffer = nullptr;
	uint32 VertexNum = 0;
};

