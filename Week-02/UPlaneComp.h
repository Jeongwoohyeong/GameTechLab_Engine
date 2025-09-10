#pragma once

#include "UPrimitiveComponent.h"
#include "URenderer.h"
#include "RTTIMacros.h"

struct ID3D11Buffer;

class UPlaneComp :public UPrimitiveComponent
{
	RTTI_DECLARE()
public:
	UPlaneComp();
	~UPlaneComp() override {};

	virtual EPrimitiveType GetPrimitiveType() override;
	virtual FMesh* GetMesh() override { return URenderer::PlaneMesh; }
	virtual void TypeCheck() override;

private:
	EPrimitiveType Type;
};