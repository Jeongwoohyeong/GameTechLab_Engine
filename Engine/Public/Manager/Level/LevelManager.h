#pragma once
#include "Core/Object.h"

class ULevel;
struct FLevelMetadata;

class ULevelManager : public UObject
{
	DECLARE_CLASS(ULevelManager, UObject)
	DECLARE_SINGLETON(ULevelManager)

public:
	void Update() const;

	// Save & Load System
	bool CreateNewLevel(const FString& InLevelName);
	bool SaveCurrentLevel(const FString& InFilePath) const;
	bool LoadLevel(const FString& InLevelName, const FString& InFilePath);

	void ClearAllLevels();
	void Release();

	// Getter
	ULevel* GetCurrentLevel() const
	{
		return CurrentLevel;
	}

	static path GetLevelDirectory();
	static path GenerateLevelFilePath(const FString& InLevelName);

	// Metadata Conversion Functions
	static FLevelMetadata ConvertLevelToMetadata(ULevel* InLevel);
	static bool LoadLevelFromMetadata(ULevel* InLevel, const FLevelMetadata& InMetadata);

private:
	ULevel* CurrentLevel;
};
