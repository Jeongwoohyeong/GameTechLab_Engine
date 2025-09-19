#pragma once

#include "Mesh/MeshComponent.h"
//#include "json.hpp"

class UStaticMesh;

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_CLASS(UStaticMeshComponent, UMeshComponent)
public:

	//StaticMesh가 구현되면 주석 해제(09/19 13:05)
	//자식 StaticMeshComponent가 본인 타입에 맞는 렌더 리스트에 알아서 추가
	//virtual void AddToRenderList(ULevel* Level) override;
	///////////////////////////////////////////////////



	//TODO: Serialize 구현
	//StaticMesh에셋을 Json에 저장하거나 로드하는 함수
	//bIsLoading이 true : 로드 false : 저장
	//void Serialize(bool bIsLoading, json::JSON Handle);

private:
	UStaticMesh* StaticMesh = nullptr;
};

