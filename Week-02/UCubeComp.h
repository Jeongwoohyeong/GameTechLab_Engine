#pragma once
#include "UPrimitiveComponent.h"
#include "Types.h"
#include "URenderer.h"

struct ID3D11Buffer;

class UCubeComp :public UPrimitiveComponent
{
public:
	UCubeComp();
	~UCubeComp() override {};
	
	virtual EPrimitiveType GetPrimitiveType() override;
	virtual FMesh* GetMesh() override { return URenderer::CubeMesh; }

private:
	EPrimitiveType Type;	
};
