#include "UTriangleComp.h"

UTriangleComp::UTriangleComp()
	:Type(EPrimitiveType::Triangle)
{
}

EPrimitiveType UTriangleComp::GetPrimitiveType()
{
	return Type;
}
