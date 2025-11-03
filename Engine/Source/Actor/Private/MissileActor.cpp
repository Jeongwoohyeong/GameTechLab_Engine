#include "pch.h"
#include "Actor/Public/MissileActor.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Collision/Public/CapsuleComponent.h"
#include "Component/Public/SceneComponent.h"

IMPLEMENT_CLASS(AMissileActor, AActor)

AMissileActor::AMissileActor()
{
	bCanEverTick = true;
}

AMissileActor::~AMissileActor()
{
}

UClass* AMissileActor::GetDefaultRootComponent()
{
	return USceneComponent::StaticClass();
}

void AMissileActor::InitializeComponents()
{
	Super::InitializeComponents();

	// 메쉬 컴포넌트 생성
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>();
	if (MeshComponent)
	{
		MeshComponent->AttachToComponent(GetRootComponent());
		MeshComponent->SetStaticMesh("Data/Missile.obj");
		UE_LOG("[MissileActor] MeshComponent created and attached");
	}

	// 캡슐 충돌 컴포넌트 생성
	MissileCollision = CreateDefaultSubobject<UCapsuleComponent>();
	if (MissileCollision)
	{
		MissileCollision->AttachToComponent(GetRootComponent());
		MissileCollision->SetCapsuleRadius(10.0f);  // 임시 값 (나중에 조절)
		MissileCollision->SetCapsuleHalfHeight(50.0f);  // 임시 값 (나중에 조절)
		MissileCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));  // 임시 위치
		MissileCollision->bGenerateOverlapEvents = true;
		UE_LOG("[MissileActor] CapsuleCollision created and attached");
	}

	UE_LOG("[MissileActor] InitializeComponents complete!");
}
