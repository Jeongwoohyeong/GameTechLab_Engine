#pragma once
#include "UPrimitiveComponent.h"
#include "URenderer.h"
#include "RTTIMacros.h"

struct ID3D11Buffer;

class USphereComp :public UPrimitiveComponent
{
	RTTI_DECLARE()
public:
	USphereComp();
	~USphereComp() override {};

	virtual EPrimitiveType GetPrimitiveType() override;
	virtual FMesh* GetMesh() override { return URenderer::SphereMesh; }
	virtual void TypeCheck() override;

private:
	EPrimitiveType Type;
};