#pragma once
#include "Utility/ArchiveFileReader.h"
#include "Utility/ArchiveFileWriter.h"

class IFileManager
{
	DECLARE_SINGLETON(IFileManager);

	FArchive* CreateFileWriter(const FString& FilePath);
	FArchive* CreateFileReader(const FString& FilePath);
};
