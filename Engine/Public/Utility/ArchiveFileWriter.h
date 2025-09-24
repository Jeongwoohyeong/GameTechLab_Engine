#pragma once
#include"Utility/Archive.h"

class FArchiveFileWriter : public FArchive
{
public:
	FArchiveFileWriter(const FString& OriginalPath, const FString& BinFilePath);
	virtual void Serialize(void* Data, uint64 Length) override;
	virtual bool IsSaving() const override { return true; }
	virtual bool IsFileOpen() override { return FilePointer != nullptr; }
	virtual bool IsFileClose() override { return FilePointer == nullptr; }
	virtual void FileClose() override;
	virtual bool IsFileExist(const FString& FilePath) override;
	virtual bool IsBinOld(const FString& OriginalFile, const FString& BinFile) override;

private:
	bool CheckDirectory(const FString& FilePath);

private:
	FILE* FilePointer;
};
