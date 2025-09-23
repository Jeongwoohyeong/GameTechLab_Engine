#include "pch.h"
#include "ArchiveFileWriter.h"

FArchiveFileWriter::FArchiveFileWriter(const FString& FilePath)
{
	// 이진파일 쓰기
	if (CheckDirectory(FilePath))
	{
		fopen_s(&FilePointer, FilePath.c_str(), "wb");
	}
	
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

void FArchiveFileWriter::FileClose()
{
	if (FilePointer)
	{
		if (fclose(FilePointer) != 0)
		{
			assert("File Close Fail");
		}
		FilePointer = nullptr;
	}	
}

bool FArchiveFileWriter::CheckDirectory(const FString& FilePath)
{
	filesystem::path FilePathName(FilePath.c_str());
	filesystem::path ParentDirectory = FilePathName.parent_path();

	UE_LOG("check dir %s", FilePath.c_str());
	if (!filesystem::exists(ParentDirectory))
	{
		try
		{
			filesystem::create_directories(ParentDirectory);
		}
		catch(const filesystem::filesystem_error& ex)
		{
			UE_LOG("디렉토리 생성 실패 : %s", FilePath.c_str());
			return false;
		}
	}

	return true;
}

bool FArchiveFileWriter::IsFileExist(const FString& FilePath)
{
	filesystem::path FilePathName(FilePath.c_str());

	if (!filesystem::exists(FilePathName))
	{
		UE_LOG(".Bin 파일이 존재하지 않습니다.");
		return false;
	}

	return true;
}
