#pragma once
#include "Core/Object.h"

class UMaterial : public UObject
{
	DECLARE_CLASS(UMaterial, UObject)

public:
	UMaterial();
	UMaterial(TMap<FString, FObjMaterialInfo*>* InFObjMaterial);
	~UMaterial();
	
	const FObjMaterialInfo* GetMaterialInfo(const FString& MtlName) const;
	const FString& GetKdTextureFilePath(const FString& MtlName);
	const FString& GetKsTextureFilePath(const FString& MtlName);
	const FString& GetBumpTextureFilePath(const FString& MtlName);

	void SetMaterialInfo(TMap<FString, FObjMaterialInfo*>* InFObjMaterial);

private:
	TMap<FString, FObjMaterialInfo*>* ObjMaterialInfoMap;
	FString KdTextureFilePath{};
	FString KsTextureFilePath{};
	FString BumpTextureFilePath{};

};
