#include "CScene.h"
#include "UObject.h"
#include "UEngineStatics.h"
#include "SimpleJSON/json.hpp"
#include "UUIManager.h"
#include "UCubeComp.h"
#include "USphereComp.h"
#include "UTriangleComp.h"
#include <fstream>

#if defined(_WIN32)
#include <direct.h>   // _mkdir
#endif

void CScene::New()
{
	Clear();

	UE_LOG("New scene created.");
}

static inline void EnsureScenesDirectory()
{
#if defined(_WIN32)
	int Result = _mkdir("Scenes");
#endif
}

// 원하는 키 순서(Version, NextUUID, Primitives)로 루트만 출력
static FString DumpSceneRootOrdered(const json::JSON& Root)
{
	using namespace json;

	TArray<TPair<const char*, const JSON*>> Items;
	if (Root.hasKey("Version"))    Items.push_back({ "Version",    &Root.at("Version") });
	if (Root.hasKey("NextUUID"))   Items.push_back({ "NextUUID",   &Root.at("NextUUID") });
	if (Root.hasKey("Primitives")) Items.push_back({ "Primitives", &Root.at("Primitives") });

	FString Result;
	Result += "{\n";
	const FString Indent = "  ";

	for (size_t i = 0; i < Items.size(); ++i)
	{
		const bool bIsLast = (i + 1 == Items.size());
		const bool bIsPrimitives = (std::strcmp(Items[i].first, "Primitives") == 0);

		Result += Indent;
		Result += "\"";
		Result += Items[i].first;
		Result += "\" : ";
		// Primitives는 중첩 객체이므로 들여쓰기 깊이를 한 단계 더 주어 보기 좋게 출력
		Result += Items[i].second->dump(bIsPrimitives ? 2 : 1, "  ");
		Result += bIsLast ? "\n" : ",\n";
	}

	Result += "}";
	return Result;
}

void CScene::Save(const FString& Name)
{
	using namespace json;

	EnsureScenesDirectory();

	JSON Root = Object();
	Root["Version"] = Version;
	Root["NextUUID"] = UEngineStatics::NextUUID;

	JSON PrimitivesObj = Object();

	for (const auto& Pair : UUIDToPrimitive)
	{
		UPrimitiveComponent* Primitive = Pair.second;
		uint32 UUID = Primitive->UUID;
		if (!Primitive) 
		{
			UE_LOG("Warning: PrimitiveComponent with UUID %d is null, skipping.", UUID);
			continue;
		}

		JSON Prim = Object();

		// Location
		Prim["Location"] = Array(
			Primitive->Transform.GetLocation().X,
			Primitive->Transform.GetLocation().Y,
			Primitive->Transform.GetLocation().Z
		);

		// Rotation
		Prim["Rotation"] = Array(
			Primitive->Transform.GetRotationRadians().X,
			Primitive->Transform.GetRotationRadians().Y,
			Primitive->Transform.GetRotationRadians().Z
		);

		// Scale
		Prim["Scale"] = Array(
			Primitive->Transform.GetScale().X,
			Primitive->Transform.GetScale().Y,
			Primitive->Transform.GetScale().Z
		);

		// Type
		Prim["Type"] = PrimitiveTypeToString(Primitive->GetPrimitiveType());

		PrimitivesObj[std::to_string(UUID)] = Prim;
	}

	Root["Primitives"] = PrimitivesObj;

	const FString OutPath = FString("Scenes/") + Name + ".Scene";
	std::ofstream Ofs(OutPath, std::ios::out | std::ios::trunc);
	if (!Ofs.is_open())
	{
		UE_LOG("Error: Could not open file %s for writing.", OutPath.c_str());
		return;
	}

	// 루트만 원하는 키 순서로 출력
	Ofs << DumpSceneRootOrdered(Root);
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
	else if (TypeStr == "Triangle")
	{
		return new UTriangleComp();
	}
	else
	{
		UE_LOG("Warning: Unknown primitive type '%s'. Skipping.", TypeStr.c_str());
		return nullptr;
	}
}

