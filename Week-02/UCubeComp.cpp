#include "UCubeComp.h"
#include "d3d11.h"
#include "URenderer.h"
#include "Cube.h"

UCubeComp::UCubeComp()
	:Type(EPrimitiveType::Cube)
{
}

EPrimitiveType UCubeComp::GetPrimitiveType()
{
	return Type;
}

void UCubeComp::TypeCheck()
{
	UE_LOG(PrimitiveTypeToString(Type).c_str());
}
