#include "USphereComp.h"

USphereComp::USphereComp()
	:Type(EPrimitiveType::Sphere)
{
}

EPrimitiveType USphereComp::GetPrimitiveType()
{
	return Type;
}
