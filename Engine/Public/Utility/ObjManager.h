#pragma once

struct FNormalVertex;
class UStaticMesh;
class FMtlManager;

class FObjManager
{
	DECLARE_SINGLETON(FObjManager);

public:
	void Intialize();

	UStaticMesh* LoadObjStaticMesh(const FString& PathFileName);	

private:
	static FStaticMesh* LoadObjStaticMeshAsset(const FString& PathFileName);
	static FStaticMesh* LoadObj(const FString& FilePath);
	static bool ParseFaceTriplet(const FString& s, int32& v, int32& vt, int32& vn);
	static bool ParseObjRaw(const FString& FilePath, FObjInfo& OutRawData);
	static bool CookObjToStaticMesh(const FObjInfo& Raw, const FObjImportOption& Opt, FStaticMesh& OutMesh);

private:
	static TMap<FString, FStaticMesh*> ObjStaticMap;

	static FMtlManager* MtlManager;
};



static inline FVector PositionToUEBasis(const FVector& InVector) { return FVector(InVector.X, -InVector.Y, InVector.Z); }

static inline FVector2 UVToBasis(const FVector2& InVector) { return FVector2(InVector.X, 1.0f - InVector.Y); }

// 인덱스가 음수인 경우 마지막 위치에서의 상대적 위치를 의미한다.
// 10개의 정점이 존재할 때 인덱스가 -3이면 10 - 1 = 7번 째 인덱스
static inline int32 ResolveIndex(int32 i, int32 count) { return (i > 0) ? (i - 1) : (count + i); }
