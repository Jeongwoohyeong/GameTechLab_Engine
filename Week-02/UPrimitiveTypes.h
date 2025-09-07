#pragma once
#include "Types.h"

enum class EPrimitiveType : uint8
{
	Cube,
	Sphere
};

static inline FString PrimitiveTypeToString(EPrimitiveType Type)
{
	switch (Type)
	{
	case EPrimitiveType::Cube:
		return "Cube";
	case EPrimitiveType::Sphere:
		return "Sphere";
	default:
		return "Unknown";
	}
}
