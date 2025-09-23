#include "pch.h"
#include "ArchiveFileReader.h"
#include "FileManager.h"

FArchiveFileReader::FArchiveFileReader(const FString& FilePath)
{
	// 이진파일 읽기
	fopen_s(&FilePointer, FilePath.c_str(), "rb");
	if (FilePointer == nullptr)
	{
		assert("FArchiveFileReader File Open 실패");
	}
}

void FArchiveFileReader::Serialize(void* Data, uint64 Length)
{
	if (FilePointer)
	{
		fread(Data, 1, Length, FilePointer);
	}
}
