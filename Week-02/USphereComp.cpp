#include "USphereComp.h"

USphereComp::USphereComp()
	:Type(EPrimitiveType::Sphere)
{
}

EPrimitiveType USphereComp::GetPrimitiveType()
{
	return Type;
}

void USphereComp::IsA()
{
	UE_LOG(PrimitiveTypeToString(Type).c_str());
}
