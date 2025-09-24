#pragma once
#include "Utility/ArchiveFileReader.h"
#include "Utility/ArchiveFileWriter.h"

enum class EFileFormat : uint8
{
	EFF_Obj,
	EFF_Mtl
};

class IFileManager
{
	DECLARE_SINGLETON(IFileManager);

public:
	FArchive* CreateFileWriter(const FString& OriginalPath, const FString& BinFilePath);
	FArchive* CreateFileReader(const FString& FilePath);
};
