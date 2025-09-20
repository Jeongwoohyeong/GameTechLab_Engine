#include "pch.h"
#include "Components/StaticMeshComponent.h"
#include "Level/Level.h"

IMPLEMENT_CLASS(UStaticMeshComponent, UMeshComponent)


//StaticMesh가 구현되면 주석 해제(09/19 13:05)
void UStaticMeshComponent::AddToRenderList(ULevel* Level)
{
	Level->AddStaticMeshComponentToRender(this);
}



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


