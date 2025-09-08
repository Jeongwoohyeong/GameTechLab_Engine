#pragma once
#include "UPrimitiveComponent.h"
#include "Types.h"

struct ID3D11Buffer;

class UCubeComp :public UPrimitiveComponent
{
public:
	UCubeComp();
	~UCubeComp() override {};
	
	virtual EPrimitiveType GetPrimitiveType() override;
	virtual void RenderPrimitive(ID3D11DeviceContext*) override;

private:
	EPrimitiveType Type;	
};
