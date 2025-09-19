#pragma once

struct FNormalVertex;

class FObjParser
{
	DECLARE_SINGLETON(FObjParser);

public:
	FStaticMesh* LoadObjStaticMesh(const FString& filePath);

private:
	static FVector PositionToUEBasis(const FVector& InVector);

	static FVector2 UVToBasis(const FVector2& InVector);
	
};
