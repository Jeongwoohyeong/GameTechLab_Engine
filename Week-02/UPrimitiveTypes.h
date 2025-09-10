#pragma once
#include "Types.h"
#include "Containers.h"

enum class EPrimitiveType : uint8
{
	Cube,
	Sphere,
	Triangle,
	Plane,
};

static inline FString PrimitiveTypeToString(EPrimitiveType Type)
{
	switch (Type)
	{
	case EPrimitiveType::Cube:
		return "Cube";
	case EPrimitiveType::Sphere:
		return "Sphere";
	case EPrimitiveType::Triangle:
		return "Triangle";
	case EPrimitiveType::Plane:
		return "Plane";
	default:
		return "Unknown";
	}
}
