#include "UTriangleComp.h"

UTriangleComp::UTriangleComp()
	:Type(EPrimitiveType::Triangle)
{
}

EPrimitiveType UTriangleComp::GetPrimitiveType()
{
	return Type;
}

void UTriangleComp::IsA()
{
	UE_LOG(PrimitiveTypeToString(Type).c_str());
}
