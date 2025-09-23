#include "pch.h"
#include "Material.h"

IMPLEMENT_CLASS(UMaterial, UObject)

UMaterial::UMaterial()
{
}

UMaterial::UMaterial(const FObjMaterialInfo& InFObjMaterial)
{
	MaterialInfo = InFObjMaterial;
	SetMaterialName();
	
}
UMaterial::~UMaterial()
{
}

void UMaterial::SetMaterialInfo(const FObjMaterialInfo& InFObjMaterial)
{
	MaterialInfo = InFObjMaterial;
	SetMaterialName();
}

void UMaterial::SetMaterialName()
{
	uint32 LastSlashIndex = -1;
	uint32 LastDotIndex = -1;

	LastSlashIndex = MaterialInfo.Map_Kd.find_last_of("/");
	LastDotIndex = MaterialInfo.Map_Kd.find_last_of(".");

	//슬래시가 존재, dot이 있든 없든 두가지 케이스 커버(Dot이 없다면 LastDotIndex-LastSlashIndex가 string 끝까지 추출)
	if (LastSlashIndex != FString::npos && LastSlashIndex < LastDotIndex)
	{
		MaterialName = MaterialInfo.Map_Kd.substr(LastSlashIndex + 1, LastDotIndex - LastSlashIndex-1);
	}
	//슬래시는 있는데 dot이 없는 경우
	else if (LastSlashIndex != FString::npos)
	{
		MaterialName = MaterialInfo.Map_Kd.substr(LastSlashIndex + 1);
	}
	//슬래시 없는 경우.
	else
	{
		MaterialName = MaterialInfo.Map_Kd.substr(0, LastDotIndex-1);
	}
}

//const FObjMaterialInfo* UMaterial::GetMaterialInfo(const FString& MtlName) const
//{
//	auto It = ObjMaterialInfoMap->find(MtlName);
//
//	if (It == ObjMaterialInfoMap->end())
//	{
//		UE_LOG("재질 정보가 없습니다.");
//		return nullptr;
//	}
//
//	return (*It).second;
//}
//
//const FString& UMaterial::GetKdTextureFilePath(const FString& MtlName)
//{
//	auto It = ObjMaterialInfoMap->find(MtlName);
//
//	if (It == ObjMaterialInfoMap->end())
//	{
//		UE_LOG("재질 정보가 없습니다.");
//		return FString();
//	}
//
//	KdTextureFilePath = (*It).second->Map_Kd;
//
//	if(KdTextureFilePath.empty())
//	{
//		return FString();
//	}
//
//	return KdTextureFilePath;
//}
//
//const FString& UMaterial::GetKsTextureFilePath(const FString& MtlName)
//{
//	auto It = ObjMaterialInfoMap->find(MtlName);
//
//	if (It == ObjMaterialInfoMap->end())
//	{
//		UE_LOG("재질 정보가 없습니다.");
//		return FString();
//	}
//
//	KsTextureFilePath = (*It).second->Map_Ks;
//
//	if (KsTextureFilePath.empty());
//	{
//		return FString();
//	}
//
//	return KsTextureFilePath;
//}
//
//const FString& UMaterial::GetBumpTextureFilePath(const FString& MtlName)
//{
//	auto It = ObjMaterialInfoMap->find(MtlName);
//
//	if (It == ObjMaterialInfoMap->end())
//	{
//		UE_LOG("재질 정보가 없습니다.");
//		return FString();
//	}
//
//	BumpTextureFilePath = (*It).second->Map_bump;
//
//	if (BumpTextureFilePath.empty());
//	{
//		return FString();
//	}
//
//	return BumpTextureFilePath;
//}

//void UMaterial::SetMaterialInfo(TMap<FString, FObjMaterialInfo*>* InFObjMaterial)
//{
//	ObjMaterialInfoMap = InFObjMaterial;
//	if (ObjMaterialInfoMap == nullptr)
//	{
//		UE_LOG("Material 정보가 없습니다.");
//		return;
//	}
//
//	/*for (auto it = ObjMaterialInfoMap->begin(); it != ObjMaterialInfoMap->end(); ++it)
//	{
//		UE_LOG("umaterial mtl %s", (*it).first.c_str());
//		UE_LOG("ka %f %f %f", (*it).second->Ka.X, (*it).second->Ka.Y, (*it).second->Ka.Z);
//		UE_LOG("kd %f %f %f", (*it).second->Kd.X, (*it).second->Kd.Y, (*it).second->Kd.Z);
//		UE_LOG("ks %f %f %f", (*it).second->Ks.X, (*it).second->Ks.Y, (*it).second->Ks.Z);
//		UE_LOG("ke %f %f %f", (*it).second->Ke.X, (*it).second->Ke.Y, (*it).second->Ke.Z);
//		UE_LOG("Ns %f", (*it).second->Ns);
//		UE_LOG("Ni %f", (*it).second->Ni);
//		UE_LOG("d %f", (*it).second->d);
//		UE_LOG("illum %d", (*it).second->illum);
//		UE_LOG("Map kd %s", (*it).second->Map_Kd.c_str());
//		UE_LOG("Map ks %s", (*it).second->Map_Ks.c_str());
//		UE_LOG("Map bump %s", (*it).second->Map_bump.c_str());
//	}*/
//}
