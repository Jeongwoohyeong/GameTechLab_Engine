#include "pch.h"
#include "Actor/Public/MissileActor.h"
#include "Component/Mesh/Public/StaticMeshComponent.h"
#include "Component/Collision/Public/CapsuleComponent.h"
#include "Component/Public/SceneComponent.h"
#include "Component/Public/ULuaScriptComponent.h"

IMPLEMENT_CLASS(AMissileActor, AActor)

AMissileActor::AMissileActor()
{
	bCanEverTick = true;
	// 메쉬 컴포넌트 생성
	StaicMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>();
	if (!StaicMeshComponent)
	{
		UE_LOG_ERROR("AMissileActor: Failed to create StaicMeshComponent");
	}
	else
	{
		SetRootComponent(StaicMeshComponent);
		StaicMeshComponent->SetStaticMesh("Data/Missile.obj");
		StaicMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
		StaicMeshComponent->SetRelativeRotation(FQuaternion::FromEuler(FVector(0.0f, 0.0f, 0.0f)));
		StaicMeshComponent->SetRelativeScale3D(FVector(50.f, 200.f, 200.f));
		UE_LOG("[MissileActor] StaicMeshComponent created and attached");
	}
	// 캡슐 충돌 컴포넌트 생성
	MissileCollision = CreateDefaultSubobject<UCapsuleComponent>();
	if (!MissileCollision)
	{
		UE_LOG_ERROR("AMissileActor: Failed to create MissileCollision");
	}
	else
	{
		MissileCollision->AttachToComponent(StaicMeshComponent);
		MissileCollision->SetCapsuleHalfHeight(0.11f);
		MissileCollision->SetCapsuleRadius(0.1f);
		MissileCollision->SetRelativeLocation(FVector(0.3f, 0.0f, 0.0f));
		MissileCollision->SetRelativeRotation(FQuaternion::FromEuler(FVector(0.0f, 90.0f, 0.0f)));
		MissileCollision->bGenerateOverlapEvents = true;
		MissileCollision->bBlockComponent = false;
		MissileCollision->SetCanEverTick(true);
		UE_LOG("[MissileActor] CapsuleCollision created: Radius=80, HalfHeight=20, Tick=true");
	}
	// 충돌 콜백 등록
	if (MissileCollision)
	{
		MissileCollision->OnComponentBeginOverlap.AddDynamic(this, &AMissileActor::OnMissileBeginOverlap);
		UE_LOG("[MissileActor] Collision callback registered");
	}

	UE_LOG("[MissileActor] InitializeComponents complete!");
}

AMissileActor::~AMissileActor()
{
}

void AMissileActor::OnMissileBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신과의 충돌 무시
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	UE_LOG("[MissileActor] OnBeginOverlap with: %s", OtherActor->GetName().ToString().c_str());

	// Lua 스크립트에 충돌 이벤트 전달
	if (ULuaScriptComponent* LuaComp = GetLuaScriptComponent())
	{
		LuaComp->ActivateFunction("OnBeginOverlap", OverlappedComp, OtherActor,
			OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
}

