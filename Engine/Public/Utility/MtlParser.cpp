#include "pch.h"
#include "MtlParser.h"

FMtlParser::FMtlParser(TMap<FString, TMap<FString, FObjMaterialInfo*>>* InMtlFileMap)
{
	MtlFileMap = InMtlFileMap;
}

bool FMtlParser::ParseMtl(const FString& PathFileName)
{
	ifstream File(PathFileName);

	if (!File.is_open())
	{
		UE_LOG("Mtl 파일 열기 실패");
		return false;
	}

	uint32 Pos = PathFileName.find_last_of("/\\");
	FString Path = PathFileName.substr(0, Pos + 1);

	FString Line;
	TMap<FString, FObjMaterialInfo*> MtlInfoMap{};
	FObjMaterialInfo* CurrentMtlInfo = nullptr;
	FString CurrentMtlName{};
	while (std::getline(File, Line))
	{
		// 빈 줄, 주석 제외
		if (Line.empty() || Line[0] == '#')
		{
			continue;
		}

		std::stringstream Ss(Line);
		FString Token;
		Ss >> Token;

		if (Token.empty())
		{
			continue;
		}
		
		FString MapFileName;
		if (Token == "newmtl")
		{
			if (CurrentMtlInfo != nullptr)
			{
				MtlInfoMap.Emplace(CurrentMtlName, CurrentMtlInfo);
				CurrentMtlInfo = nullptr;
			}

			Ss >> CurrentMtlName;
			CurrentMtlInfo = new FObjMaterialInfo();
			UE_LOG("mtl name %s", CurrentMtlName.c_str());
		}
		else if (CurrentMtlInfo == nullptr)
		{
			continue;
		}
		else if (Token == "Ka")
		{
			Ss >> CurrentMtlInfo->Ka.X >> CurrentMtlInfo->Ka.Y >> CurrentMtlInfo->Ka.Z;
			UE_LOG("%f %f %f", CurrentMtlInfo->Ka.X, CurrentMtlInfo->Ka.Y, CurrentMtlInfo->Ka.Z);
		}
		else if (Token == "Kd")
		{
			Ss >> CurrentMtlInfo->Kd.X >> CurrentMtlInfo->Kd.Y >> CurrentMtlInfo->Kd.Z;
			UE_LOG("%f %f %f", CurrentMtlInfo->Kd.X, CurrentMtlInfo->Kd.Y, CurrentMtlInfo->Kd.Z);
		}
		else if (Token == "Ks")
		{
			Ss >> CurrentMtlInfo->Ks.X >> CurrentMtlInfo->Ks.Y >> CurrentMtlInfo->Ks.Z;
			UE_LOG("%f %f %f", CurrentMtlInfo->Ks.X, CurrentMtlInfo->Ks.Y, CurrentMtlInfo->Ks.Z);
		}
		else if (Token == "Ke")
		{
			Ss >> CurrentMtlInfo->Ke.X >> CurrentMtlInfo->Ke.Y >> CurrentMtlInfo->Ke.Z;
			UE_LOG("%f %f %f", CurrentMtlInfo->Ke.X, CurrentMtlInfo->Ke.Y, CurrentMtlInfo->Ke.Z);
		}
		else if (Token == "Ns")
		{
			Ss >> CurrentMtlInfo->Ns;
			UE_LOG("Ns %f", CurrentMtlInfo->Ns);
		}
		else if (Token == "Ni")
		{
			Ss >> CurrentMtlInfo->Ni;
			UE_LOG("Ni %f", CurrentMtlInfo->Ni);
		}
		else if (Token == "d")
		{
			Ss >> CurrentMtlInfo->d;
			UE_LOG("d %f", CurrentMtlInfo->d);
		}
		else if (Token == "illum")
		{
			Ss >> CurrentMtlInfo->illum;
			UE_LOG("illum %d", CurrentMtlInfo->illum);
		}
		else if (Token == "map_Kd")
		{
			Ss >> MapFileName;
			MapFileName = Path + MapFileName;
			CurrentMtlInfo->Map_Kd = MapFileName;
			UE_LOG("Map Kd %s", CurrentMtlInfo->Map_Kd.c_str());
		}
		else if (Token == "map_Ks")
		{
			Ss >> MapFileName;
			MapFileName = Path + MapFileName;
			CurrentMtlInfo->Map_Ks = MapFileName;
			UE_LOG("Map Ks %s", CurrentMtlInfo->Map_Ks.c_str());
		}
		else if (Token == "map_bump")
		{
			Ss >> MapFileName;
			MapFileName = Path + MapFileName;
			CurrentMtlInfo->Map_bump = MapFileName;
			UE_LOG("Map bump %s", CurrentMtlInfo->Map_bump.c_str());
		}
	}

	if (CurrentMtlInfo != nullptr)
	{
		MtlInfoMap.Emplace(CurrentMtlName, CurrentMtlInfo);
	}

	MtlFileMap->Emplace(PathFileName, MtlInfoMap);

	return true;
}
