#pragma once
#include "Types.h"

class UEngineStatics
{
	static uint32 GenUUID()
	{
		return NextUUID++;
	}

	static uint32 NextUUID;
};
