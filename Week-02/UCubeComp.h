#pragma once
#include "UPrimitiveComponent.h"

class UCubeComp :public UPrimitiveComponent
{
public:
	UCubeComp();
	~UCubeComp() override {};
	
	virtual EPrimitiveType GetPrimitiveType() override;

private:
	EPrimitiveType Type;
};
