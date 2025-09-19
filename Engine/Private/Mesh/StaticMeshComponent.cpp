#include "pch.h"
#include "Mesh/StaticMeshComponent.h"

IMPLEMENT_CLASS(UStaticMeshComponent, UMeshComponent)






//TODO: Serialize 구현
//void UStaticMeshComponent::Serialize(bool bIsLoading, json::JSON Handle)
//{
//	//부모 객체부터 Serialize가 필요한데 일단 패스
//	//Super::Serialize(bIsLoading, Handle);
//
//	/*if (bIsLoading)
//	{
//		FString assetName;
//		Handle << "ObjStaticMeshAsset" << assetName;
//		StaticMesh = FObjManager::LoadObjStaticMesh(assetName);
//	}
//	else
//	{
//		FString assetName = StaticMesh->GetAssetPathFileName();
//		Handle << "ObjStaticMeshAsset" << assetName;
//	}*/
//}
