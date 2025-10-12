#pragma once

#include <fstream>
#include <sstream>

#include "nlohmann/json.hpp"   
#include "Vector.h"
#include "UEContainer.h"
#include "SerializationData.h"

using namespace json;

class FSceneLoader
{
public:
    // Version 2 API
    static FSceneData Load(const FString& FileName);
    static void Save(const FSceneData& SceneData, const FString& SceneName);

    static bool TryReadNextUUID(const FString& FilePath, uint32& OutNextUUID);

private:
    static FSceneData Parse(const JSON& Json);
};