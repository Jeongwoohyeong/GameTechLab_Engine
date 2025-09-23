#pragma once
#include"Utility/Archive.h"

class FArchiveFileReader : public FArchive
{
public:
	FArchiveFileReader(const FString& FilePath);
	virtual void Serialize(void* Data, uint64 Length) override;
	virtual bool IsLoading() const override { return true; }
	virtual bool IsFileOpen() override { return FilePointer != nullptr; }
	virtual bool IsFileClose() override { return FilePointer == nullptr; }
	virtual void FileClose() override;
	virtual bool IsFileExist(const FString& FilePath) override;

private:

private:
	FILE* FilePointer;
};
