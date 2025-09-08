#pragma once

#include "Types.h"
#include "Containers.h"
#include "UPrimitiveComponent.h"

class CScene
{
public:
	static CScene& GetInstance()
	{
		static CScene Instance;
		return Instance;
	}

	void New();
	void Save(const FString& Name);
	void Load(const FString& Name);

private:
	uint32 Version = 1;
	TArray<UPrimitiveComponent*> Primitives;
};
