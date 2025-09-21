#pragma once
#include "Core/Object.h"

class UMaterial : public UObject
{
	DECLARE_CLASS(UMaterial, UObject)

public:
	UMaterial();
	UMaterial(FObjMaterialInfo* InFObjMaterial);
	~UMaterial();
	
	const FObjMaterialInfo* GetMaterialInfo() const { return ObjMaterialInfo; }
	const FString& GetKdTextureFilePath() const { return KdTextureFilePath; }
	const FString& GetKsTextureFilePath() const { return KsTextureFilePath; }
	const FString& GetBumpTextureFilePath() const { return BumpTextureFilePath; }

	void SetMaterialInfo(FObjMaterialInfo* InFObjMaterial);

private:
	FObjMaterialInfo* ObjMaterialInfo = nullptr;
	FString KdTextureFilePath{};
	FString KsTextureFilePath{};
	FString BumpTextureFilePath{};

};
