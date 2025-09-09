#pragma once

#include "Types.h"
#include "Containers.h"
#include "UPrimitiveComponent.h"
#include "UPrimitiveTypes.h"
#include "UCamera.h"

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

	void Spawn(EPrimitiveType Type, uint32 Count);
	void DestroySelectedPrimitive();

	int32 PickGizmoAtMouse(int ClientX, int ClientY, int ClientW, int ClientH);
    UPrimitiveComponent* PickUObjectAtMouse(int ClientX, int ClientY, int ClientW, int ClientH, uint32& OutUUID);

	TMap<uint32, UPrimitiveComponent*>& GetPrimitives() { return UUIDToPrimitive; }
	UPrimitiveComponent* GetSelectedPrimitive() { return SelectedPrimitive; }
	void SetSelectedPrimitiveByUUID(uint32 UUID);

private:
	void Clear();

private:
	uint32 Version = 1;
	TMap<uint32, UPrimitiveComponent*> UUIDToPrimitive;
	UPrimitiveComponent* SelectedPrimitive = nullptr;
};
