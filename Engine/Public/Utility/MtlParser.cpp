#include "pch.h"
#include "MtlParser.h"
#include "Mesh/Material.h"

FMtlParser::FMtlParser(TMap<FString, UMaterial*>* InMaterials)
{
	Materials = InMaterials;
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
	FObjMaterialInfo CurrentMaterialInfo;
	bool bIsEmpty = true;
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
			if (!bIsEmpty)
			{
				Materials->emplace(CurrentMaterialInfo.MaterialName, new UMaterial(CurrentMaterialInfo));
				bIsEmpty = true;
			}
			CurrentMaterialInfo = {};
			Ss >> CurrentMaterialInfo.MaterialName;
			
			bIsEmpty = false;
			UE_LOG("mtl name %s", CurrentMaterialInfo.MaterialName.c_str());
		}
		else if (bIsEmpty)
		{
			continue;
		}
		else if (Token == "Ka")
		{
			Ss >> CurrentMaterialInfo.Ka.X >> CurrentMaterialInfo.Ka.Y >> CurrentMaterialInfo.Ka.Z;
			UE_LOG("%f %f %f", CurrentMaterialInfo.Ka.X, CurrentMaterialInfo.Ka.Y, CurrentMaterialInfo.Ka.Z);
		}
		else if (Token == "Kd")
		{
			Ss >> CurrentMaterialInfo.Kd.X >> CurrentMaterialInfo.Kd.Y >> CurrentMaterialInfo.Kd.Z;
			UE_LOG("%f %f %f", CurrentMaterialInfo.Kd.X, CurrentMaterialInfo.Kd.Y, CurrentMaterialInfo.Kd.Z);
		}
		else if (Token == "Ks")
		{
			Ss >> CurrentMaterialInfo.Ks.X >> CurrentMaterialInfo.Ks.Y >> CurrentMaterialInfo.Ks.Z;
			UE_LOG("%f %f %f", CurrentMaterialInfo.Ks.X, CurrentMaterialInfo.Ks.Y, CurrentMaterialInfo.Ks.Z);
		}
		else if (Token == "Ke")
		{
			Ss >> CurrentMaterialInfo.Ke.X >> CurrentMaterialInfo.Ke.Y >> CurrentMaterialInfo.Ke.Z;
			UE_LOG("%f %f %f", CurrentMaterialInfo.Ke.X, CurrentMaterialInfo.Ke.Y, CurrentMaterialInfo.Ke.Z);
		}
		else if (Token == "Ns")
		{
			Ss >> CurrentMaterialInfo.Ns;
			UE_LOG("Ns %f", CurrentMaterialInfo.Ns);
		}
		else if (Token == "Ni")
		{
			Ss >> CurrentMaterialInfo.Ni;
			UE_LOG("Ni %f", CurrentMaterialInfo.Ni);
		}
		else if (Token == "d")
		{
			Ss >> CurrentMaterialInfo.d;
			UE_LOG("d %f", CurrentMaterialInfo.d);
		}
		else if (Token == "illum")
		{
			Ss >> CurrentMaterialInfo.illum;
			UE_LOG("illum %d", CurrentMaterialInfo.illum);
		}
		else if (Token == "map_Kd")
		{
			Ss >> MapFileName;
			MapFileName = Path + MapFileName;
			CurrentMaterialInfo.Map_Kd = MapFileName;
			UE_LOG("Map Kd %s", CurrentMaterialInfo.Map_Kd.c_str());
		}
		else if (Token == "map_Ks")
		{
			Ss >> MapFileName;
			MapFileName = Path + MapFileName;
			CurrentMaterialInfo.Map_Ks = MapFileName;
			UE_LOG("Map Ks %s", CurrentMaterialInfo.Map_Ks.c_str());
		}
		else if (Token == "map_bump")
		{
			Ss >> MapFileName;
			MapFileName = Path + MapFileName;
			CurrentMaterialInfo.Map_bump = MapFileName;
			UE_LOG("Map bump %s", CurrentMaterialInfo.Map_bump.c_str());
		}
	}

	if (!bIsEmpty)
	{
		Materials->emplace(CurrentMaterialInfo.MaterialName, new UMaterial(CurrentMaterialInfo));
	}

	return true;
}