void CScene::Load(const FString& Name)
{
	using namespace json;

	Clear();

	const FString InPath = FString("Scenes/") + Name + ".Scene";
	std::ifstream Ifs(InPath, std::ios::in);
	if (!Ifs.is_open())
	{
		UE_LOG("Error: Could not open file %s for reading.", InPath.c_str());
	}

	// 파일 전체를 문자열로 로드
	FString Content((std::istreambuf_iterator<char>(Ifs)), std::istreambuf_iterator<char>());
	Ifs.close();

	JSON Root = JSON::Load(Content);
	if (Root.JSONType() != JSON::Class::Object)
	{
		UE_LOG("Error: Invalid scene format in %s.", InPath.c_str());
	}

	// Version / NextUUID
	if (Root.hasKey("Version"))  
	{
		Version = static_cast<int32>(Root.at("Version").ToInt());
	}
	if (Root.hasKey("NextUUID")) 
	{
		UEngineStatics::NextUUID = static_cast<uint32>(Root.at("NextUUID").ToInt());
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
				Comp->Transform.SetLocation(ReadVec3(Prim.at("Location")));
			}
			if (Prim.hasKey("Rotation")) 
			{
				Comp->Transform.SetRotationDeg(ReadVec3(Prim.at("Rotation")));
			}
			if (Prim.hasKey("Scale"))    
			{
				Comp->Transform.SetScale(ReadVec3(Prim.at("Scale")));
			}

			// UUID 설정
			Comp->UUID = UUID;

			// Scene에 추가
			UUIDToPrimitive[UUID] = Comp;
		}
	}
	else
	{
		UE_LOG("Warning: No 'Primitives' object in scene file %s.", InPath.c_str());
	}

	UE_LOG("Scene loaded from %s with %zu primitives.", InPath.c_str(), UUIDToPrimitive.size());
}

void CScene::Spawn(EPrimitiveType Type, uint32 Count)
{
	for (uint32 i = 0; i < Count; ++i)
	{
		UPrimitiveComponent* NewComp = nullptr;
		switch (Type)
		{
		case EPrimitiveType::Cube:
			NewComp = new UCubeComp();
			break;
		case EPrimitiveType::Sphere:
			NewComp = new USphereComp();
			break;
		case EPrimitiveType::Triangle:
			NewComp = new UTriangleComp();
			break;
		default:
			UE_LOG("Warning: Unknown primitive type enum value %d. Skipping spawn.", static_cast<uint8>(Type));
			continue;
		}

		if (NewComp)
		{
			NewComp->UUID = UEngineStatics::GenUUID();
			UUIDToPrimitive[NewComp->UUID] = NewComp;

			UE_LOG("Spawned %s with UUID %d", PrimitiveTypeToString(Type).c_str(), NewComp->UUID);
		}
	}
}

void CScene::DestroySelectedPrimitive()
{
	if (SelectedPrimitive)
	{
		uint32 UUID = SelectedPrimitive->UUID;
		auto It = UUIDToPrimitive.find(UUID);
		if (It != UUIDToPrimitive.end())
		{
			It->second->Release();
			delete It->second;
			UUIDToPrimitive.erase(It);
			UE_LOG("Destroyed primitive with UUID %d", UUID);
		}
		else
		{
			UE_LOG("Warning: Selected primitive with UUID %d not found in scene map.", UUID);
		}

		SelectedPrimitive = nullptr;
	}
	else
	{
		UE_LOG("No primitive is currently selected to destroy.");
	}
}

void CScene::SetSelectedPrimitiveByUUID(uint32 UUID)
{
	for (auto& Pair : UUIDToPrimitive)
	{
		if (Pair.first == UUID)
		{
			SelectedPrimitive = Pair.second;
			UE_LOG("Selected primitive with UUID %d", SelectedPrimitive->UUID);
			return;
		}
	}

	SelectedPrimitive = nullptr;
	UE_LOG("No primitive found with UUID %d. No primitive selected.", UUID);
}

void CScene::Clear()
{
	for (auto& Pair : UUIDToPrimitive)
	{
		if (Pair.second)
		{
			Pair.second->Release();
			delete Pair.second;
		}
	}

	UUIDToPrimitive.clear();
	SelectedPrimitive = nullptr;
}
