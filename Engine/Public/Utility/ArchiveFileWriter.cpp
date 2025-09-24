#include "pch.h"
#include "ArchiveFileWriter.h"

FArchiveFileWriter::FArchiveFileWriter(const FString& OriginalPath, const FString& BinFilePath)
{
	// 이진파일 쓰기
	if (CheckDirectory(BinFilePath) && IsBinOld(OriginalPath, BinFilePath))
	{
		fopen_s(&FilePointer, BinFilePath.c_str(), "wb");
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

bool FArchiveFileWriter::IsBinOld(const FString& OriginalFile, const FString& BinFile)
{
	// .obj가 존재하지 않고 .bin이 존재하면 bin파일 삭제
	if (!IsFileExist(OriginalFile) && IsFileExist(BinFile))
	{
		filesystem::remove(BinFile);
		return false;
	}

	// obj파일이 최신이면 bin 작성
	try
	{
		auto BinFileTime = filesystem::last_write_time(BinFile);
		auto ObjFileTime = filesystem::last_write_time(OriginalFile);

		if (ObjFileTime > BinFileTime)
		{
			UE_LOG("%s 갱신", BinFile.c_str());
			return true;
		}
	}
	// try 구문이 실패하면 무조건 bin 작성
	catch (const filesystem::filesystem_error& ex)
	{
		UE_LOG("파일 시간 비교 중 오류 발생 %s", ex.what());
		return true;
	}

	return false;
}
