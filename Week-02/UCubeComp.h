#pragma once
#include "UPrimitiveComponent.h"
#include "Types.h"
#include "URenderer.h"
#include "RTTIMacros.h"

struct ID3D11Buffer;

class UCubeComp :public UPrimitiveComponent
{
	RTTI_DECLARE()
public:
	UCubeComp();
	~UCubeComp() override {};
	
	virtual EPrimitiveType GetPrimitiveType() override;
	virtual FMesh* GetMesh() override { return URenderer::CubeMesh; }
	virtual void TypeCheck() override;

private:
	EPrimitiveType Type;	
};
