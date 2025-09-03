#pragma once
#include <string>
#include "Vector.h"

struct FSpriteFrame
{
	std::string name{};
	FVector4 spriteSourceSize{};
	FVector2 sourceSize{};
	FVector2 position{};
	FVector2 size{};
	bool isRotate{};
	bool isTrim{};
};

struct FSpriteMeta
{
	std::string imagePath{};
	FVector2 size{};
	float scale{};
};