#pragma once

#include "Containers.h"
#include "UScene.h"

class CSceneSerializer
{
public:
	CSceneSerializer() = default;

	void SaveScene(const FString& Filepath, const UScene& Scene);
	UScene LoadScene(const FString& Filepath);
};
