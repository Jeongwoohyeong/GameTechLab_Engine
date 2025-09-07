#pragma once
#include "UPrimitiveComponent.h"

class USphereComp :public UPrimitiveComponent
{
public:
	USphereComp();
	~USphereComp() override {};

	virtual EPrimitiveType GetPrimitiveType() override;

private:
	EPrimitiveType Type;
};