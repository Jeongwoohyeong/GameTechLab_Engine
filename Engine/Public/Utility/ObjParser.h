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

static inline int32 ResolveIndex(int32 i, int32 count) { return (i > 0) ? (i - 1) : (count + i); }
