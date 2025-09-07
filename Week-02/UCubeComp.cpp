#include "UCubeComp.h"

UCubeComp::UCubeComp()
	:Type(EPrimitiveType::Cube) 
{
}

EPrimitiveType UCubeComp::GetPrimitiveType()
{
	return Type;
}
