#pragma once

#include "Types.h"
#include "Containers.h"
#include "UPrimitiveComponent.h"

class CScene
{
public:
	void Initialize();
	void Save(const FString& Name);
	void Load(const FString& Name);

private:
	void Serialize(const FString& Name);
	void Parse(const FString& Name);

private:
	int32 Version;
	uint32 NextUUID;
	TArray<UPrimitiveComponent*> Primitives;
};
