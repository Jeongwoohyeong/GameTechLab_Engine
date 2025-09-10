#pragma once

#include "UPlaneComp.h"

RTTI_IMPL(UPlaneComp, UPrimitiveComponent);

UPlaneComp::UPlaneComp()
	:Type(EPrimitiveType::Triangle)
{
	
}

EPrimitiveType UPlaneComp::GetPrimitiveType()
{
	return Type;
}

void UPlaneComp::TypeCheck()
{
	UE_LOG(PrimitiveTypeToString(Type).c_str());
}
