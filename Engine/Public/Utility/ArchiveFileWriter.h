#pragma once
#include"Utility/Archive.h"

class FArchiveFileWriter : public FArchive
{
public:
	FArchiveFileWriter(const FString& FilePath);
	virtual void Serialize(void* Data, uint64 Length) override;
	virtual bool IsSaving() const override { return true; }
	virtual bool IsFileOpen() override { return FilePointer != nullptr; }
	virtual bool IsFileClose() override { return FilePointer == nullptr; }
	virtual void FileClose() override;
	virtual bool IsFileExist(const FString& FilePath) override;

private:
	bool CheckDirectory(const FString& FilePath);

private:
	FILE* FilePointer;
};
