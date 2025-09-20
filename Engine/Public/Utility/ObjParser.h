#pragma once

struct FNormalVertex;

class FObjParser
{
	DECLARE_SINGLETON(FObjParser);

public:
	FStaticMesh* LoadObjStaticMesh(const FString& filePath);

private:
	static inline FVector PositionToUEBasis(const FVector& InVector) { return FVector(InVector.X, -InVector.Y, InVector.Z); }

	static inline FVector2 UVToBasis(const FVector2& InVector) { return FVector2(InVector.X, 1.0f - InVector.Y); }
};
