#include "CSceneSerializer.h"
#include "SimpleJSON/json.hpp"
#include "UUIManager.h"
#include "UCubeComp.h"
#include "USphereComp.h"
#include <fstream>

#if defined(_WIN32)
#include <direct.h>   // _mkdir
#endif

static inline void EnsureScenesDirectory()
{
#if defined(_WIN32)
	int Result = _mkdir("Scenes");
#endif
}

void CSceneSerializer::SaveScene(const FString& Name, const FScene& Scene)
{
	using namespace json;

	EnsureScenesDirectory();

	JSON Root = Object();
	Root["Version"] = Scene.Version;
	Root["NextUUID"] = Scene.NextUUID;

	JSON PrimitivesObj = Object();

	for (UPrimitiveComponent* Primitive : Scene.Primitives)
	{
		uint32 UUID = Primitive->UUID;
		if (!Primitive) 
		{
			UE_LOG("Warning: PrimitiveComponent with UUID %d is null, skipping.", UUID);
			continue;
		}

		JSON Prim = Object();

		// Location
		Prim["Location"] = Array(
			Primitive->RelativeLocation.X,
			Primitive->RelativeLocation.Y,
			Primitive->RelativeLocation.Z
		);

		// Rotation
		Prim["Rotation"] = Array(
			Primitive->RelativeRotation.X,
			Primitive->RelativeRotation.Y,
			Primitive->RelativeRotation.Z
		);

		// Scale
		Prim["Scale"] = Array(
			Primitive->RelativeScale3D.X,
			Primitive->RelativeScale3D.Y,
			Primitive->RelativeScale3D.Z
		);

		// Type
		Prim["Type"] = PrimitiveTypeToString(Primitive->GetPrimitiveType());

		PrimitivesObj[std::to_string(UUID)] = Prim;
	}

	Root["Primitives"] = PrimitivesObj;

	const FString OutPath = FString("Scenes/") + Name + ".scene";
	std::ofstream Ofs(OutPath, std::ios::out | std::ios::trunc);
	if (!Ofs.is_open())
	{
		UE_LOG("Error: Could not open file %s for writing.", OutPath.c_str());
		return;
	}

	// 보기 좋은 포맷으로 출력(SimpleJSON의 dump는 기본 2 스페이스 들여쓰기)
	Ofs << Root.dump();
	Ofs.close();

	UE_LOG("Scene saved to %s", OutPath.c_str());
}

inline float JSONNumberToFloat(const json::JSON& Json)
{
	using Class = json::JSON::Class;

	if (Json.JSONType() == Class::Floating) 
	{
		return static_cast<float>(Json.ToFloat());
	}
	if (Json.JSONType() == Class::Integral) 
	{
		return static_cast<float>(Json.ToInt());
	}

	return 0.0f;
}

inline FVector ReadVec3(const json::JSON& Arr)
{
	FVector Result{ 0, 0, 0 };
	if (Arr.JSONType() == json::JSON::Class::Array && Arr.length() >= 3)
	{
		Result.X = JSONNumberToFloat(Arr.at(0));
		Result.Y = JSONNumberToFloat(Arr.at(1));
		Result.Z = JSONNumberToFloat(Arr.at(2));
	}
	return Result;
}

inline UPrimitiveComponent* CreatePrimitiveFromType(const FString& TypeStr)
{
	if (TypeStr == "Cube")
	{
		return new UCubeComp();
	}
	else if (TypeStr == "Sphere")
	{
		return new USphereComp();
	}
	else
	{
		UE_LOG("Warning: Unknown primitive type '%s'. Skipping.", TypeStr.c_str());
		return nullptr;
	}
}

FScene CSceneSerializer::LoadScene(const FString& Name)
{
	using namespace json;

	FScene Scene;
	Scene.Version = 1;
	Scene.NextUUID = 0;
	Scene.Primitives.clear();

	const FString InPath = FString("Scenes/") + Name + ".scene";
	std::ifstream Ifs(InPath, std::ios::in);
	if (!Ifs.is_open())
	{
		UE_LOG("Error: Could not open file %s for reading.", InPath.c_str());
		return Scene;
	}

	// 파일 전체를 문자열로 로드
	FString Content((std::istreambuf_iterator<char>(Ifs)), std::istreambuf_iterator<char>());
	Ifs.close();

	JSON Root = JSON::Load(Content);
	if (Root.JSONType() != JSON::Class::Object)
	{
		UE_LOG("Error: Invalid scene format in %s.", InPath.c_str());
		return Scene;
	}

	// Version / NextUUID
	if (Root.hasKey("Version"))  
	{
		Scene.Version = static_cast<int32>(Root.at("Version").ToInt());
	}
	if (Root.hasKey("NextUUID")) 
	{
		Scene.NextUUID = static_cast<uint32>(Root.at("NextUUID").ToInt());
	}

	// Primitives
	if (Root.hasKey("Primitives") && Root.at("Primitives").JSONType() == JSON::Class::Object)
	{
		const JSON& PrimsObj = Root.at("Primitives");
		for (const auto& Pair : PrimsObj.ObjectRange())
		{
			const FString& UuidStr = Pair.first;
			const JSON& Prim = Pair.second;

			// UUID 파싱
			uint32 UUID = static_cast<uint32>(std::stoul(UuidStr));

			// Type
			if (!Prim.hasKey("Type") || Prim.at("Type").JSONType() != JSON::Class::String)
			{
				UE_LOG("Warning: Primitive %u has no valid 'Type'. Skipping.", UUID);
				continue;
			}
			const FString TypeStr = Prim.at("Type").ToString();

			UPrimitiveComponent* Comp = CreatePrimitiveFromType(TypeStr);
			if (!Comp) 
			{
				continue;
			}

			// Transform
			if (Prim.hasKey("Location")) 
			{
				Comp->RelativeLocation = ReadVec3(Prim.at("Location"));
			}
			if (Prim.hasKey("Rotation")) 
			{
				Comp->RelativeRotation = ReadVec3(Prim.at("Rotation"));
			}
			if (Prim.hasKey("Scale"))    
			{
				Comp->RelativeScale3D = ReadVec3(Prim.at("Scale"));
			}

			// UUID 설정
			Comp->UUID = UUID;

			// Scene에 추가
			Scene.Primitives.push_back(Comp);
		}
	}
	else
	{
		UE_LOG("Warning: No 'Primitives' object in scene file %s.", InPath.c_str());
	}

	return Scene;
}
