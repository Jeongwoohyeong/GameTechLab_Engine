#pragma once
#include "UPrimitiveComponent.h"
#include "URenderer.h"
#include "RTTIMacros.h"

struct ID3D11Buffer;

class UTriangleComp : public UPrimitiveComponent
{
	RTTI_DECLARE()
public:
	UTriangleComp();
	~UTriangleComp() override {};

	virtual EPrimitiveType GetPrimitiveType() override;
	virtual FMesh* GetMesh() override { return URenderer::TriangleMesh; }
	virtual void TypeCheck() override;

private:
	EPrimitiveType Type;
};

