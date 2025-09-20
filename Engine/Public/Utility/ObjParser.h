#pragma once

struct FNormalVertex;


class FObjParser
{
	DECLARE_SINGLETON(FObjParser);

public:
	FStaticMesh* LoadObjStaticMesh(const FString& filePath);

private:
	static bool ParseFaceTriplet(const FString& s, int32& v, int32& vt, int32& vn);
	static bool ParseObjRaw(const FString& filePath, FObjInfo& outRawData);
	static bool CookObjToStaticMesh(const FObjInfo& raw, const FObjImportOption& opt, FStaticMesh& outMesh);
	
};

static inline FVector PositionToUEBasis(const FVector& InVector) { return FVector(InVector.X, -InVector.Y, InVector.Z); }

static inline FVector2 UVToBasis(const FVector2& InVector) { return FVector2(InVector.X, 1.0f - InVector.Y); }

// 인덱스가 음수인 경우 마지막 위치에서의 상대적 위치를 의미한다.
// 10개의 정점이 존재할 때 인덱스가 -3이면 10 - 1 = 7번 째 인덱스
static inline int32 ResolveIndex(int32 i, int32 count) { return (i > 0) ? (i - 1) : (count + i); }
