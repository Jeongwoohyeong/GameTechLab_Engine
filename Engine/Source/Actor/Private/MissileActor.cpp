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
		// ✅ StaticMeshComponent는 충돌 처리하지 않으므로 Tick 비활성화
		StaicMeshComponent->SetCanEverTick(false);
		StaicMeshComponent->bGenerateOverlapEvents = false;
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

void AMissileActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG("[MissileActor] ========== BeginPlay START ==========");

	// CapsuleComponent를 Level에 등록하여 Tick과 충돌 감지 활성화
	if (MissileCollision)
	{
		UE_LOG("[MissileActor] Registering CapsuleComponent...");
		RegisterComponent(MissileCollision);
		UE_LOG("[MissileActor] ✅ CapsuleComponent registered to Level!");
		UE_LOG("[MissileActor] CapsuleComponent CanEverTick: %d", MissileCollision->CanEverTick());
		UE_LOG("[MissileActor] CapsuleComponent bGenerateOverlapEvents: %d", MissileCollision->bGenerateOverlapEvents);
	}
	else
	{
		UE_LOG_ERROR("[MissileActor] ❌ MissileCollision is nullptr!");
	}

	UE_LOG("[MissileActor] ========== BeginPlay END ==========");
}

void AMissileActor::OnMissileBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG("[MissileActor] ========== OnMissileBeginOverlap CALLED! ==========");
	UE_LOG("[MissileActor] This missile: %s", GetName().ToString().c_str());

	// 자기 자신과의 충돌 무시
	if (!OtherActor || OtherActor == this)
	{
		UE_LOG("[MissileActor] ❌ Collision ignored: OtherActor is nullptr or self");
		return;
	}

	// ✅ 플레이어와의 충돌 무시 (미사일이 발사된 직후 플레이어와 충돌하는 것 방지)
	FString OtherName = OtherActor->GetName().ToString();
	UE_LOG("[MissileActor] Collision detected with: %s", OtherName.c_str());

	if (OtherName.find("APlayerCharacter") != std::string::npos)
	{
		// 플레이어와의 충돌은 무시
		UE_LOG("[MissileActor] ❌ Collision ignored: OtherActor is PlayerCharacter");
		return;
	}

	// ✅ 다른 미사일과의 충돌 무시
	if (OtherName.find("AMissileActor") != std::string::npos)
	{
		// 미사일끼리의 충돌은 무시
		UE_LOG("[MissileActor] ❌ Collision ignored: OtherActor is another Missile");
		return;
	}

	UE_LOG("[MissileActor] ✅ Valid collision! Passing to Lua: %s", OtherName.c_str());

	// Lua 스크립트에 충돌 이벤트 전달
	if (ULuaScriptComponent* LuaComp = GetLuaScriptComponent())
	{
		UE_LOG("[MissileActor] ✅ Lua script found, calling OnBeginOverlap");
		LuaComp->ActivateFunction("OnBeginOverlap", OverlappedComp, OtherActor,
			OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	}
	else
	{
		UE_LOG("[MissileActor] ❌ WARNING: No Lua script component! Cannot call OnBeginOverlap");
	}

	UE_LOG("[MissileActor] ========== OnMissileBeginOverlap END ==========");
}

