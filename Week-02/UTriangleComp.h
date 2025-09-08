#pragma once
#include "UPrimitiveComponent.h"
#include "URenderer.h"

struct ID3D11Buffer;

class UTriangleComp : public UPrimitiveComponent
{
public:
	UTriangleComp();
	~UTriangleComp() override {};

	virtual EPrimitiveType GetPrimitiveType() override;
	virtual FMesh* GetMesh() override { return URenderer::TriangleMesh; }

private:
	EPrimitiveType Type;
};

