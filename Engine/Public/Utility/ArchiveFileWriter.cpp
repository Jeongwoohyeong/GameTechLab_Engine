#include "pch.h"
#include "ArchiveFileWriter.h"

FArchiveFileWriter::FArchiveFileWriter(const FString& FilePath)
{
	// 이진파일 쓰기
	fopen_s(&FilePointer, FilePath.c_str(), "wb");
	if (FilePointer == nullptr)
	{
		assert("FArchiveFileWriter File Open 실패");
	}
}

void FArchiveFileWriter::Serialize(void* Data, uint64 Length)
{
	if (FilePointer)
	{
		fwrite(Data, 1, Length, FilePointer);
	}
}
