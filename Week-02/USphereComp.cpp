#include "USphereComp.h"

RTTI_IMPL(USphereComp, UPrimitiveComponent)

USphereComp::USphereComp()
	:Type(EPrimitiveType::Sphere)
{
}

EPrimitiveType USphereComp::GetPrimitiveType()
{
	return Type;
}

void USphereComp::TypeCheck()
{
	UE_LOG(PrimitiveTypeToString(Type).c_str());
}
