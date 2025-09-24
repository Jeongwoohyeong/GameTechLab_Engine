#include "pch.h"
#include "FileManager.h"

IMPLEMENT_SINGLETON(IFileManager);

IFileManager::IFileManager()
{

}

IFileManager::~IFileManager()
{

}

FArchive* IFileManager::CreateFileWriter(const FString& OriginalPath, const FString& BinFilePath)
{
	FArchive* Writer = new FArchiveFileWriter(OriginalPath, BinFilePath);
	return Writer;
}

FArchive* IFileManager::CreateFileReader(const FString& FilePath)
{
	FArchive* Reader = new FArchiveFileReader(FilePath);
	return Reader;
}
