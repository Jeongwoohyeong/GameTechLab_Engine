#include "pch.h"
#include "FileManager.h"

IMPLEMENT_SINGLETON(IFileManager);

IFileManager::IFileManager()
{

}

IFileManager::~IFileManager()
{

}

FArchive* IFileManager::CreateFileWriter(const FString& FilePath)
{
	FArchive* Writer = new FArchiveFileWriter(FilePath);
	return Writer;
}

FArchive* IFileManager::CreateFileReader(const FString& FilePath)
{
	FArchive* Reader = new FArchiveFileReader(FilePath);
	return Reader;
}
