#pragma once

#include "FScene.h"

class CSceneSerializer
{
public:
	static void Serialize(const FString& Name, const FScene& Scene);
	static FScene Parse(const FString& Name);
};
