#pragma once

struct FObjMaterialInfo;

class FMtlManager
{
public:
	FMtlManager() {};
	~FMtlManager() {};
	bool ParseMtl(const FString& PathFileName);
	FObjMaterialInfo* LoadMtlInfo(const FString& MtlFileName, const FString& MtlName);

private:
	// TMap<mtl파일명, TMap<mtl파일 안의 mtl명, mtl정보포인터>>
	static TMap<FString, TMap<FString, FObjMaterialInfo*>> MtlFileMap;	

};
