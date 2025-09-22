#pragma once

struct FObjMaterialInfo;

class FMtlParser
{
public:
	FMtlParser() {};
	FMtlParser(TMap<FString, TMap<FString, FObjMaterialInfo*>>*);
	~FMtlParser() {};
	bool ParseMtl(const FString& PathFileName);	

private:
	TMap<FString, TMap<FString, FObjMaterialInfo*>>* MtlFileMap = nullptr;
};
