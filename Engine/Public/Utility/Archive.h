#pragma once

struct FStaticMesh;
struct FObjMaterialInfo;

class FArchive
{
public:

	virtual void Serialize(void* Data, uint64 Length) = 0;
	virtual bool IsLoading() const { return false; }
	virtual bool IsSaving() const { return false; }
	virtual bool IsFileOpen() = 0;
	virtual bool IsFileClose() = 0;
	virtual void FileClose() = 0;
	virtual bool IsFileExist(const FString& FilePath) = 0;

private:

};


#pragma region operator<<overloading

FArchive& operator<<(FArchive& Ar, int8& Value);

FArchive& operator<<(FArchive& Ar, int16& Value);

FArchive& operator<<(FArchive& Ar, int32& Value);

FArchive& operator<<(FArchive& Ar, int64& Value);

FArchive& operator<<(FArchive& Ar, uint8& Value);

FArchive& operator<<(FArchive& Ar, uint16& Value);

FArchive& operator<<(FArchive& Ar, uint32& Value);

FArchive& operator<<(FArchive& Ar, uint64& Value);

FArchive& operator<<(FArchive& Ar, float& Value);

FArchive& operator<<(FArchive& Ar, FString& Value);

FArchive& operator<<(FArchive& Ar, FVector2& Value);

FArchive& operator<<(FArchive& Ar, FVector& Value);

FArchive& operator<<(FArchive& Ar, FVector4& Value);

FArchive& operator<<(FArchive& Ar, FNormalVertex& Value);

FArchive& operator<<(FArchive& Ar, FMeshSection& Value);

template<typename T>
FArchive& operator<<(FArchive& Ar, TArray<T>& Value);

FArchive& operator<<(FArchive& Ar, FStaticMesh& Value);

FArchive& operator<<(FArchive& Ar, FObjMaterialInfo& Value);

#pragma endregion
