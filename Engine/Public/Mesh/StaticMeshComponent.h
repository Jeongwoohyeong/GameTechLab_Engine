#pragma once

#include "Mesh/MeshComponent.h"
//#include "json.hpp"

class UStaticMesh;

class UStaticMeshComponent : public UMeshComponent
{
	DECLARE_CLASS(UStaticMeshComponent, UMeshComponent)
public:





	//TODO: Serialize 구현
	//StaticMesh에셋을 Json에 저장하거나 로드하는 함수
	//bIsLoading이 true : 로드 false : 저장
	//void Serialize(bool bIsLoading, json::JSON Handle);

private:
	UStaticMesh* StaticMesh = nullptr;
};

