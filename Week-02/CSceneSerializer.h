#pragma once

#include "FScene.h"

class CSceneSerializer
{
public:
	static void SaveScene(const FString& Name, const FScene& Scene);
	static FScene LoadScene(const FString& Name);
};
