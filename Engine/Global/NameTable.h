#pragma once

class FNameTable
{
	DECLARE_SINGLETON(FNameTable);

public:
	TPair<int32, int32> FindOrAddName(const FString& Str);
	FName GetUniqueName(const FString& BaseStr);

	FString GetDisplayString(int32 Idx) const;

	void Reset();
private:
	FString ToLower(const FString& Str) const;

	TArray<FString> ComparisonStringPool; // 아래 맵의 인덱스로 액터만 지우도록
	TArray<FString> DisplayStringPool;

	// 이름, 인덱스
	TMap<FString, int32> ComparisonMap;
	TMap<FString, int32> DisplayMap;
	TMap<FString, int32> NextNumberMap;
};
