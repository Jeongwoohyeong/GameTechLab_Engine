#pragma once

#include "FScene.h"

class CSceneSerializer
{
public:
	CSceneSerializer() = default;

	void SaveScene(const FString& Name, const FScene& Scene);
	FScene LoadScene(const FString& Name);
};
