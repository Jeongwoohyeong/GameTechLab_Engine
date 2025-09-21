#include "pch.h"
#include "Material.h"

IMPLEMENT_CLASS(UMaterial, UObject)

UMaterial::UMaterial()
{
}

UMaterial::UMaterial(FObjMaterialInfo* InFObjMaterial)
{
		
}

UMaterial::~UMaterial()
{
}

void UMaterial::SetMaterialInfo(FObjMaterialInfo* InFObjMaterial)
{
	ObjMaterialInfo = InFObjMaterial;
	if (ObjMaterialInfo == nullptr)
	{
		UE_LOG("Material 정보가 없습니다.");
		return;
	}

	if (!ObjMaterialInfo->Map_Kd.empty())
	{
		KdTextureFilePath = ObjMaterialInfo->Map_Kd;
	}

	if (!ObjMaterialInfo->Map_Ks.empty())
	{
		KdTextureFilePath = ObjMaterialInfo->Map_Ks;
	}

	if (!ObjMaterialInfo->Map_bump.empty())
	{
		KdTextureFilePath = ObjMaterialInfo->Map_bump;
	}
}
