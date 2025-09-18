#pragma once
#include "Core/Object.h"

class UStaticMesh : public UObject
{
	DECLARE_CLASS(UStaticMesh, UObject)

public:

	const FString& GetAssetPathFileName() const { return StaticMeshAsset->PathFileName; }

	void SetStaticMeshAsset(FStaticMesh* InStaticMesh) { StaticMeshAsset = InStaticMesh; }


private:
	FStaticMesh* StaticMeshAsset = nullptr;

};

